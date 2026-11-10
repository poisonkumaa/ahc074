from pathlib import Path
import subprocess
import sys

directory = Path(__file__).resolve().parent
source = (directory / "fast_fixed.cpp").read_text(encoding="utf-8")


def replace_once(old, new):
    global source
    assert source.count(old) == 1, (old[:100], source.count(old))
    source = source.replace(old, new, 1)


source = '''// Apply the target before standard headers, including bitset::count.
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC target("popcnt")
#endif
''' + source
replace_once('''namespace fixed_deck {
using Mask=bitset<K>;''', '''namespace fixed_deck {
using Mask=bitset<K>;
struct UniqueStatistics {
    long long trials=0,nodes=0,subsets=0,wins=0,saved=0,timeouts=0,skipped=0;
    double cpu=0;
};
static UniqueStatistics unique_stats;
static bool unique_enabled(){static const bool value=search_budget::integer("ROUTE_FIXED_ALL",0)!=0;return value;}''')
replace_once('''    int limit,nodes=0,subsets=0;
    Work(const vector<uint64_t>& ink,const vector<int>& start,const vector<int>& goal,int limit,
         const Columns* shared_columns=nullptr,bool cache_blockers=false):
           ink(ink),start(start),goal(goal),columns(shared_columns?shared_columns:&local_columns),
           cache_blockers(cache_blockers),limit(limit){''', '''    int limit,nodes=0,subsets=0;
    clock_t deadline=0;bool stopped=false;
    bool expired(){if(deadline>0 && clock()>=deadline)stopped=true;return stopped;}
    Work(const vector<uint64_t>& ink,const vector<int>& start,const vector<int>& goal,int limit,
         const Columns* shared_columns=nullptr,bool cache_blockers=false,clock_t deadline=0):
           ink(ink),start(start),goal(goal),columns(shared_columns?shared_columns:&local_columns),
           cache_blockers(cache_blockers),limit(limit),deadline(deadline){''')
replace_once('''        if(++subsets>64)return false;''', '''        if(expired() || ++subsets>64)return false;''')
replace_once('''                if(++steps>1200 || dead[m%2048]==m)return false;''', '''                if((!(steps&31) && expired()) || ++steps>1200 || dead[m%2048]==m)return false;''')
replace_once('''        for(int m=0;m<full && parent[full]<0;m++)if(parent[m]>=0){
            for(int left=full^m;left;left&=left-1){''', '''        for(int m=0;m<full && parent[full]<0;m++) {
            if(!(m&63) && expired())return false;
            if(parent[m]>=0){
            for(int left=full^m;left;left&=left-1){''')
replace_once('''        if(parent[full]<0)return false;''', '''        }
        if(parent[full]<0)return false;''')
replace_once('''        if(++nodes>250 || subsets>64)return false;''', '''        if(expired() || ++nodes>250 || subsets>64)return false;''')
replace_once('''static void optimize(const OnlineRouter& r,const vector<int>& start,vector<pair<int,int>>& moves){
    if(moves.size()<2||moves.size()>22)return;
    Mask seen;bool repeated=false;
    for(auto op:moves){repeated|=seen.test(op.first);seen.set(op.first);}
    if(!repeated)return;
    const bool fast=fast_enabled();''', '''static void optimize(const OnlineRouter& r,const vector<int>& start,vector<pair<int,int>>& moves,clock_t deadline=0){
    if(moves.size()<2||moves.size()>22)return;
    Mask seen;bool repeated=false;
    for(auto op:moves){repeated|=seen.test(op.first);seen.set(op.first);}
    clock_t begun=0,stop=0;
    if(!repeated) {
        if(!unique_enabled())return;
        static const double budget=search_budget::seconds(std::max(0.0,search_budget::real("ROUTE_FIXED_CPU",.15)));
        if(unique_stats.cpu>=budget || routing_budget::expired(deadline)){unique_stats.skipped++;return;}
        begun=clock();
        stop=begun+clock_t(std::max(0.0,budget-unique_stats.cpu)*CLOCKS_PER_SEC);
        if(deadline>0)stop=std::min(deadline,stop);
        unique_stats.trials++;
    }
    const bool fast=fast_enabled();''')
replace_once('''    Work w(r.opaque,start,r.sim.order,int(moves.size())-1,
           fast?r.fixed_deck_column_masks.get():nullptr,fast);''', '''    Work w(r.opaque,start,r.sim.order,int(moves.size())-1,
           fast?r.fixed_deck_column_masks.get():nullptr,fast,stop);''')
replace_once('''    if(fast && verify_enabled()) {
        Work reference''', '''    // A stopped bounded search is incomplete, so it is not comparable to an
    // untimed reference. Completed searches must match the original Work.
    if(fast && verify_enabled() && !w.stopped) {
        Work reference''')
replace_once('''    if(found)moves=move(w.answer);
}
}

// Rebuild a post-promotion suffix''', '''    if(!repeated) {
        unique_stats.nodes+=w.nodes;unique_stats.subsets+=w.subsets;unique_stats.timeouts+=w.stopped;
        if(found){unique_stats.wins++;unique_stats.saved+=int(moves.size())-int(w.answer.size());}
        unique_stats.cpu+=double(clock()-begun)/CLOCKS_PER_SEC;
    }
    if(found)moves=move(w.answer);
}
}

// Rebuild a post-promotion suffix''')
replace_once('fixed_deck::optimize(actual,trace_start,moves);',
             'fixed_deck::optimize(actual,trace_start,moves,actual.route_deadline);')
anchor = '    cerr<<"FINAL_DECK saved="<<end_deck_saved<<"\\n";'
replace_once(anchor, anchor + r'''
    const auto&fu=fixed_deck::unique_stats;
    cerr<<"ROUTE_FIXED_ALL enabled="<<fixed_deck::unique_enabled()<<" trials="<<fu.trials
        <<" nodes="<<fu.nodes<<" subsets="<<fu.subsets<<" wins="<<fu.wins<<" saved="<<fu.saved
        <<" timeouts="<<fu.timeouts<<" skipped="<<fu.skipped<<" cpu="<<fu.cpu<<"\n";
''')
target = directory / "route_fastall.cpp"
target.write_text(source, encoding="utf-8")
fixture = directory / "route_fastall_fixture.cpp"
subprocess.run([sys.executable, str(directory.parent / "beam_v1/make_fixture.py"), str(target), str(fixture)], check=True)
fixture_source = fixture.read_text(encoding="utf-8")
assert fixture_source.count('#include "fixture_io.inc"') == 1
fixture.write_text(fixture_source.replace('#include "fixture_io.inc"', '#include "../beam_v1/fixture_io.inc"'), encoding="utf-8")

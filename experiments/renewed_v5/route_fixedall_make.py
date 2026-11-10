from pathlib import Path

directory = Path(__file__).resolve().parent
source = (directory / "main_baseline.cpp").read_text(encoding="utf-8")
start = source.index("namespace fixed_deck {")
end = source.index("// Rebuild a post-promotion suffix", start)
block = source[start:end]


def replace_once(old, new):
    global block
    assert block.count(old) == 1, (old[:100], block.count(old))
    block = block.replace(old, new, 1)


replace_once('using Mask=bitset<K>;', '''using Mask=bitset<K>;
struct UniqueStatistics {
    long long trials=0,nodes=0,subsets=0,wins=0,saved=0,timeouts=0,skipped=0;
    double cpu=0;
};
static UniqueStatistics unique_stats;
static bool unique_enabled(){static const bool value=search_budget::integer("ROUTE_FIXED_ALL",0)!=0;return value;}''')
replace_once('''    int limit,nodes=0,subsets=0;
    Work(const vector<uint64_t>& ink,const vector<int>& start,const vector<int>& goal,int limit):
           ink(ink),start(start),goal(goal),limit(limit){''', '''    int limit,nodes=0,subsets=0;
    clock_t deadline=0;bool stopped=false;
    bool expired(){if(deadline>0 && clock()>=deadline)stopped=true;return stopped;}
    Work(const vector<uint64_t>& ink,const vector<int>& start,const vector<int>& goal,int limit,clock_t deadline=0):
           ink(ink),start(start),goal(goal),limit(limit),deadline(deadline){''')
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
    Work w(r.opaque,start,r.sim.order,int(moves.size())-1);
    if(w.search({},0))moves=move(w.answer);
}''', '''static void optimize(const OnlineRouter& r,const vector<int>& start,vector<pair<int,int>>& moves,clock_t deadline=0){
    if(moves.size()<2||moves.size()>22)return;
    Mask seen;bool repeated=false;
    for(auto op:moves){repeated|=seen.test(op.first);seen.set(op.first);}
    if(repeated) {
        Work w(r.opaque,start,r.sim.order,int(moves.size())-1);
        if(w.search({},0))moves=move(w.answer);
        return;
    }
    if(!unique_enabled())return;
    static const double budget=search_budget::seconds(std::max(0.0,search_budget::real("ROUTE_FIXED_CPU",.15)));
    auto& log=unique_stats;
    if(log.cpu>=budget || routing_budget::expired(deadline)){log.skipped++;return;}
    const clock_t begun=clock();
    const clock_t stop=begun+clock_t(std::max(0.0,budget-log.cpu)*CLOCKS_PER_SEC);
    Work w(r.opaque,start,r.sim.order,int(moves.size())-1,deadline>0?std::min(deadline,stop):stop);
    log.trials++;
    if(w.search({},0)) {
        log.wins++;log.saved+=int(moves.size())-int(w.answer.size());
        moves=move(w.answer);
    }
    log.nodes+=w.nodes;log.subsets+=w.subsets;log.timeouts+=w.stopped;
    log.cpu+=double(clock()-begun)/CLOCKS_PER_SEC;
}''')
source = source[:start] + block + source[end:]
anchor = 'fixed_deck::optimize(actual,trace_start,moves);'
assert source.count(anchor) == 1
source = source.replace(anchor, 'fixed_deck::optimize(actual,trace_start,moves,actual.route_deadline);')
anchor = '    cerr<<"FINAL_DECK saved="<<end_deck_saved<<"\\n";'
assert source.count(anchor) == 1
source = source.replace(anchor, anchor + r'''
    const auto&fu=fixed_deck::unique_stats;
    cerr<<"ROUTE_FIXED_ALL enabled="<<fixed_deck::unique_enabled()<<" trials="<<fu.trials
        <<" nodes="<<fu.nodes<<" subsets="<<fu.subsets<<" wins="<<fu.wins<<" saved="<<fu.saved
        <<" timeouts="<<fu.timeouts<<" skipped="<<fu.skipped<<" cpu="<<fu.cpu<<"\n";
''')
(directory / "route_fixedall.cpp").write_text(source, encoding="utf-8")

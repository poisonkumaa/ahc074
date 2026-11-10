"""Lazily build exact backing-distance rows and bound pair68 inner work.

PAIRS_LAZY=0 restores eager construction; PAIRS_TIME_GUARD=0 restores the
original outer-plan-only deadline checks. PAIRS_BUDGET is in native CPU seconds
and defaults to the original .060. PAIRS_VERIFY=1 independently BFS-checks each
used distance row. No candidate order or distance approximation is introduced.
"""
from pathlib import Path
import hashlib
import subprocess
import sys

here = Path(__file__).resolve().parent
source = here / 'alternative_both.cpp'
assert hashlib.sha256(source.read_bytes()).hexdigest() == '93fe9f99aec1c9f3a8cb6e1c49a3b3df723f85b5bf01bb686ef63ea211c3eeb5'
s = source.read_text(encoding='utf-8')
begin = s.index('namespace pair68 {')
end = s.index('namespace headvalue64 {', begin)
part = s[begin:end]

def once(old, new):
    global part
    assert part.count(old) == 1, (old[:100], part.count(old))
    part = part.replace(old, new, 1)

once('''static long long calls=0,pairs=0,goals=0,reached=0,wins=0,saved=0,backsave=0,ordsave=0,loans=0,spent=0;
''', '''static long long calls=0,pairs=0,goals=0,reached=0,wins=0,saved=0,backsave=0,ordsave=0,loans=0,spent=0;
static long long distance_rows=0,distance_verified=0,deadline_stops=0;
static clock_t distance_ticks=0;
''')
once('''    static const bool on=!getenv("NO_PAIR68");
    if(!on || r.prod_to_pair_idx[a]<0 || ops.size()>17 || ops.size()<4 || clock()>=end || spent>clock_t(.060*CLOCKS_PER_SEC))return;
''', '''    static const bool on=!getenv("NO_PAIR68");
    static const bool lazy=search_budget::integer("PAIRS_LAZY",1)!=0;
    static const bool guard=search_budget::integer("PAIRS_TIME_GUARD",1)!=0;
    static const bool verify=search_budget::integer("PAIRS_VERIFY",0)!=0;
    static const clock_t allowance=clock_t(max(0.,search_budget::real("PAIRS_BUDGET",.060))*CLOCKS_PER_SEC);
    if(!on || !r.graph || r.prod_to_pair_idx[a]<0 || ops.size()>17 || ops.size()<4 || clock()>=end || spent>allowance)return;
''')
old = '''    static vector<shift68::Core> cores;static vector<uint8_t> distance;
    const auto& tab=r.graph->table;int S=tab.states;array<uint8_t,9> bo{};
    int nb=0;for(int k:start)if(k<B)bo[nb++]=k;
    const int src=tab.rank(bo);
    if(cores.empty()){
        cores.resize(S);distance.resize(S*S);
        for(int j=0;j<S;j++){
            auto& c=cores[j];for(int i=0;i<B;i++){int k=tab.permutations[j][i];c.ord[i]=k;c.color|=r.black[k]&~c.prefix[i];c.prefix[i+1]=c.prefix[i]|r.opaque[k];}
            array<uint8_t,9> pos{},relative{};for(int i=0;i<B;i++)pos[tab.permutations[j][i]]=i;
            for(int t=0;t<S;t++){for(int i=0;i<B;i++)relative[i]=pos[tab.permutations[t][i]];distance[j*S+t]=tab.distance[tab.rank(relative)];}
        }
    }
'''
new = '''    static vector<shift68::Core> cores;static vector<uint8_t> distance,ready,checked;
    const auto& tab=r.graph->table;int S=tab.states;array<uint8_t,9> bo{};
    int nb=0;for(int k:start)if(k<B)bo[nb++]=k;
    const int src=tab.rank(bo);
    auto build_row=[&](int j){
        array<uint8_t,9> pos{},relative{};
        for(int i=0;i<B;i++)pos[tab.permutations[j][i]]=i;
        for(int t=0;t<S;t++){
            for(int i=0;i<B;i++)relative[i]=pos[tab.permutations[t][i]];
            distance[j*S+t]=tab.distance[tab.rank(relative)];
        }
        ready[j]=1;++distance_rows;
    };
    if(cores.empty()){
        cores.resize(S);distance.resize(S*S);ready.assign(S,0);checked.assign(S,0);
        for(int j=0;j<S;j++){
            auto& c=cores[j];for(int i=0;i<B;i++){int k=tab.permutations[j][i];c.ord[i]=k;c.color|=r.black[k]&~c.prefix[i];c.prefix[i+1]=c.prefix[i]|r.opaque[k];}
        }
        if(!lazy){const clock_t t=clock();for(int j=0;j<S;j++)build_row(j);distance_ticks+=clock()-t;}
    }
    if(!ready[src]){const clock_t t=clock();build_row(src);distance_ticks+=clock()-t;}
    if(verify && !checked[src]){
        // Independent graph traversal from the actual labelled source order.
        // The lowest backing is not selectable: preserve that rule in the BFS.
        vector<uint8_t> expected(S,255);vector<int> queue;queue.reserve(S);
        expected[src]=0;queue.push_back(src);
        for(size_t h=0;h<queue.size();h++){
            int from=queue[h];
            for(int i=0;i<B-1;i++)for(int j=0;j<B;j++)if(i!=j){
                auto order=tab.permutations[from];uint8_t k=order[i];
                if(i<j)for(int z=i;z<j;z++)order[z]=order[z+1];
                else for(int z=i;z>j;z--)order[z]=order[z-1];
                order[j]=k;int to=tab.rank(order);
                if(expected[to]==255){expected[to]=expected[from]+1;queue.push_back(to);}
            }
        }
        assert(int(queue.size())==S);
        for(int t=0;t<S;t++){assert(distance[src*S+t]==expected[t]);++distance_verified;}
        checked[src]=1;
    }
    bool stopped=false;
    auto out_of_time=[&](){
        if(stopped)return true;
        if(!guard)return false;
        const clock_t at=clock();
        if(at>=end || spent+at-now>allowance){++deadline_stops;stopped=true;}
        return stopped;
    };
'''
once(old,new)
once('''        if(clock()>=end || spent+clock()-now>clock_t(.060*CLOCKS_PER_SEC))break;''',
     '''        if(stopped || clock()>=end || spent+clock()-now>allowance)break;''')
once('''        for(int zc=0;zc<S;zc++){
''', '''        for(int zc=0;zc<S && !stopped;zc++){
            if((zc&31)==0 && out_of_time())break;
''')
once('''            for(int dl=B;dl>=0;dl--){''', '''            for(int dl=B;dl>=0 && !stopped;dl--){''')
once('''                for(int du=dl;du>=0;du--){''', '''                for(int du=dl;du>=0 && !stopped;du--){''')
once('''                    for(unsigned mask=0;mask<(1u<<B);mask++){''', '''                    for(unsigned mask=0;mask<(1u<<B) && !stopped;mask++){''')
once('''                        ++goals;fixed_deck::Work w(r.opaque,start,goal,n);if(!w.orderable(set))continue;''', '''                        if(out_of_time())break;
                        ++goals;fixed_deck::Work w(r.opaque,start,goal,n);if(!w.orderable(set))continue;''')
s = s[:begin] + part + s[end:]
needle = '    cerr<<"PAIR68 '
assert s.count(needle) == 1
log = '    cerr<<"PAIR_LAZY rows="<<pair68::distance_rows<<" verified="<<pair68::distance_verified<<" guards="<<pair68::deadline_stops<<" distance_cpu="<<double(pair68::distance_ticks)/CLOCKS_PER_SEC<<"\\n";\n'
s = s.replace(needle, log + needle, 1)
target = here / 'alternative_lazy.cpp'
target.write_text(s, encoding='utf-8')
subprocess.run([sys.executable,str(here/'make_fixture.py'),str(target),str(here/'alternative_lazy_fixture.cpp')],check=True)
print(hashlib.sha256(target.read_bytes()).hexdigest())

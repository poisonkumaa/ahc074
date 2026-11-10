"""Final-request display-only shortening across the entire raw route reservoir."""
from pathlib import Path
import hashlib
import subprocess
import sys

here = Path(__file__).resolve().parent
source = here / 'future_last.cpp'
assert hashlib.sha256(source.read_bytes()).hexdigest() == '20bd583066425bf42bba2e2ef29353be92318e413dabf9eddb29c0c408578af0'
s = source.read_text(encoding='utf-8')

def once(old, new):
    global s
    assert s.count(old) == 1, (old[:90], s.count(old))
    s = s.replace(old, new, 1)

once('''void optimize(OnlineRouter& r,const vector<int>& start,vector<pair<int,int>>& moves,int product){
    static const bool enabled=search_budget::integer("LAST_REQUEST",1)!=0;
    if(!enabled || moves.empty())return;
    const clock_t began=clock(),deadline=began+clock_t(.008*CLOCKS_PER_SEC);''', '''void optimize(OnlineRouter& r,const vector<int>& start,vector<pair<int,int>>& moves,int product,clock_t deadline,int max_deleted){
    if(moves.empty())return;
    const clock_t began=clock();''')
once('''deleted<=min(4,m)''', '''deleted<=min(max_deleted,m)''')

wrapper = r'''
namespace last_request_wide {
long long paths=0,valid_paths=0,wins=0,saved=0;double cpu=0;
bool enabled(){static const bool on=search_budget::integer("LAST_REQUEST",1)!=0;return on;}
void optimize(OnlineRouter& r,const vector<int>& start,vector<pair<int,int>>& moves,
              const vector<vector<pair<int,int>>>& raw,int product){
    if(!enabled() || moves.empty())return;
    const auto began=clock(),deadline=began+clock_t(.008*CLOCKS_PER_SEC);
    const int original_size=int(moves.size());
    struct Plan{vector<pair<int,int>> ops;vector<int> order;};
    vector<Plan> plans;plans.push_back({moves,r.sim.order});
    auto append=[&](const vector<pair<int,int>>& ops){
        for(const auto& p:plans)if(p.ops==ops)return;
        ++paths;auto order=start;
        for(auto [k,d]:ops){
            if(k<0 || k>=K || d<0 || d>=K)return;
            auto it=find(order.begin(),order.end(),k);
            uint64_t ink=r.opaque[k];for(auto jt=order.begin();jt!=it && ink;++jt)ink&=~r.opaque[*jt];
            if(!ink)return;
            order.erase(it);order.insert(order.begin()+d,k);
        }
        uint64_t seen=0,bits=0;
        for(int k:order){bits|=r.black[k]&~seen;seen|=r.opaque[k];if(seen==~0ULL)break;}
        if(bits!=r.A[product])return;
        ++valid_paths;plans.push_back({ops,move(order)});
    };
    for(const auto& ops:raw)append(ops);
    auto best=moves;auto best_order=r.sim.order;
    // Every raw endpoint is valid for the final request regardless of its
    // queue value, support count, or compatibility with a following request.
    for(const auto& p:plans)if(p.ops.size()<best.size()){best=p.ops;best_order=p.order;}
    const int n=int(plans.size());
    // Give each distinct candidate a share of the first half of the budget.
    // Single deletions repeat to a fixed point; then focus the remaining time
    // on coordinated omissions from the shortest resulting legal trace.
    for(int i=0;i<n && clock()<deadline;i++){
        auto& p=plans[i];r.sim.order=p.order;r.sim.rebuild_positions();
        const clock_t local=min(deadline,began+clock_t(.004*CLOCKS_PER_SEC*(i+1)/n));
        final_request::optimize(r,start,p.ops,product,local,1);
        if(p.ops.size()<best.size()){best=p.ops;best_order=r.sim.order;++wins;}
    }
    r.sim.order=move(best_order);r.sim.rebuild_positions();moves=move(best);
    final_request::optimize(r,start,moves,product,deadline,4);
    saved+=original_size-int(moves.size());
    cpu+=double(clock()-began)/CLOCKS_PER_SEC;
}
}
'''
once('int main() {', wrapper + '\nint main() {')
once('''        auto moves=actual.query(a);
        actual.save_second=false;''', '''        auto moves=actual.query(a);
        vector<vector<pair<int,int>>> last_request_raw;
        if(q==R-1 && last_request_wide::enabled()){
            last_request_raw.reserve(actual.reservoir.size()+3);
            last_request_raw.push_back(moves);
            for(const auto& candidate:actual.reservoir)last_request_raw.push_back(candidate.moves);
            if(!actual.second_moves.empty())last_request_raw.push_back(actual.second_moves);
            if(!actual.third_moves.empty())last_request_raw.push_back(actual.third_moves);
        }
        actual.save_second=false;''')
once('''        if(q==R-1)final_request::optimize(actual,trace_start,moves,a);''', '''        if(q==R-1)last_request_wide::optimize(actual,trace_start,moves,last_request_raw,a);''')
once('''    cerr<<"LAST_REQUEST trials="''', '''    cerr<<"LAST_WIDE paths="<<last_request_wide::paths<<" valid="<<last_request_wide::valid_paths<<" wins="<<last_request_wide::wins<<" saved="<<last_request_wide::saved<<" cpu="<<last_request_wide::cpu<<"\\n";
    cerr<<"LAST_REQUEST trials="''')
once('''<<" saved="<<final_request::saved''', '''<<" path_saved="<<final_request::saved''')
target = here / 'future_lastwide.cpp'
target.write_text(s, encoding='utf-8')
subprocess.run([sys.executable, str(here.parent/'beam_v1/make_fixture.py'), str(target), str(here/'future_lastwide_fixture.cpp')], check=True)
print(hashlib.sha256(target.read_bytes()).hexdigest())

"""The final request needs a legal display, but no reusable following deck."""
from pathlib import Path
import hashlib
import subprocess
import sys

here = Path(__file__).resolve().parent
source = here / 'main_baseline.cpp'
assert hashlib.sha256(source.read_bytes()).hexdigest() == '407a5355d6a75bbe07bd332a265fcb694b532dc7f35de0dee465006bf136a7e7'
s = source.read_text(encoding='utf-8')
helper = r'''
namespace final_request {
long long trials=0,accepted=0,saved=0,rejected=0;double cpu=0;
struct Action{int k,left,right,depth;};
void optimize(OnlineRouter& r,const vector<int>& start,vector<pair<int,int>>& moves,int product){
    static const bool enabled=search_budget::integer("LAST_REQUEST",1)!=0;
    if(!enabled || moves.empty())return;
    const clock_t began=clock(),deadline=began+clock_t(.008*CLOCKS_PER_SEC);
    const auto original=moves;
    const uint64_t target=r.A[product];
    array<uint8_t,K> initial{},initial_pos{},best_order{};
    for(int i=0;i<K;i++){initial[i]=uint8_t(start[i]);initial_pos[start[i]]=uint8_t(i);}
    auto display=[&](const auto& order){
        uint64_t seen=0,bits=0;
        for(int k:order){bits|=r.black[k]&~seen;seen|=r.opaque[k];if(seen==~0ULL)break;}
        return bits;
    };
    auto best=moves;
    for(int i=0;i<K;i++)best_order[i]=uint8_t(r.sim.order[i]);
    if(display(initial)==target){best.clear();best_order=initial;}
    bool timeout=false;
    while(!best.empty() && clock()<deadline){
        const auto reference_moves=best;
        const int m=int(reference_moves.size());
        vector<Action> actions;actions.reserve(m);
        auto reference=start;
        for(auto [k,d]:reference_moves){
            auto it=find(reference.begin(),reference.end(),k);
            reference.erase(it);reference.insert(reference.begin()+d,k);
            actions.push_back({k,d?reference[d-1]:-1,d+1<K?reference[d+1]:-1,d});
        }
        auto trial=[&](uint32_t omitted,int mode){
            if(timeout)return;
            if((trials&15)==0 && clock()>=deadline){timeout=true;return;}
            ++trials;
            auto order=initial,pos=initial_pos;
            vector<pair<int,int>> result;result.reserve(m);
            for(int i=0;i<m;i++){
                if(i<32 && (omitted>>i&1))continue;
                const auto x=actions[i];int from=pos[x.k],depth;
                uint64_t ink=r.opaque[x.k];
                for(int j=0;j<from && ink;j++)ink&=~r.opaque[order[j]];
                // Deleting an earlier move can make a later move invisible.
                // It cannot be emitted, but leaving the sheet in place is legal.
                if(!ink)continue;
                if(x.depth==0)depth=0;
                else if(mode==3 || (mode==2 && x.k<r.tot_B))depth=x.depth;
                else if(mode==1 && x.left>=0)depth=pos[x.left]+1-int(from<pos[x.left]);
                else if(x.right>=0)depth=pos[x.right]-int(from<pos[x.right]);
                else if(x.left>=0)depth=pos[x.left]+1-int(from<pos[x.left]);
                else depth=0;
                if(depth<0 || depth>=K)return;
                if(depth!=from){
                    if(from<depth)memmove(order.data()+from,order.data()+from+1,size_t(depth-from));
                    else memmove(order.data()+depth+1,order.data()+depth,size_t(from-depth));
                    order[depth]=uint8_t(x.k);
                    for(int j=min(from,depth);j<=max(from,depth);j++)pos[order[j]]=uint8_t(j);
                    result.emplace_back(x.k,depth);
                    if(result.size()>=best.size())return;
                }
                // Final-request evaluation deliberately imposes no main-at-top,
                // support-count, or reusable-shelf structural requirement.
                if(display(order)==target){
                    best=move(result);best_order=order;++accepted;return;
                }
            }
        };
        // A valid original prefix or implicit no-op removal may already win.
        trial(0,3);
        if(m<=21){
            for(int deleted=1;deleted<=min(4,m) && !timeout;deleted++){
                auto enumerate=[&](auto&& self,int at,int left,uint32_t omitted)->void{
                    if(timeout)return;
                    if(!left){for(int mode=0;mode<4 && !timeout;mode++)trial(omitted,mode);return;}
                    for(int i=at;i<=m-left && !timeout;i++)self(self,i+1,left-1,omitted|(uint32_t(1)<<i));
                };
                enumerate(enumerate,0,deleted,0);
            }
        }
        if(best.size()>=reference_moves.size())break;
    }
    if(best.size()<original.size()){
        // Independently replay the proposed output with ordinary vector moves.
        // Keep the original trace if any legality or display check fails.
        auto deck=start;bool valid=true;
        for(auto [k,d]:best){
            if(k<0 || k>=K || d<0 || d>=K){valid=false;break;}
            auto it=find(deck.begin(),deck.end(),k);
            uint64_t ink=r.opaque[k];for(auto jt=deck.begin();jt!=it && ink;++jt)ink&=~r.opaque[*jt];
            if(!ink){valid=false;break;}
            deck.erase(it);deck.insert(deck.begin()+d,k);
        }
        valid=valid && display(deck)==target && equal(deck.begin(),deck.end(),best_order.begin());
        if(valid){saved+=int(original.size())-int(best.size());moves=move(best);r.sim.order=move(deck);r.sim.rebuild_positions();}
        else ++rejected;
    }
    cpu+=double(clock()-began)/CLOCKS_PER_SEC;
}
}
'''
assert s.count('int main() {') == 1
s = s.replace('int main() {', helper + '\nint main() {', 1)
needle = "        cout<<moves.size()<<'\\n';"
assert s.count(needle) == 1
s = s.replace(needle, "        if(q==R-1)final_request::optimize(actual,trace_start,moves,a);\n" + needle, 1)
needle = '    cerr<<"U48 eligible="'
assert s.count(needle) == 1
s = s.replace(needle, '    cerr<<"LAST_REQUEST trials="<<final_request::trials<<" accepted="<<final_request::accepted<<" saved="<<final_request::saved<<" rejected="<<final_request::rejected<<" cpu="<<final_request::cpu<<"\\n";\n' + needle, 1)
target = here / 'future_last.cpp'
target.write_text(s, encoding='utf-8')
subprocess.run([sys.executable, str(here.parent/'beam_v1/make_fixture.py'), str(target), str(here/'future_last_fixture.cpp')], check=True)
print(hashlib.sha256(target.read_bytes()).hexdigest())

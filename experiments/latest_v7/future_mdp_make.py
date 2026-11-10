"""Use an ordinary-shelf MDP correction only in matched endpoint comparisons.

The two routes end at the same product. Shelves containing a live support in
either state are excluded together, so missing-model offsets never influence
the comparison. The same exact or front-two-projected model used for insertion
also supplies a finite-horizon correction for the final ordinary queues.
"""
from pathlib import Path
import hashlib
import subprocess
import sys

here = Path(__file__).resolve().parent
source = here / 'main_baseline.cpp'
assert hashlib.sha256(source.read_bytes()).hexdigest() == '407a5355d6a75bbe07bd332a265fcb694b532dc7f35de0dee465006bf136a7e7'
s = source.read_text(encoding='utf-8')

helper = r'''
    double endpoint_mdp_delta(const RouteState& before,const RouteState& after) const {
        if(bucket_mdp_weight==0 || bucket_mdp_tables.empty() || before.main!=after.main)return 0;
        uint64_t blocked=0;
        for(int k:before.supports)blocked|=1ULL<<paper_bucket[k];
        for(int k:after.supports)blocked|=1ULL<<paper_bucket[k];
        auto value=[&](const RouteState& state){
            array<uint32_t,64> codes{};
            array<uint8_t,64> count{};
            array<array<uint8_t,4>,64> seen{};
            for(int k:state.order)if(k>=tot_B && k!=state.main){
                int w=paper_bucket[k];
                if(w<C || w>=T || (blocked>>w&1) || !bucket_mdp_tables[w])continue;
                int type=bucket_mdp_paper_type[k];
                if(bucket_mdp_projected[w] && seen[w][type]++>=bucket_mdp_projection_cap)continue;
                if(count[w]>=bucket_mdp::max_size){count[w]=bucket_mdp::max_size+1;continue;}
                codes[w]|=uint32_t(type)<<(2*count[w]++);
            }
            double answer=0;
            for(int w=C;w<T;w++)if(!(blocked>>w&1) && bucket_mdp_tables[w]){
                double delta=0;
                if(bucket_mdp_tables[w]->lookup(codes[w],count[w],delta))answer+=delta;
            }
            return bucket_mdp_weight*answer/P;
        };
        return value(after)-value(before);
    }
'''
anchor = '    struct SharedTail{'
assert s.count(anchor) == 1
s = s.replace(anchor, helper + '\n' + anchor, 1)
anchor = '                if(forecast<-.05)choose=true;'
assert s.count(anchor) == 1
s = s.replace(anchor, '''                static const double endpoint_mdp_weight=search_budget::real("ENDPOINT_MDP",0.);
                if(endpoint_mdp_weight!=0)forecast+=endpoint_mdp_weight*actual.endpoint_mdp_delta(first_state,other);
''' + anchor, 1)
target = here / 'future_mdp.cpp'
target.write_text(s, encoding='utf-8')
subprocess.run([sys.executable, str(here.parent/'beam_v1/make_fixture.py'), str(target), str(here/'future_mdp_fixture.cpp')], check=True)
print(hashlib.sha256(target.read_bytes()).hexdigest())

"""Add core-order future cost to every ordinary route-state comparison.

This experiment starts from the frozen native v25 source, without an extra
target pragma. CORE_STATE_WEIGHT defaults to zero. A cached permutation rank
is keyed by the complete core order; its current future value is never cached.
"""
from pathlib import Path
import hashlib
import subprocess
import sys

here=Path(__file__).resolve().parent
baseline=here/'main_baseline.cpp'
assert hashlib.sha256(baseline.read_bytes()).hexdigest()=='a9e5cb6b45ee1562c60721be09bde2d0246ad8eeb95585c7721048d74143d7da'
s=baseline.read_text(encoding='utf-8')


def once(old,new):
    global s
    assert s.count(old)==1,(old[:100],s.count(old))
    s=s.replace(old,new,1)


once('''    double route_queue_weight=0.1,route_drop_weight=.5;
    int route_drop_model=1;''','''    double route_queue_weight=0.1,route_drop_weight=.5;
    double core_state_weight=search_budget::real("CORE_STATE_WEIGHT",0.);
    bool core_state_verify=search_budget::integer("CORE_STATE_VERIFY",0)!=0;
    mutable uint32_t core_state_code=std::numeric_limits<uint32_t>::max();
    mutable int core_state_rank=0;
    mutable long long core_state_calls=0,core_state_ranked=0,core_state_verified=0;
    int route_drop_model=1;''')
once('''        if(route_drop_weight!=0)for(int k:active_supports)cost+=route_drop_weight*(route_drop_model==1 ? 1.0 : support_drop[k][support_cut[k]+1]);
        return cost;''','''        if(route_drop_weight!=0)for(int k:active_supports)cost+=route_drop_weight*(route_drop_model==1 ? 1.0 : support_drop[k][support_cut[k]+1]);
        // This term is separate from the bank-subsequence cache. Changing only
        // the backing order must still change the state valuation immediately.
        if(core_state_weight!=0 && graph && !graph->future_value.empty() && tot_B>=2 && tot_B<=8){
            array<uint8_t,9> core{};int count=0;uint32_t code=0;
            for(int k:sim.order)if(k<tot_B){
                core[count++]=uint8_t(k);code=(code<<3)|uint32_t(k);
                if(count==tot_B)break;
            }
            if(count==tot_B){
                core_state_calls++;
                if(code!=core_state_code){
                    core_state_code=code;core_state_rank=graph->table.rank(core);core_state_ranked++;
                }
                if(core_state_verify){
                    if(core_state_rank!=graph->table.rank(core))std::abort();
                    core_state_verified++;
                }
                cost+=core_state_weight*graph->future_value[core_state_rank];
            }
        }
        return cost;''')
once('''    cerr<<"FINAL_DECK saved="<<end_deck_saved<<"\\n";''','''    cerr<<"CORE_STATE weight="<<actual.core_state_weight<<" calls="<<actual.core_state_calls
        <<" ranked="<<actual.core_state_ranked<<" verified="<<actual.core_state_verified<<"\\n";
    cerr<<"FINAL_DECK saved="<<end_deck_saved<<"\\n";''')
target=here/'core_state.cpp'
target.write_text(s,encoding='utf-8',newline='\n')
fixture=here/'core_state_fixture.cpp'
subprocess.run([sys.executable,str(here.parent/'beam_v1/make_fixture.py'),str(target),str(fixture)],check=True)
s=fixture.read_text(encoding='utf-8')
assert s.count('#include "fixture_io.inc"')==1
fixture.write_text(s.replace('#include "fixture_io.inc"','#include "../beam_v1/fixture_io.inc"'),encoding='utf-8',newline='\n')
print(target.name,hashlib.sha256(target.read_bytes()).hexdigest())

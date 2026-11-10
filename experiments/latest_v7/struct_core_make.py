"""Make the backing-order future value available to raw route and trace search.

The existing learned endpoint critic already has this value as feature 2.
By default its feature 0 and raw-prior contribution retain their old meaning.
"""
from pathlib import Path
import hashlib

here=Path(__file__).resolve().parent
source=here/'main_baseline.cpp'
assert hashlib.sha256(source.read_bytes()).hexdigest()=='407a5355d6a75bbe07bd332a265fcb694b532dc7f35de0dee465006bf136a7e7'
s=source.read_text(encoding='utf-8')

def once(old,new):
    global s
    assert s.count(old)==1,(old[:80],s.count(old))
    s=s.replace(old,new,1)

once('''    double route_queue_weight=0.1,route_drop_weight=.5;
    int route_drop_model=1;''','''    double route_queue_weight=0.1,route_drop_weight=.5;
    double core_state_weight=search_budget::real("CORE_STATE_WEIGHT",1.);
    bool core_state_verify=search_budget::integer("CORE_STATE_VERIFY",0)!=0;
    bool core_state_critic_correct=search_budget::integer("CORE_STATE_CRITIC_CORRECT",1)!=0;
    mutable uint64_t core_state_code=~uint64_t(0);
    mutable int core_state_rank=0;
    mutable long long core_state_calls=0,core_state_ranked=0,core_state_verified=0;
    int route_drop_model=1;''')
once('''    double route_state_value() const {
''','''    double core_order_value() const {
        if(core_state_weight==0 || !graph || graph->future_value.empty())return 0;
        assert(tot_B>=2 && tot_B<=9);
        array<uint8_t,9> core{};int count=0;uint64_t code=0;
        for(int k:sim.order)if(k<tot_B){
            core[count++]=uint8_t(k);code=(code<<4)|uint64_t(k);
            if(count==tot_B)break;
        }
        assert(count==tot_B);++core_state_calls;
        if(code!=core_state_code){
            core_state_code=code;core_state_rank=graph->table.rank(core);++core_state_ranked;
        }
        if(core_state_verify){
            assert(core_state_rank==graph->table.rank(core));++core_state_verified;
        }
        // Cache only the permutation rank: the future table may be refreshed
        // by live feedback without changing the current physical permutation.
        return core_state_weight*graph->future_value[core_state_rank];
    }
    double route_state_value() const {
''')
once('''        if(route_drop_weight!=0)for(int k:active_supports)cost+=route_drop_weight*(route_drop_model==1 ? 1.0 : support_drop[k][support_cut[k]+1]);
        return cost;''','''        if(route_drop_weight!=0)for(int k:active_supports)cost+=route_drop_weight*(route_drop_model==1 ? 1.0 : support_drop[k][support_cut[k]+1]);
        if(core_state_weight!=0)cost+=core_order_value();
        return cost;''')
once('''                        int row=actual.graph->table.rank(core);v[2]=actual.graph->future_value[row];
                    }
                    array<uint8_t,K> loan{};''','''                        int row=actual.graph->table.rank(core);v[2]=actual.graph->future_value[row];
                    }
                    if(actual.core_state_critic_correct && actual.core_state_weight!=0)
                        v[0]-=actual.core_state_weight*v[2];
                    array<uint8_t,K> loan{};''')
once('''                const double prior=alt_value-(first_score-moves.size());
                const double future=.75*avg+.25*prior;''','''                double prior=alt_value-(first_score-moves.size());
                if(actual.core_state_critic_correct && actual.core_state_weight!=0)
                    prior-=actual.core_state_weight*(fb[2]-fa[2]);
                const double future=.75*avg+.25*prior;''')
once('''    cerr<<"FINAL_DECK saved="<<end_deck_saved<<"\\n";''','''    cerr<<"CORE_STATE weight="<<actual.core_state_weight<<" critic_correct="<<actual.core_state_critic_correct
        <<" calls="<<actual.core_state_calls<<" ranked="<<actual.core_state_ranked
        <<" verified="<<actual.core_state_verified<<"\\n";
    cerr<<"FINAL_DECK saved="<<end_deck_saved<<"\\n";''')
target=here/'struct_core.cpp'
target.write_text(s,encoding='utf-8')
print(hashlib.sha256(target.read_bytes()).hexdigest())

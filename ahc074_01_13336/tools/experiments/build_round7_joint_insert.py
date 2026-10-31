from pathlib import Path
d=Path(__file__).resolve().parent;root=d.parents[1]
s=(root/'main.cpp').read_text()
s=s.replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100').replace('#define AHC_REFINEMENT_BUDGET 1.65','#define AHC_REFINEMENT_BUDGET 100')
s=s.replace('int main(){','#include "round7_joint_insert.h"\nint main(int argc,char**argv){joint_insert_limit=argc>1?atoi(argv[1]):10;')
s=s.replace('exact_state_refit(A,papers,1,43,32,-1,0);','exact_state_refit(A,papers,1,43,32,8,0);')
s=s.replace('  auto target=direct_final(order,length,selected,&future);','  auto target=joint_insert_final(order,length,selected,&future);')
s=s.replace('   fallbacks++;order=move(before);operations.clear();','''   joint_insert_fallback++;order=before;operations.clear();target=direct_final(order,length,selected,&future);
   if(!direct_moves(order,length,selected,target,operations)||int(operations.size())!=moves){
   fallbacks++;order=before;operations.clear();''')
s=s.replace('  uint64_t bit=1ULL<<(r%64);','  }\n  uint64_t bit=1ULL<<(r%64);')
s=s.replace(' cerr<<"PathFallbacks="',' cerr<<"JointDP="<<joint_insert_dp<<" Changed="<<joint_insert_changed<<" Strict="<<joint_insert_strict<<" Gain="<<joint_insert_gain<<" JointFallback="<<joint_insert_fallback<<"\\n";cerr<<"PathFallbacks="')
(d/'round7_joint_insert.cpp').write_text(s)

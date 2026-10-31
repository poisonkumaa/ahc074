from pathlib import Path
p=Path(__file__).parent
s=(p/'goal_reward_fast.cpp').read_text()
s=s.replace('#define STRUCTURE2_TRAINING_CPU_LIMIT 0.9','#define STRUCTURE2_TRAINING_CPU_LIMIT 100.0')
s=s.replace('int main(){','#include "greedy_alloc_agent.h"\nint main(int argc,char**argv){')
s=s.replace('train_backbone_dictionary(A,papers,n,6,1,8);','train_greedy_allocation(A,papers,argc>1?atoi(argv[1]):6,argc>2?atoi(argv[2]):32,argc>3?atof(argv[3]):100,argc>4?atoi(argv[4]):0,argc>7?atoi(argv[7]):1);')
s=s.replace('refit_all_prefix_papers(A,papers,1,8);','refit_all_prefix_papers(A,papers,argc>5?atoi(argv[5]):4,argc>6?atoi(argv[6]):32);')
(p/'greedy_alloc_agent.cpp').write_text(s)

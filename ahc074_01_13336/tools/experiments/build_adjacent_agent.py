from pathlib import Path
p=Path(__file__).parent
s=(p/'goal_reward_fast.cpp').read_text()
s=s.replace('#define STRUCTURE2_TRAINING_CPU_LIMIT 0.9','#define STRUCTURE2_TRAINING_CPU_LIMIT 100.0')
s=s.replace('int main(){','#include "adjacent_agent.h"\nint main(int argc,char**argv){')
s=s.replace('refit_all_prefix_papers(A,papers,1,8);','refit_all_prefix_papers(A,papers,argc>6?atoi(argv[6]):4,argc>7?atoi(argv[7]):32);\n AdjacentDesigner adj(A,papers,argc>8?atoi(argv[8]):32);adj.optimize(cols,flip,argc>1?atoi(argv[1]):126,argc>2?atoi(argv[2]):0,argc>3?atoi(argv[3]):0,argc>4?atof(argv[4]):100,argc>9?atoi(argv[9]):0);\n if(argc>5&&atoi(argv[5]))refit_all_prefix_papers(A,papers,atoi(argv[5]),32);')
(p/'adjacent_agent.cpp').write_text(s)

from pathlib import Path
p=Path(__file__).parent
s=(p/'goal_reward_fast.cpp').read_text()
s=s.replace('#define STRUCTURE2_TRAINING_CPU_LIMIT 0.9','#define STRUCTURE2_TRAINING_CPU_LIMIT 100.0')
s=s.replace('int main(){','#include "prototype_agent.h"\nint main(int argc,char**argv){')
s=s.replace('refit_all_prefix_papers(A,papers,1,8);','refit_all_prefix_papers(A,papers,argc>6?atoi(argv[6]):4,argc>7?atoi(argv[7]):32);\n PrototypeDesigner proto(A,papers);proto.optimize(argc>1?atoi(argv[1]):2000,argc>2?atoi(argv[2]):32,argc>3?atof(argv[3]):2,argc>4?atof(argv[4]):1.2,argc>5?atoi(argv[5]):0);')
(p/'prototype_agent.cpp').write_text(s)

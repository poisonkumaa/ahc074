from pathlib import Path
p=Path(__file__).parent
s=(p.parent.parent/'main.cpp').read_text()
s=s.replace('struct MultiModel {','#include "round6_frontier.h"\nstruct MultiModel {')
s=s.replace('train_backbone_dictionary(A,papers,n,6,1,8);','train_frontier_dictionary(A,papers,round6_frontier_amount);')
s=s.replace('refit_all_prefix_papers(A,papers,4,32);','refit_all_prefix_papers(A,papers,round6_frontier_refit,32);')
s=s.replace('int main(){','int main(int argc,char**argv){\n round6_frontier_amount=argc>1?atoi(argv[1]):8;round6_frontier_refit=argc>2?atoi(argv[2]):4;round6_frontier_repetitions=argc>3?atoi(argv[3]):0;')
s=s.replace('exact_state_refit(A,papers,1,43,32,-1,0);','if(round6_frontier_repetitions)exact_state_refit(A,papers,1,43,32,round6_frontier_repetitions,0);')
(p/'round6_frontier.cpp').write_text(s)

from pathlib import Path
p=Path(__file__).parent
s=(p.parent.parent/'main.cpp').read_text()
header=(p/'greedy_alloc_agent.h').read_text()
(p/'round6_greedy.h').write_text(header)
code='''#include "round6_greedy.h"
static int round6_greedy_amount=6,round6_greedy_min=32,round6_greedy_repetitions=8,round6_greedy_bottom=1;
'''
s=s.replace('struct MultiModel {',code+'struct MultiModel {')
s=s.replace('train_backbone_dictionary(A,papers,n,6,1,8);','train_greedy_allocation(A,papers,round6_greedy_amount,32,100,0,round6_greedy_min);')
s=s.replace('int main(){','int main(int argc,char**argv){\n round6_greedy_amount=argc>1?atoi(argv[1]):6;round6_greedy_min=argc>2?atoi(argv[2]):32;round6_greedy_repetitions=argc>3?atoi(argv[3]):8;round6_greedy_bottom=argc>4?atoi(argv[4]):1;')
s=s.replace('exact_state_refit(A,papers,1,43,32,-1,0);','if(round6_greedy_repetitions)exact_state_refit(A,papers,1,round6_greedy_bottom,32,round6_greedy_repetitions,0);')
(p/'round6_greedy.cpp').write_text(s)

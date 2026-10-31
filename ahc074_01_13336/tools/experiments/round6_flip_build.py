from pathlib import Path
p=Path(__file__).parent
s=(p.parent.parent/'main.cpp').read_text()
s=s.replace('struct MultiModel {','#include "round6_flip.h"\nstruct MultiModel {')
s=s.replace('train_backbone_dictionary(A,papers,n,6,1,8);','train_backbone_dictionary(A,papers,n,6,1,8);\n round6_optimize_flips(A,flip,papers,round6_flip_passes,round6_flip_weight);')
s=s.replace('int main(){','int main(int argc,char**argv){\n round6_flip_passes=argc>1?atoi(argv[1]):2;round6_flip_weight=argc>2?atoi(argv[2]):1;round6_flip_repetitions=argc>3?atoi(argv[3]):8;')
s=s.replace('exact_state_refit(A,papers,1,43,32,-1,0);','if(round6_flip_repetitions)exact_state_refit(A,papers,1,43,32,round6_flip_repetitions,0);')
(p/'round6_flip.cpp').write_text(s)

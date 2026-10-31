from pathlib import Path
p=Path(__file__).parent
s=(p.parent.parent/'main.cpp').read_text()
start=s.index('inline void refit_all_prefix_papers(')
end=s.index('// Construct the final permutation',start)
f=s[start:end]
f=f.replace('inline void refit_all_prefix_papers(', 'inline bool round6_refit_level(')
f=f.replace('int passes,int importance)', 'int target,int importance)')
f=f.replace('for(int pass=0;pass<passes;pass++)for(int l=N;l>0;l--)', 'for(int l=target;l>=target;l--)')
f=f.replace('std::cerr<<"refit "<<initial<<" -> "<<distances()<<" accepted="<<accepted<<" rejected="<<rejected<<"\\n";', 'return accepted==1;')
assert 'return accepted==1' in f
s=s.replace('struct MultiModel {',f+'\n#include "round6_merge.h"\nstruct MultiModel {')
s=s.replace('refit_all_prefix_papers(A,papers,4,32);','refit_all_prefix_papers(A,papers,4,32);\n round6_merge_dictionary(A,papers,round6_merge_width,round6_merge_max,round6_merge_refit);')
s=s.replace('int main(){','int main(int argc,char**argv){\n round6_merge_width=argc>1?atoi(argv[1]):2;round6_merge_max=argc>2?atoi(argv[2]):44;round6_merge_refit=argc>3?atoi(argv[3]):0;round6_merge_repetitions=argc>4?atoi(argv[4]):0;')
s=s.replace('exact_state_refit(A,papers,1,43,32,-1,0);','if(round6_merge_repetitions)exact_state_refit(A,papers,1,43,32,round6_merge_repetitions,0);')
(p/'round6_merge.cpp').write_text(s)

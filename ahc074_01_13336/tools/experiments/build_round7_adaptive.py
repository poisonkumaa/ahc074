from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'round7_fast_maskdp.cpp').read_text()
s=s.replace('#define AHC_LEARNING_BUDGET 100','#define AHC_LEARNING_BUDGET 0.5').replace('#define AHC_REFINEMENT_BUDGET 100','#define AHC_REFINEMENT_BUDGET 1.65')
s=s.replace('int main(int argc,char**argv){int reps=argc>1?atoi(argv[1]):8;','int main(){')
s=s.replace('exact_state_refit(A,papers,1,43,32,reps,0);','exact_state_refit(A,papers,1,43,32,-1,0);')
s=s.replace('// CPU guards preserve a feasible dictionary at every completed update.','// CPU guards preserve a feasible dictionary at every completed update.\n// Prefix-DP edge costs use 192-position bit masks. Final-stack insertion uses\n// an exact frequency specialization, with a general fallback checked at runtime.')
(d/'round7_adaptive.cpp').write_text(s)

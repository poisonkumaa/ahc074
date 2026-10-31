from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'round5_sampling_adaptive.cpp').read_text()
s=s.replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100').replace('#define AHC_REFINEMENT_BUDGET 1.65','#define AHC_REFINEMENT_BUDGET 100')
s=s.replace('int main(){','int main(int argc,char**argv){')
s=s.replace(' exact_state_refit(A,papers,1,43,32,-1,0);',' for(int cycle=0;cycle<(argc>1?atoi(argv[1]):3);cycle++){cerr<<"PolicyCycle="<<cycle<<"\\n";exact_state_refit(A,papers,1,argc>3?atoi(argv[3]):43,32,argc>2?atoi(argv[2]):4,0);}')
(d/'round6_policy_iteration.cpp').write_text(s)

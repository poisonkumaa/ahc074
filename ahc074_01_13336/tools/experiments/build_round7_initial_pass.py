from pathlib import Path
d=Path(__file__).resolve().parent
s=(d.parents[1]/'main.cpp').read_text().replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100').replace('#define AHC_REFINEMENT_BUDGET 1.65','#define AHC_REFINEMENT_BUDGET 100')
s=s.replace('MultiModel train_multi_model(', 'static int initial_passes=4;\nMultiModel train_multi_model(')
s=s.replace('refit_all_prefix_papers(A,papers,4,32);','refit_all_prefix_papers(A,papers,initial_passes,32);')
s=s.replace('int main(){','int main(int argc,char**argv){initial_passes=argc>1?atoi(argv[1]):4;int reps=argc>2?atoi(argv[2]):8;')
s=s.replace('exact_state_refit(A,papers,1,43,32,-1,0);','exact_state_refit(A,papers,1,43,32,reps,0);')
(d/'round7_initial_pass.cpp').write_text(s)

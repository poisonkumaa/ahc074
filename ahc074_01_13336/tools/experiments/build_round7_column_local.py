from pathlib import Path
d=Path(__file__).resolve().parent
s=(d.parents[1]/'main.cpp').read_text().replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100').replace('#define AHC_REFINEMENT_BUDGET 1.65','#define AHC_REFINEMENT_BUDGET 100')
s=s.replace('inline void refit_all_prefix_papers(', 'static int column_level=-1;\ninline void refit_all_prefix_papers(')
s=s.replace('int amount=level[l].size();if(!amount)', 'int amount=level[l].size();if(column_level>=0&&l!=column_level){advance(l);continue;}if(!amount)')
s=s.replace(' std::cerr<<"refit "', ' if(column_level<0)std::cerr<<"refit "')
s=s.replace('int main(){', '#include "round7_column_local.h"\nint main(int argc,char**argv){int passes=argc>1?atoi(argv[1]):4,repetitions=argc>2?atoi(argv[2]):1;')
s=s.replace('exact_state_refit(A,papers,1,43,32,-1,0);','exact_state_refit(A,papers,1,43,32,8,0);column_local_search(A,papers,cols,flip,passes,repetitions);')
(d/'round7_column_local.cpp').write_text(s)

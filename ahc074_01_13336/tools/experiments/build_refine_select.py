from pathlib import Path
d=Path(__file__).resolve().parent
h=(d/'refit_cost.h').read_text().replace('refit_all_prefix_papers','refit_cost_papers').replace('std::array<int,65>dp,split','std::array<int,9>dp,split')
h=h.replace('for(int pass=0;pass<passes;pass++)for(int l=N;l>0;l--){','for(int pass=0;pass<passes;pass++)for(int l=N;l>0;l--){if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>STRUCTURE2_TRAINING_CPU_LIMIT)break;')
(d/'refit_cost_fast.h').write_text(h)
s=(d/'structure2_agent_main.cpp').read_text()
s=s.replace('int main(){','#include "refit_cost_fast.h"\n#include "refine_select.h"\nint main(int argc,char**argv){')
s=s.replace(' refit_all_prefix_papers(A,papers,1,8);',' papers=select_refined_dictionary(A,papers,argc>1?atof(argv[1]):0.0,argc>2?atoi(argv[2]):3);')
(d/'refine_select_submit.cpp').write_text(s)

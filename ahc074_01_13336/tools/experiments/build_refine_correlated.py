from pathlib import Path
d=Path(__file__).resolve().parent
h=(d/'refine_select.h').read_text().replace('select_refined_dictionary','select_refined_correlated')
h=h.replace('GlobalPrefix::best_move(order,length,selected);','GlobalPrefix::best_move(order,length,selected,.5,&future,1);')
teacher='''
  std::vector<std::array<uint64_t,4>>future(K);std::mt19937 freq_rng(912367);std::vector<int>tt(A.size());std::iota(tt.begin(),tt.end(),0);std::shuffle(tt.begin(),tt.end(),freq_rng);
  for(int r=0;r<192;r++){
   int v=r<128?freq_rng()%A.size():tt[r-128],dist;auto chosen=dp.solve(order,A[v],dist);std::vector<char>sel(K);for(int k:chosen){sel[k]=true;if(r>=128)future[k][0]|=1ULL<<(r-128);}
   for(int m=0;m<dist;m++){auto[from,to]=GlobalPrefix::best_move(order,length,sel);if(from<0)std::abort();int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);}
  }
  std::iota(order.begin(),order.end(),0);
'''
h=h.replace('PrefixDP dp(64,paper);int total=0;','PrefixDP dp(64,paper);int total=0;'+teacher)
(d/'refine_correlated.h').write_text(h)
s=(d/'policy2_agent_union.cpp').read_text()
s=s.replace('int main(int argc,char**argv){','#include "refit_cost_fast.h"\n#include "refine_correlated.h"\nint main(int argc,char**argv){')
s=s.replace(' refit_all_prefix_papers(A,papers,1,8);',' papers=select_refined_correlated(A,papers,argc>1?atof(argv[1]):0.0,argc>2?atoi(argv[2]):3);')
s=s.replace('int samples=argc>1?atoi(argv[1]):64,words=', 'int samples=64,words=')
(d/'refine_correlated_submit.cpp').write_text(s)

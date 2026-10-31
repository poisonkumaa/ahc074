from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'round5_policy_base.cpp').read_text()
s=s.replace('int main(){','int main(int argc,char**argv){')
s=s.replace('ios::sync_with_stdio(false);cin.tie(nullptr);','ios::sync_with_stdio(false);cin.tie(nullptr);\n int refresh_period=argc>1?atoi(argv[1]):16,refresh_after=argc>2?atoi(argv[2]):0;')
a='  for(int k=0;k<int(papers.size());k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);'
s=s.replace(a,a+'''
  if(refresh_period>0&&r%refresh_period==0&&!refresh_after){
   vector<U64>fresh(papers.size());dp.prepare(order);
   for(int sample=0;sample<64;sample++){int v=(sample*127+(r/refresh_period)*37)%255;if(v>=product)v++;int count;auto teacher=dp.solve_prepared(A[v],count);for(int k:teacher)fresh[k]|=1ULL<<sample;}
   future=move(fresh);for(int k=0;k<int(papers.size());k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);
  }
''')
a='  if(!direct_moves(order,length,selected,target,operations)'
s=s.replace(a,'''
  if(refresh_period>0&&r%refresh_period==0&&refresh_after){
   vector<U64>fresh(papers.size());dp.prepare(target);
   for(int sample=0;sample<64;sample++){int v=(sample*127+(r/refresh_period)*37)%255;if(v>=product)v++;int count;auto teacher=dp.solve_prepared(A[v],count);for(int k:teacher)fresh[k]|=1ULL<<sample;}
   future=move(fresh);for(int k=0;k<int(papers.size());k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);
   chosen=dp.solve(order,A[product],moves);fill(selected.begin(),selected.end(),0);for(int k:chosen)selected[k]=true;target=direct_final(order,length,selected,&future);
  }
'''+a)
(p/'round5_policy_refresh.cpp').write_text(s)

from pathlib import Path
d=Path(__file__).resolve().parent
source=(d/'round4_exact_trie.cpp').read_text()
first=source.index('vector<int> direct_final(')
last=source.index('inline vector<uint64_t> selection_teacher',first)
incremental=(d/'round4_policy_direct_incremental.h').read_text().replace('#pragma once\n','')
source=source[:first]+incremental+'''\ninline vector<int> direct_final(const vector<int>&order,const vector<int>&length,const vector<char>&selected,const vector<uint64_t>*future=nullptr){return GlobalPrefix::final_order_incremental(order,length,selected,future);}\n\n'''+source[last:]
source=source.replace('// Internal public-data simulation only: construct a goal permutation directly.\n// Actual responses use the legal, distance-decreasing moves above.', '// Construct the final permutation for simulations and actual responses.\n// Actual responses also generate and check the legal moves below.')
source=source.replace('#define AHC_LEARNING_BUDGET 1.0','#define AHC_LEARNING_BUDGET 0.5')
source=source.replace('#ifndef AHC_SELECTION_BUDGET\n#define AHC_SELECTION_BUDGET 1.4\n#endif', '#ifndef AHC_REFINEMENT_BUDGET\n#define AHC_REFINEMENT_BUDGET 1.45\n#endif')
source=source.replace('if(!multi_training_finished)return {move(A),move(cols),move(flip),move(papers),{}};auto future=selection_teacher(A,papers);return {move(A),move(cols),move(flip),move(papers),move(future)};', 'return {move(A),move(cols),move(flip),move(papers),{}};')
header=(d/'round4_exact_fast.h').read_text().replace('#pragma once\n','')
header=header.replace(' const int N=64,K=papers.size(),', ' auto expired=[](){return double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>=AHC_REFINEMENT_BUDGET;};\n if(expired())return;\n const int N=64,K=papers.size(),')
header=header.replace('for(int r=0;r<128+M;r++){', 'for(int r=0;r<128+M;r++){\n   if((r&63)==0&&expired())return;')
header=header.replace('for(int e=0;e<M;e++){\n   auto&s=scenarios[e];int v=s.target;auto&t=*tables[e];', 'for(int e=0;e<M;e++){\n   if((e&63)==0&&expired())return;\n   auto&s=scenarios[e];int v=s.target;auto&t=*tables[e];')
header=header.replace('for(int l=N;l>=bottom;l--){', 'for(int l=N;l>=bottom;l--){\n   if(expired())break;')
source=source.replace('#include "round4_exact_fast.h"',header+'\n'+(d/'round4_policy_direct_moves.h').read_text().replace('#pragma once\n',''))
source=source[:source.index('int main(int argc,char**argv){')]+r'''
int main(){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int n,K,P,R;if(!(cin>>n>>K>>P>>R)||n!=64||K!=192||P!=256)return 0;
 vector<U64>original(P);for(int v=0;v<P;v++){string x;cin>>x;if(int(x.size())!=n)return 0;for(int j=0;j<n;j++)if(x[j]=='1')original[v]|=1ULL<<j;}
 structure2_training_origin=clock();multi_training_budget=AHC_LEARNING_BUDGET;
 auto model=train_multi_model(original,12345);auto A=move(model.A);auto cols=move(model.cols);auto flip=move(model.flip);auto papers=move(model.papers);
 exact_state_refit(A,papers,1,43,32,8,0);
 auto future=selection_teacher(A,papers);
 PrefixDP dp(n,papers);dp.public_data(A);vector<int>order(papers.size()),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);
 cerr<<"PrepCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
 for(int k=0;k<K;k++){
  string pattern(n,'.');if(k<int(papers.size()))for(int j=0;j<papers[k].len;j++)pattern[cols[j]]=char('0'+(((papers[k].color>>j)&1)^flip[j]));
  cout<<pattern<<'\n';
 }
 cout.flush();int fallbacks=0;
 for(int r=0;r<R;r++){
  int product;if(!(cin>>product)||product<0||product>=P)return 0;
  for(int k=0;k<int(papers.size());k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);
  int moves;auto chosen=dp.solve(order,A[product],moves);if(moves<0||moves>2*K)return 0;
  vector<char>selected(papers.size());for(int k:chosen)selected[k]=true;
  vector<pair<int,int>>operations;auto before=order;
  auto target=direct_final(order,length,selected,&future);
  if(!direct_moves(order,length,selected,target,operations)||int(operations.size())!=moves){
   fallbacks++;order=move(before);operations.clear();
   for(int i=0;i<moves;i++){
    auto [from,to]=GlobalPrefix::best_move(order,length,selected,.5,&future);if(from<0)return 0;
    int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);operations.push_back({k,to});
   }
  }
  uint64_t bit=1ULL<<(r%64);for(int k=0;k<int(papers.size());k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
  cout<<operations.size()<<'\n';for(auto [k,d]:operations)cout<<k<<' '<<d<<'\n';cout.flush();
 }
 cerr<<"PathFallbacks="<<fallbacks<<" FinishCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
}
'''
comment='''// AHC074: prefix dictionaries fitted to exact legal move counts.
// Initial learning and all simulated requests use only the public 256 products.
// Real requests are read one at a time after flushing the previous response.
// CPU guards preserve a feasible dictionary at every completed update.
'''
source=comment+source[source.index('#pragma GCC optimize'):]
assert '#include "' not in source and 'argc' not in source and 'argv' not in source
(d/'round4_submit.cpp').write_text(source)

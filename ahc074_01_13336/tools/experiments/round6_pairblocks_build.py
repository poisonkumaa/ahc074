from pathlib import Path
root=Path(__file__).resolve().parents[2]
text=(root/'main.cpp').read_text();text=text[:text.index('int main(){')]
text+=r'''
#include "round6_pairblocks.h"
#include "round6_pairblocks_train.h"
#include "round6_general_future.h"
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int n,K,P,R;if(!(cin>>n>>K>>P>>R)||n!=64||K!=192||P!=256)return 0;
 vector<U64>original(P);for(int v=0;v<P;v++){string x;cin>>x;if(int(x.size())!=n)return 0;for(int j=0;j<n;j++)if(x[j]=='1')original[v]|=1ULL<<j;}
 int first_pair=argc>1?atoi(argv[1]):0,repetitions=argc>2?atoi(argv[2]):8,teacher=argc>3?atoi(argv[3]):0;assert(first_pair>=0&&first_pair<=64&&first_pair%2==0);
 structure2_training_origin=clock();multi_training_budget=AHC_LEARNING_BUDGET;
 auto model=train_multi_model(original,12345);auto A=move(model.A);auto cols=move(model.cols);auto flip=move(model.flip);auto papers=move(model.papers);
 exact_state_refit(A,papers,1,43,32,repetitions,0);
 int train=argc>4?atoi(argv[4]):0;
 if(train){
  bool seen[65]{};int extra63=-1,extra64=-1;
  for(int k=0;k<K;k++){int l=papers[k].len;if(!seen[l]){seen[l]=true;papers[k].color=PrefixDP::low(l-1);}else{if(l==63)extra63=k;if(l==64)extra64=k;}}
  assert(extra63>=0&&extra64>=0);papers[extra63].color=PrefixDP::low(63);papers[extra64].color=PrefixDP::low(64);
 }
 PairBlocksDP dp(papers,first_pair);if(!dp.public_data(A))return 3;if(train){train_pairblocks(dp,A,train);papers=dp.papers;}
 vector<int>order(K);iota(order.begin(),order.end(),0);auto future=selection_teacher(A,papers);
 auto visible=[&](){vector<char>result(K);U64 covered=0;for(int k:order){result[k]=bool(dp.mask[k]&~covered);covered|=dp.mask[k];}return result;};
 auto display=[&](){U64 covered=0,value=0;for(int k:order){value|=papers[k].color&dp.mask[k]&~covered;covered|=dp.mask[k];}return value;};
 long long all_potential=0,all_moves=0;mt19937 rng(7171237);
 auto solve=[&](int target,int round){
  for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);
  int potential;vector<int>length;auto chosen=dp.solve(order,target,potential,length);vector<char>selected(K);for(int k:chosen)selected[k]=true;
  if(potential!=GlobalPrefix::fixed_distance(order,length,selected)){cerr<<"DISTANCE MISMATCH "<<potential<<" "<<GlobalPrefix::fixed_distance(order,length,selected)<<"\n";abort();}
  vector<pair<int,int>>operations;all_potential+=potential;
  while(GlobalPrefix::fixed_distance(order,length,selected)){
   auto vis=visible();auto[from,to]=GeneralPrefixFuture::best_move(order,length,selected,vis,length,.5,&future);
   if(from<0||!vis[order[from]]){cerr<<"STUCK\n";abort();}
   int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);operations.push_back({k,to});all_moves++;
  }
  if(display()!=A[target]||operations.size()>384){cerr<<"INVALID r="<<round<<" diff="<<__builtin_popcountll(display()^A[target])<<"\n";abort();}
  U64 bit=1ULL<<(round%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
  return operations;
 };
 for(int r=0;r<teacher;r++)solve(rng()%P,r);iota(order.begin(),order.end(),0);all_moves=all_potential=0;
 cerr<<"PrepCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<" first_pair="<<first_pair<<"\n";
 for(int k=0;k<K;k++){string pattern(n,'.');for(int j=0;j<n;j++)if(dp.mask[k]>>j&1)pattern[cols[j]]=char('0'+(((papers[k].color>>j)&1)^flip[j]));cout<<pattern<<'\n';}cout.flush();
 for(int r=0;r<R;r++){
  int product;if(!(cin>>product)||product<0||product>=P)return 0;auto operations=solve(product,r);
  cout<<operations.size()<<'\n';for(auto[k,d]:operations)cout<<k<<' '<<d<<'\n';cout.flush();
 }
 cerr<<"Actual="<<all_moves<<" potential="<<all_potential<<" FinishCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
}
'''
(root/'tools/experiments/round6_pairblocks.cpp').write_text(text)

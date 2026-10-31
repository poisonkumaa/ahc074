#define PREFIX_MASK_CHECK
#define R7_FINAL_CHECK
#define main archived_solver_main
#include "../../main.cpp"
#undef main
int main(){
 mt19937 rng(427612);long long comparisons=0;
 for(int test=0;test<2000;test++){
  int n=1+rng()%64,K=2+rng()%191;vector<PrefixPaper>papers(K);vector<int>length(K),order(K);vector<vector<int>>groups(n+1);
  for(int k=0;k<K;k++){length[k]=k?1+rng()%n:n;papers[k]={length[k],0};groups[length[k]].push_back(k);order[k]=k;}shuffle(order.begin(),order.end(),rng);
  PrefixDP dp(n,papers);dp.prepare(order);
  for(int it=0;it<500;it++){if(it==250){shuffle(order.begin(),order.end(),rng);dp.prepare(order);}int l=1+rng()%n,j=rng()%l,h=rng()%K,q=h+1+rng()%(K-h);dp.mask_cost(j,l,q,h);comparisons++;}
  vector<char>selected(K);for(int l=1;l<=n;l++)if(!groups[l].empty()&&(l==n||rng()%4==0))selected[groups[l][rng()%groups[l].size()]]=true;
  vector<U64>future(K);for(int bit=0;bit<64;bit++)for(int l=1;l<=n;l++)if(!groups[l].empty()&&(l==n||rng()%4==0))future[groups[l][rng()%groups[l].size()]]|=1ULL<<bit;
  auto a=GlobalPrefix::final_order_incremental(order,length,selected,&future);auto b=GlobalPrefix::final_order_incremental(order,length,selected);assert(a.size()==K&&b.size()==K);
  if(test%10==0&&K>1){future[0]|=1;for(int k=1;k<K;k++)if(length[k]==length[0]){future[k]|=1;break;}GlobalPrefix::final_order_incremental(order,length,selected,&future);}
 }
 cout<<"mask_cost_checks="<<comparisons<<" final_calls="<<GlobalPrefix::r7_final_calls<<" final_fallbacks="<<GlobalPrefix::r7_final_fallbacks<<"\n";
}

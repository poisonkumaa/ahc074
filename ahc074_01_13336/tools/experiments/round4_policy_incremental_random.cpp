#define main ignored_solver_main
#include "policy4_agent_direct_compare.cpp"
#undef main
#include "round4_policy_direct_incremental.h"
int main(){mt19937 rng(5631223);for(int test=0;test<100;test++){
 int N=3+rng()%12,K=2*N+5;vector<int>length(K),order(K);for(int k=0;k<K;k++)length[k]=1+rng()%N;length[K-1]=N;iota(order.begin(),order.end(),0);stable_sort(order.begin(),order.end(),[&](int a,int b){return length[a]<length[b];});vector<uint64_t>future(K);vector<vector<int>>level(N+1);for(int k=0;k<K;k++)level[length[k]].push_back(k);
 auto goal=[&](){vector<char>s(K);for(int l=1;l<=N;l++)if(!level[l].empty()&&(l==N||rng()%2))s[level[l][rng()%level[l].size()]]=true;return s;};
 for(int f=0;f<64;f++){auto s=goal();for(int k=0;k<K;k++)if(s[k])future[k]|=1ULL<<f;}
 for(int r=0;r<100;r++){auto selected=goal();int distance=GlobalPrefix::fixed_distance(order,length,selected);auto a=order,b=GlobalPrefix::final_order_incremental(order,length,selected,&future);if(direct_final(order,length,selected)!=GlobalPrefix::final_order_incremental(order,length,selected)){cerr<<"Uniform mismatch";return 5;}
  for(int step=0;step<distance;step++){auto[i,j]=GlobalPrefix::best_move(a,length,selected,.5,&future);if(i<0)return 2;int k=a[i];a.erase(a.begin()+i);a.insert(a.begin()+j,k);}
  if(a!=b){cerr<<"Counterexample test="<<test<<" r="<<r<<" N="<<N<<" K="<<K<<" moves="<<distance<<"\n";return 1;}order=a;uint64_t bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
 }
 }cerr<<"10000 reachable random states exact\n";}

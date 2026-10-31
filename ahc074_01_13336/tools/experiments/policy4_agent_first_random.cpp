#define main ignored_solver_main
#include "policy4_agent_first_compare.cpp"
#undef main
int main(){mt19937 rng(652346);for(int trial=0;trial<10000;trial++){
 int N=3+rng()%10,K=2*N+5;vector<int>length(K),order(K);for(int k=0;k<K;k++)length[k]=1+rng()%N;length[K-1]=N;iota(order.begin(),order.end(),0);shuffle(order.begin(),order.end(),rng);vector<char>selected(K);vector<uint64_t>future(K);vector<vector<int>>level(N+1);for(int k=0;k<K;k++)level[length[k]].push_back(k);
 for(int f=0;f<65;f++)for(int l=1;l<=N;l++)if(!level[l].empty()&&(l==N||rng()%2)){int k=level[l][rng()%level[l].size()];if(f==64)selected[k]=true;else future[k]|=1ULL<<f;}
 int distance=GlobalPrefix::fixed_distance(order,length,selected);auto a=order,b=order;
 for(int step=0;step<distance;step++){auto[i,j]=GlobalPrefix::best_move(a,length,selected,.5,&future);if(i<0){cerr<<"bad base";return 2;}int k=a[i];a.erase(a.begin()+i);a.insert(a.begin()+j,k);auto[u,v]=GlobalPrefix::first_move(b,length,selected,.5,&future);if(u<0){cerr<<"bad first";return 3;}k=b[u];b.erase(b.begin()+u);b.insert(b.begin()+v,k);}
 if(a!=b){cerr<<"Counterexample trial="<<trial<<" N="<<N<<" K="<<K<<" moves="<<distance<<"\n";return 1;}
 }cerr<<"10000 random states exact\n";}

#define main reviewed_solver_main
#include "multi_round_fast_agent.cpp"
#undef main

int main(){
 vector<PrefixPaper>papers{{1,0},{2,1},{3,3},{4,7},{4,15},{3,5}};
 const int N=4,K=papers.size();vector<int>length;for(auto p:papers)length.push_back(p.len);
 vector<U64>A;for(int v=0;v<(1<<N);v++)A.push_back(v);
 PrefixDP dp(N,papers);dp.public_data(A);mt19937_64 rng(123487);vector<U64>future(K);for(auto&x:future)x=rng();for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);
 vector<vector<int>>permutations;map<vector<int>,int>id;vector<int>p(K);iota(p.begin(),p.end(),0);
 do{id[p]=permutations.size();permutations.push_back(p);}while(next_permutation(p.begin(),p.end()));
 int S=permutations.size();vector<vector<int>>reverse(S);vector<U64>display(S);
 auto render=[&](const vector<int>&order){U64 covered=0,result=0;for(int k:order){U64 mask=PrefixDP::low(length[k]);result|=papers[k].color&mask&~covered;covered|=mask;}return result;};
 for(int s=0;s<S;s++){
  const auto&o=permutations[s];display[s]=render(o);int high=0;
  for(int i=0;i<K;i++){int k=o[i];if(length[k]<=high)continue;high=length[k];for(int d=0;d<K;d++)if(d!=i){auto next=o;next.erase(next.begin()+i);next.insert(next.begin()+d,k);reverse[id[next]].push_back(s);}}
 }
 long long checks=0;
 for(U64 target:A){
  vector<int>distance(S,1000),queue;for(int s=0;s<S;s++)if(display[s]==target)distance[s]=0,queue.push_back(s);
  for(int head=0;head<int(queue.size());head++){int s=queue[head];for(int next:reverse[s])if(distance[next]>distance[s]+1)distance[next]=distance[s]+1,queue.push_back(next);}
  for(int s=0;s<S;s++){
   int found;auto goal=dp.solve(permutations[s],target,found);if(found!=distance[s]){cerr<<"DP ERROR "<<target<<" "<<s<<" "<<found<<" "<<distance[s]<<"\n";return 1;}
   int prepared;dp.solve_prepared(target,prepared,false);if(prepared!=found)abort();vector<char>selected(K);for(int k:goal)selected[k]=1;
   auto direct=direct_final(permutations[s],length,selected,&future);auto sorted=direct;sort(sorted.begin(),sorted.end());for(int i=0;i<K;i++)if(sorted[i]!=i)abort();if(render(direct)!=target)abort();
   auto order=permutations[s];for(int step=0;step<found;step++){
    int before=GlobalPrefix::fixed_distance(order,length,selected);auto[from,to]=GlobalPrefix::best_move(order,length,selected,.5,&future);if(from<0||to<0||from>=K||to>=K)abort();int high=0;for(int i=0;i<from;i++)high=max(high,length[order[i]]);int k=order[from];if(length[k]<=high)abort();order.erase(order.begin()+from);order.insert(order.begin()+to,k);if(GlobalPrefix::fixed_distance(order,length,selected)!=before-1)abort();
   }
   if(render(order)!=target)abort();checks++;
  }
 }
 cerr<<"PASS "<<checks<<" exhaustive state/target pairs; sparse exact distance, first move legality, direct_final permutation/display\n";
}

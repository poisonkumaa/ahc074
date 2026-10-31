// Long rollouts use synthetic requests drawn only from the public products.
struct R7State {vector<int>order;vector<U64>future;};
struct R7Simulator {
 const vector<U64>&A;const vector<PrefixPaper>&papers;vector<int>length;PrefixDP dp;
 long long steps=0,fallbacks=0,couplings=0;
 R7Simulator(const vector<U64>&a,const vector<PrefixPaper>&p):A(a),papers(p),dp(64,p){dp.public_data(A);for(auto x:p)length.push_back(x.len);}
 U64 display(const vector<int>&order)const{int high=0;U64 out=0;for(int k:order){out|=papers[k].color&(PrefixDP::low(length[k])^PrefixDP::low(min(high,length[k])));high=max(high,length[k]);}return out;}
 int step(R7State&state,int product,int round,vector<pair<int,int>>*output=nullptr){
  steps++;int K=papers.size();for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(state.future[k]);int moves;auto chosen=dp.solve(state.order,A[product],moves);vector<char>selected(K);for(int k:chosen)selected[k]=true;
  auto before=state.order,target=direct_final(state.order,length,selected,&state.future);vector<pair<int,int>>ops;
  if(!direct_moves(state.order,length,selected,target,ops)||int(ops.size())!=moves){fallbacks++;state.order=move(before);ops.clear();for(int i=0;i<moves;i++){auto[from,to]=GlobalPrefix::best_move(state.order,length,selected,.5,&state.future);assert(from>=0);int k=state.order[from];state.order.erase(state.order.begin()+from);state.order.insert(state.order.begin()+to,k);ops.push_back({k,to});}}
  U64 bit=1ULL<<(round%64);for(int k=0;k<K;k++)state.future[k]=(state.future[k]&~bit)|(selected[k]?bit:0ULL);
  assert(display(state.order)==A[product]);if(output)*output=move(ops);return moves;
 }
 array<int,3> quality(const R7State&state)const{
  U64 tree[65]{};int invtree[65]{},potential=0,inversions=0,records=0,high=0;
  for(int k:state.order)if(length[k]>high){records++;high=length[k];}
  for(int i=int(state.order.size())-1;i>=0;i--){int k=state.order[i];U64 mask=0;for(int x=length[k];x;x-=x&-x)mask|=tree[x];potential+=__builtin_popcountll(mask);for(int x=length[k]-1;x;x-=x&-x)inversions+=invtree[x];for(int x=length[k];x<=64;x+=x&-x){tree[x]|=state.future[k];invtree[x]++;}}
  return {potential,inversions,records};
 }
};
struct R7Candidate{R7State state;vector<pair<int,int>>extra;array<int,3>quality;int displacement=0;};
inline vector<R7Candidate> r7_candidates(R7Simulator&sim,const R7State&base,int product,int round,int maximum){
 int K=base.order.size();vector<R7Candidate>all;unordered_set<U64>hashes;
 auto add=[&](R7State state,vector<pair<int,int>>extra){if(state.order==base.order)return;U64 hash=0x718921ULL;int displacement=0;for(int i=0;i<K;i++){hash=(hash^U64(state.order[i]+1))*0x100000001b3ULL;displacement+=state.order[i]!=base.order[i];}if(!hashes.insert(hash).second)return;state.future=base.future;auto q=sim.quality(state);all.push_back({move(state),move(extra),q,displacement});};
 int high=0;
 for(int from=0;from<K;from++){
  int k=base.order[from],len=sim.length[k];if(len<=high)continue;high=len;
  auto without=base.order;without.erase(without.begin()+from);U64 background=sim.display(without);int prefix[193]{};for(int i=0;i<K-1;i++)prefix[i+1]=max(prefix[i],sim.length[without[i]]);
  for(int to=0;to<K;to++)if(to!=from){U64 visible=PrefixDP::low(len)&~PrefixDP::low(prefix[to]);U64 shown=(background&~visible)|(sim.papers[k].color&visible);if(shown!=sim.A[product])continue;R7State state=base;state.order=without;state.order.insert(state.order.begin()+to,k);add(move(state),{{k,to}});}
  // A deliberate disturbance followed by exact legal repair can cross more
  // than one of the usual shortest-path terminal arrangements.
  for(int to:{0,K-1})if(to!=from){R7State state=base;state.order=without;state.order.insert(state.order.begin()+to,k);vector<pair<int,int>>repair;int cost=sim.step(state,product,round,&repair);if(cost+1>4)continue;vector<pair<int,int>>extra{{k,to}};extra.insert(extra.end(),repair.begin(),repair.end());add(move(state),move(extra));}
 }
 vector<int>selected;auto pick=[&](auto cmp,int count){vector<int>indices(all.size());iota(indices.begin(),indices.end(),0);stable_sort(indices.begin(),indices.end(),cmp);for(int index:indices)if(find(selected.begin(),selected.end(),index)==selected.end()){selected.push_back(index);if(--count==0)break;}};
 pick([&](int x,int y){return all[x].quality[0]+8*int(all[x].extra.size())<all[y].quality[0]+8*int(all[y].extra.size());},3);
 pick([&](int x,int y){return all[x].quality[1]<all[y].quality[1];},1);
 pick([&](int x,int y){return all[x].quality[2]>all[y].quality[2];},2);
 pick([&](int x,int y){return all[x].displacement>all[y].displacement;},2);
 vector<R7Candidate>answer;for(int index:selected){answer.push_back(move(all[index]));if(int(answer.size())>=maximum)break;}return answer;
}
struct R7Scenario{vector<int>targets;vector<R7State>baseline;vector<int>cost;int total=0;};
inline vector<R7Scenario> r7_scenarios(R7Simulator&sim,const R7State&base,int product,int round,int count,int horizon,U64 seed){
 mt19937_64 rng(seed);vector<R7Scenario>result(count);for(auto&s:result){s.baseline.push_back(base);int previous=product;for(int i=0;i<horizon;i++){int target=rng()%(sim.A.size()-1);if(target>=previous)target++;previous=target;s.targets.push_back(target);auto state=s.baseline.back();int moves=sim.step(state,target,round+i+1);s.cost.push_back(moves);s.total+=moves;s.baseline.push_back(move(state));}}return result;
}
inline double r7_rollout_gain(R7Simulator&sim,const R7Candidate&candidate,const R7Scenario&scenario,int round){
 auto state=candidate.state;int gain=-int(candidate.extra.size());for(int i=0;i<int(scenario.targets.size());i++){gain+=scenario.cost[i]-sim.step(state,scenario.targets[i],round+i+1);if(state.order==scenario.baseline[i+1].order&&state.future==scenario.baseline[i+1].future){sim.couplings++;break;}}return gain;
}

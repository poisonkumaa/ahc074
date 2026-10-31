#pragma once
struct MarginalState{vector<int>order;int current;vector<U64>future;};
struct MarginalTrace{vector<MarginalState>states;vector<int>usage,blockers;};

inline bool marginal_feasible(const vector<U64>&A,const vector<PrefixPaper>&papers){
 vector<vector<int>>levels(65);for(int k=0;k<int(papers.size());k++)if(papers[k].len)levels[papers[k].len].push_back(k);
 for(U64 target:A){U64 reachable=1;bool finish=false;
  for(int l=1;l<=64;l++){
   bool reached=false;for(int k:levels[l]){U64 d=(papers[k].color^target)&PrefixDP::low(l);int start=d?64-__builtin_clzll(d):0;if(reachable&(PrefixDP::low(l)^PrefixDP::low(start))){reached=true;break;}}
   if(reached){if(l==64)finish=true;else reachable|=1ULL<<l;}
  }
  if(!finish)return false;
 }return true;
}

inline MarginalTrace marginal_trace(const vector<U64>&A,const vector<PrefixPaper>&papers,int samples){
 int K=papers.size();PrefixDP dp(64,papers);dp.public_data(A);vector<int>order(K),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);
 vector<U64>future=selection_teacher(A,papers);mt19937 rng(515237);vector<int>targets(A.size());iota(targets.begin(),targets.end(),0);MarginalTrace trace;trace.usage.resize(K);trace.blockers.resize(K);int previous=-1;
 for(int r=0;r<128+samples*32;r++){
  int target=rng()%A.size();while(target==previous)target=rng()%A.size();previous=target;
  for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);int distance;auto goal=dp.solve(order,A[target],distance);vector<char>selected(K);for(int k:goal)selected[k]=1;
  int suffix=65;for(int i=K-1;i>=0;i--){int k=order[i];if(r>=128){trace.usage[k]+=selected[k];trace.blockers[k]+=length[k]>=suffix;}if(selected[k])suffix=min(suffix,length[k]);}
  order=direct_final(order,length,selected,&future);U64 bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
  if(r>=128&&(r-128)%32==31)trace.states.push_back({order,target,future});
 }return trace;
}

inline long long marginal_cost(const vector<U64>&A,const vector<PrefixPaper>&papers,const vector<MarginalState>&states){
 PrefixDP dp(64,papers);dp.public_data(A);long long total=0;vector<int>length;for(auto p:papers)length.push_back(p.len);
 for(auto&s:states){
  for(int k=0;k<int(papers.size());k++)dp.reward[k]=16+2*__builtin_popcountll(s.future[k]);int repair;auto goal=dp.solve(s.order,A[s.current],repair);vector<char>selected(papers.size());for(int k:goal)selected[k]=1;
  auto order=direct_final(s.order,length,selected,&s.future);dp.prepare(order);
  for(int v=0;v<int(A.size());v++)if(v!=s.current){int cost;dp.solve_prepared(A[v],cost,false);total+=cost;}
 }
 return total;
}

inline int marginal_remove(const vector<U64>&A,vector<PrefixPaper>&papers,int samples,int candidates,int count,double use_weight){
 int accepted=0;
 for(int it=0;it<count;it++){
  auto trace=marginal_trace(A,papers,samples);long long initial=marginal_cost(A,papers,trace.states),best=initial;int removed=-1;
  vector<pair<double,int>>rank;for(int k=0;k<int(papers.size());k++)rank.push_back({use_weight*trace.usage[k]-trace.blockers[k],k});sort(rank.begin(),rank.end());
  int tested=0;
  for(auto[priority,k]:rank){auto candidate=papers;candidate[k].len=0;candidate[k].color=0;if(!marginal_feasible(A,candidate))continue;
   long long cost=marginal_cost(A,candidate,trace.states);tested++;if(cost<best){best=cost;removed=k;}
   if(tested>=candidates)break;
  }
  cerr<<"MARGINAL pass="<<it<<" initial="<<initial<<" best="<<best<<" tested="<<tested;
  if(removed<0){cerr<<" no change\n";break;}
  cerr<<" remove="<<removed<<" len="<<papers[removed].len<<" use="<<trace.usage[removed]<<" blocker="<<trace.blockers[removed]<<"\n";papers.erase(papers.begin()+removed);accepted++;
 }
 return accepted;
}

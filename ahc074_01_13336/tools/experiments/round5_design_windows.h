#pragma once
inline void optimize_adjacent_window(vector<U64>&A,vector<PrefixPaper>&papers,vector<int>&cols,vector<int>&flip,int finalists=3,int evaluation_states=64){
 const int K=papers.size(),P=A.size();const auto began=clock();
 struct Snapshot{vector<int>order;int target;};vector<Snapshot>snapshots;long long baseline=0;
 vector<int>length,representative(K,-1);for(int k=0;k<K;k++){length.push_back(papers[k].len);for(int v=0;v<P;v++)if((A[v]&PrefixDP::low(length[k]))==papers[k].color){representative[k]=v;break;}}
 {
  PrefixDP dp(64,papers);dp.public_data(A);vector<int>order(K);iota(order.begin(),order.end(),0);auto future=selection_teacher(A,papers);mt19937 rng(119873);vector<int>targets(P);iota(targets.begin(),targets.end(),0);shuffle(targets.begin(),targets.end(),rng);
  for(int r=0;r<128+P;r++){int target=r<128?rng()%P:targets[r-128];if(r>=128)snapshots.push_back({order,target});for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);int moves;auto goal=dp.solve(order,A[target],moves);if(r>=128)baseline+=moves;vector<char>selected(K);for(int k:goal)selected[k]=1;
   order=direct_final(order,length,selected,&future);U64 bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
  }
 }
 auto exchanged=[](U64 value,int x){U64 bit=((value>>x)^(value>>(x+1)))&1;return value^(bit<<x)^(bit<<(x+1));};
 auto make_candidate=[&](int x){auto data=A;auto design=papers;for(auto&a:data)a=exchanged(a,x);for(int k=0;k<K;k++){if(length[k]>x+1)design[k].color=exchanged(design[k].color,x);else if(length[k]==x+1)design[k].color=data[representative[k]]&PrefixDP::low(length[k]);}return pair{data,design};};
 auto feasible=[&](const vector<U64>&data,const vector<PrefixPaper>&design){vector<vector<int>>levels(65);for(int k=0;k<K;k++)levels[length[k]].push_back(k);for(U64 target:data){U64 reachable=1;bool finish=false;for(int l=1;l<=64;l++){bool reached=false;for(int k:levels[l]){U64 diff=(target^design[k].color)&PrefixDP::low(l);int start=diff?64-__builtin_clzll(diff):0;if(reachable&(PrefixDP::low(l)^PrefixDP::low(start))){reached=true;break;}}if(reached){if(l==64)finish=true;else reachable|=1ULL<<l;}}if(!finish)return false;}return true;};
 vector<pair<long long,int>>proposals;
 for(int x=0;x<63;x++){
  bool known=true;for(int k=0;k<K;k++)if(length[k]==x+1&&representative[k]<0)known=false;if(!known)continue;
  auto[data,design]=make_candidate(x);if(!feasible(data,design))continue;PrefixDP dp(64,design);dp.public_data(data);long long total=0;
  for(auto&s:snapshots){int cost;dp.solve(s.order,data[s.target],cost,false);total+=cost;}
  if(total<baseline)proposals.push_back({total,x});
 }
 sort(proposals.begin(),proposals.end());
 auto evaluate=[&](const vector<U64>&data,const vector<PrefixPaper>&design){
  PrefixDP dp(64,design);dp.public_data(data);vector<int>order(K);iota(order.begin(),order.end(),0);auto future=selection_teacher(data,design);mt19937 rng(392837);long long total=0;int previous=-1;
  for(int r=0;r<128+evaluation_states*8;r++){int target=rng()%P;while(target==previous)target=rng()%P;previous=target;for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);int moves;auto goal=dp.solve(order,data[target],moves);vector<char>selected(K);for(int k:goal)selected[k]=1;
   order=direct_final(order,length,selected,&future);U64 bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
   if(r>=128&&(r-128)%8==7){dp.prepare(order);for(int v=0;v<P;v++)if(v!=target){int cost;dp.solve_prepared(data[v],cost,false);total+=cost;}}
  }return total;
 };
 long long oldscore=evaluate(A,papers),bestscore=oldscore;int winner=-1;
 for(int i=0;i<min(finalists,int(proposals.size()));i++){int x=proposals[i].second;auto[data,design]=make_candidate(x);long long score=evaluate(data,design);cerr<<"window candidate="<<x<<" frozen="<<proposals[i].first-baseline<<" rollout="<<score-oldscore<<"\n";if(score<bestscore)bestscore=score,winner=x;}
 if(winner>=0){auto[data,design]=make_candidate(winner);A=move(data);papers=move(design);swap(cols[winner],cols[winner+1]);swap(flip[winner],flip[winner+1]);}
 cerr<<"window selected="<<winner<<" proposals="<<proposals.size()<<" gain="<<double(oldscore-bestscore)/(evaluation_states*(P-1))<<" CPU="<<double(clock()-began)/CLOCKS_PER_SEC<<"\n";
}

// Only initial public products and internally generated requests are used.
inline void prune_dictionary(const vector<U64>&A,vector<PrefixPaper>&papers,int limit,int sample_count){
 if(limit==0)return;clock_t began=clock();int K=papers.size(),P=A.size();
 PrefixDP dp(64,papers);dp.public_data(A);vector<int>order(K),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);
 struct State{int target,cost;vector<int>order;array<U64,3>selected{};};vector<State>states;
 vector<int>usage(K),gain(K);auto future=selection_teacher(A,papers);mt19937 rng(718921);vector<int>targets(P);iota(targets.begin(),targets.end(),0);shuffle(targets.begin(),targets.end(),rng);
 for(int r=0;r<128+sample_count;r++){
  if(r>=128&&(r-128)%P==0)shuffle(targets.begin(),targets.end(),rng);
  int target=r<128?rng()%P:targets[(r-128)%P],moves;for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);
  auto chosen=dp.solve(order,A[target],moves);vector<char>selected(K);for(int k:chosen)selected[k]=1;
  if(r>=128){
   State state{target,moves,order,{}};for(int k:chosen){state.selected[k/64]|=1ULL<<(k%64);usage[k]++;}states.push_back(move(state));
   bool mandatory[192]{};int suffix=65,pos[192],owner[65],bad[192]{};fill(owner,owner+65,-1);for(int k:chosen)owner[length[k]]=k;
   int root=-1;for(int l=64;l>=0;l--){if(owner[l]>=0)root=owner[l];owner[l]=root;}
   for(int i=K-1;i>=0;i--){int k=order[i];pos[k]=i;mandatory[k]=length[k]>=suffix;if(selected[k])suffix=min(suffix,length[k]);}
   for(int k=0;k<K;k++)if(!selected[k]&&!mandatory[k]&&pos[k]<pos[owner[length[k]]])bad[owner[length[k]]]++;
   for(int k=0;k<K;k++)if(!selected[k]){int own=owner[length[k]];gain[k]+=mandatory[k]||(!mandatory[own]&&bad[own]==1&&pos[k]<pos[own]);}
  }
  order=direct_final(order,length,selected,&future);U64 bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
 }
 vector<array<uint8_t,192>>start(P);for(int v=0;v<P;v++)for(int k=0;k<K;k++){U64 diff=(A[v]^papers[k].color)&PrefixDP::low(length[k]);start[v][k]=diff?64-__builtin_clzll(diff):0;}
 auto feasible=[&](const vector<bool>&removed){for(int v=0;v<P;v++){int reach=0;for(int k=0;k<K;k++)if(!removed[k]&&start[v][k]<=reach)reach=max(reach,length[k]);if(reach<64)return false;}return true;};
 vector<bool>removed(K);vector<int>ranking;for(int drop=0;drop<K;drop++){
  if(gain[drop]==0)continue;removed[drop]=true;bool valid=feasible(removed);removed[drop]=false;if(!valid){gain[drop]=INT_MIN/2;continue;}
  vector<PrefixPaper>candidate;for(int k=0;k<K;k++)if(k!=drop)candidate.push_back(papers[k]);PrefixDP reduced(64,candidate);reduced.public_data(A);
  for(auto&s:states)if(s.selected[drop/64]>>(drop%64)&1){vector<int>order;order.reserve(K-1);for(int k:s.order)if(k!=drop)order.push_back(k-(k>drop));int cost;reduced.solve(order,A[s.target],cost,false);gain[drop]+=s.cost-cost;}
  ranking.push_back(drop);
 }
 sort(ranking.begin(),ranking.end(),[&](int x,int y){if(gain[x]!=gain[y])return gain[x]>gain[y];return x<y;});
 for(int index=0;index<min(40,int(ranking.size()));index++){
  int drop=ranking[index],saving=0;vector<PrefixPaper>candidate;for(int k=0;k<K;k++)if(k!=drop)candidate.push_back(papers[k]);PrefixDP reduced(64,candidate);reduced.public_data(A);
  for(auto&s:states){vector<int>order;order.reserve(K-1);for(int k:s.order)if(k!=drop)order.push_back(k-(k>drop));int cost;reduced.solve(order,A[s.target],cost,false);saving+=s.cost-cost;}
  gain[drop]=saving;
 }
 sort(ranking.begin(),ranking.end(),[&](int x,int y){if(gain[x]!=gain[y])return gain[x]>gain[y];return x<y;});
 int kept=0;for(int k:ranking){if(kept>=max(0,limit)||gain[k]<=0)break;removed[k]=true;if(!feasible(removed)){removed[k]=false;continue;}kept++;}
 cerr<<"PruneCandidates="<<ranking.size()<<" Removed="<<kept<<" CPU="<<double(clock()-began)/CLOCKS_PER_SEC<<"\n";
 for(int i=0;i<min(30,int(ranking.size()));i++){int k=ranking[i];cerr<<"Prune "<<k<<" len="<<length[k]<<" usage="<<usage[k]<<" saving="<<gain[k]<<" removed="<<removed[k]<<"\n";}
 vector<PrefixPaper>out;for(int k=0;k<K;k++)if(!removed[k])out.push_back(papers[k]);papers=move(out);
}

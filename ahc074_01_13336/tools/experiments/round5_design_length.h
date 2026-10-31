#pragma once
// Learn a one-paper length change, including its best color, from public states.
inline void optimize_one_length(const vector<U64>&A,vector<PrefixPaper>&papers,int limit=12,int finalists=3,int evaluation_states=16){
 const int N=64,K=papers.size(),P=A.size(),INF=30000;
 struct Scenario{int target;array<int,192>pos;array<array<uint8_t,193>,66>cnt;};
 vector<Scenario>scenarios(P);vector<int>length,usage(K),blockers(K);for(auto p:papers)length.push_back(p.len);long long baseline=0;
 {
  PrefixDP dp(N,papers);dp.public_data(A);vector<int>order(K);iota(order.begin(),order.end(),0);auto future=selection_teacher(A,papers);mt19937 rng(918367);vector<int>targets(P);iota(targets.begin(),targets.end(),0);shuffle(targets.begin(),targets.end(),rng);
  for(int r=0;r<128+P;r++){
   int target=r<128?rng()%P:targets[r-128];if(r>=128){auto&s=scenarios[r-128];s.target=target;for(int i=0;i<K;i++)s.pos[order[i]]=i;for(int l=0;l<=65;l++){s.cnt[l][0]=0;for(int i=0;i<K;i++)s.cnt[l][i+1]=s.cnt[l][i]+(length[order[i]]>=l);}}
   for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);int moves;auto goal=dp.solve(order,A[target],moves);vector<char>selected(K);for(int k:goal)selected[k]=1;
   if(r>=128){baseline+=moves;int suffix=65;for(int i=K-1;i>=0;i--){int k=order[i];usage[k]+=selected[k];blockers[k]+=length[k]>=suffix;if(selected[k])suffix=min(suffix,length[k]);}}
   order=direct_final(order,length,selected,&future);U64 bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
  }
 }
 vector<array<vector<uint8_t>,64>>candidates(P);
 for(int v=0;v<P;v++)for(int k=0;k<K;k++){U64 diff=(A[v]^papers[k].color)&PrefixDP::low(length[k]);int start=diff?64-__builtin_clzll(diff):0;for(int j=start;j<length[k];j++)candidates[v][j].push_back(k);}
 vector<pair<int,int>>rank;for(int k=0;k<K;k++)if(length[k]>=40)rank.push_back({usage[k]-blockers[k],k});sort(rank.begin(),rank.end());vector<int>chosen;int perlevel[65]{};
 for(auto[priority,k]:rank)if(perlevel[length[k]]<2){perlevel[length[k]]++;chosen.push_back(k);if(int(chosen.size())>=limit)break;}
 struct Proposal{long long cost;int paper,len;U64 color;};vector<Proposal>proposals;
 const auto began=clock();
 for(int z:chosen)for(int change:{-1,1}){
  int new_length=length[z]+change;if(new_length<1||new_length>64)continue;int threshold=max(new_length,length[z]);
  vector<array<int,65>>route(P);vector<int>bypass(P,INF);
  for(int e=0;e<P;e++){
   auto&s=scenarios[e];int v=s.target,position=s.pos[z],minimum_h=position+1;
   auto count=[&](int l,int i){return int(s.cnt[l][i])+(l==threshold&&i>position?change:0);};
   auto cost=[&](int j,int h,int k,int len){int q=s.pos[k];if(q<h)return 0;int moved=count(len,q)-count(len,h),upper=count(j+1,q)-count(j+1,h);return moved+(upper>moved);};
   array<array<uint16_t,193>,65>F,B;array<vector<uint8_t>,65>active;for(auto&r:F)r.fill(INF);F[0][0]=0;active[0].push_back(0);
   for(int j=0;j<N;j++)for(int k:candidates[v][j])if(k!=z){int l=length[k],q=s.pos[k];for(int h:active[j]){int nh=max(q+1,h),value=F[j][h]+cost(j,h,k,l);if(F[l][nh]==INF)active[l].push_back(nh);if(value<F[l][nh])F[l][nh]=value;}}
   for(int h:active[N])bypass[e]=min(bypass[e],int(F[N][h]));B[N].fill(0);
   for(int j=N-1;j>=new_length;j--){B[j].fill(INF);for(int k:candidates[v][j])if(k!=z){int l=length[k],q=s.pos[k];for(int h=minimum_h;h<=K;h++){int value=cost(j,h,k,l)+B[l][max(q+1,h)];if(value<B[j][h])B[j][h]=value;}}}
   auto&rr=route[e];rr.fill(INF);for(int j=new_length-1;j>=0;j--){int value=rr[j+1];for(int h:active[j])value=min(value,F[j][h]+cost(j,h,z,new_length)+B[new_length][max(position+1,h)]);rr[j]=value;}
  }
  struct Node{int child[2]{-1,-1},sample=0;long long weight=0;};vector<Node>trie(1);
  for(int e=0;e<P;e++){int v=scenarios[e].target,x=0;long long previous=0;
   for(int d=1;d<=new_length;d++){int b=A[v]>>(new_length-d)&1;if(trie[x].child[b]<0){int next=trie.size();trie[x].child[b]=next;trie.emplace_back();}x=trie[x].child[b];trie[x].sample=v;int value=route[e][new_length-d];long long gain=value>=INF?0:(bypass[e]>=INF?1000000000000LL+10000-value:max(0,bypass[e]-value));trie[x].weight+=gain-previous;previous=gain;}
  }
  long long best=LLONG_MIN;int sample=-1;for(int x=0;x<int(trie.size());x++){auto&node=trie[x];bool leaf=true;for(int b=0;b<2;b++)if(node.child[b]>=0){trie[node.child[b]].weight+=node.weight;leaf=false;}if(leaf&&node.weight>best)best=node.weight,sample=node.sample;}
  U64 color=A[sample]&PrefixDP::low(new_length);long long total=0;bool valid=true;
  for(int e=0;e<P;e++){U64 diff=(A[scenarios[e].target]^color)&PrefixDP::low(new_length);int start=diff?64-__builtin_clzll(diff):0;int value=min(bypass[e],route[e][start]);total+=value;valid&=value<INF;}
  if(valid&&total<baseline)proposals.push_back({total,z,new_length,color});
 }
 sort(proposals.begin(),proposals.end(),[](auto&a,auto&b){return a.cost<b.cost;});
 auto evaluate=[&](const vector<PrefixPaper>&design){
  PrefixDP dp(N,design);dp.public_data(A);vector<int>order(K),len;for(auto p:design)len.push_back(p.len);iota(order.begin(),order.end(),0);auto future=selection_teacher(A,design);mt19937 rng(712827);long long total=0;int previous=-1;
  for(int r=0;r<128+evaluation_states*8;r++){int target=rng()%P;while(target==previous)target=rng()%P;previous=target;for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);int moves;auto goal=dp.solve(order,A[target],moves);vector<char>selected(K);for(int k:goal)selected[k]=1;
   order=direct_final(order,len,selected,&future);U64 bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
   if(r>=128&&(r-128)%8==7){dp.prepare(order);for(int v=0;v<P;v++)if(v!=target){int cost;dp.solve_prepared(A[v],cost,false);total+=cost;}}
  }return total;
 };
 long long oldscore=evaluate(papers),bestscore=oldscore;int winner=-1;
 for(int i=0;i<min(finalists,int(proposals.size()));i++){auto p=proposals[i];auto candidate=papers;candidate[p.paper]={p.len,p.color};long long score=evaluate(candidate);cerr<<"length candidate="<<p.paper<<" "<<length[p.paper]<<"->"<<p.len<<" frozen="<<p.cost-baseline<<" rollout="<<score-oldscore<<"\n";if(score<bestscore)bestscore=score,winner=i;}
 if(winner>=0){auto p=proposals[winner];papers[p.paper]={p.len,p.color};}
 cerr<<"length selected="<<winner<<" proposals="<<proposals.size()<<" gain="<<double(oldscore-bestscore)/(evaluation_states*(P-1))<<" CPU="<<double(clock()-began)/CLOCKS_PER_SEC<<"\n";
}

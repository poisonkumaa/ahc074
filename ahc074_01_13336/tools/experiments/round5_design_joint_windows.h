#pragma once
inline void optimize_joint_adjacent_window(vector<U64>&A,vector<PrefixPaper>&papers,vector<int>&cols,vector<int>&flip,int finalists=3,int evaluation_states=64){
 const int N=64,K=papers.size(),P=A.size(),INF=30000;auto began=clock();vector<int>length,representative(K,-1);vector<vector<int>>level(65);
 for(int k=0;k<K;k++){length.push_back(papers[k].len);level[length[k]].push_back(k);for(int v=0;v<P;v++)if((A[v]&PrefixDP::low(length[k]))==papers[k].color){representative[k]=v;break;}}
 struct Scenario{int target;array<int,192>pos;array<array<uint8_t,193>,66>cnt;};vector<Scenario>scenarios(P);long long baseline=0;
 {
  PrefixDP dp(N,papers);dp.public_data(A);vector<int>order(K);iota(order.begin(),order.end(),0);auto future=selection_teacher(A,papers);mt19937 rng(119873);vector<int>targets(P);iota(targets.begin(),targets.end(),0);shuffle(targets.begin(),targets.end(),rng);
  for(int r=0;r<128+P;r++){int target=r<128?rng()%P:targets[r-128];if(r>=128){auto&s=scenarios[r-128];s.target=target;for(int i=0;i<K;i++)s.pos[order[i]]=i;for(int l=0;l<=65;l++){s.cnt[l][0]=0;for(int i=0;i<K;i++)s.cnt[l][i+1]=s.cnt[l][i]+(length[order[i]]>=l);}}
   for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);int moves;auto goal=dp.solve(order,A[target],moves);if(r>=128)baseline+=moves;vector<char>selected(K);for(int k:goal)selected[k]=1;
   order=direct_final(order,length,selected,&future);U64 bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
  }
 }
 auto exchanged=[](U64 value,int x){U64 bit=((value>>x)^(value>>(x+1)))&1;return value^(bit<<x)^(bit<<(x+1));};
 vector<array<int,192>>start(P);for(int v=0;v<P;v++)for(int k=0;k<K;k++){U64 d=(A[v]^papers[k].color)&PrefixDP::low(length[k]);start[v][k]=d?64-__builtin_clzll(d):0;}
 struct Table{array<array<uint16_t,193>,65>F,B;array<vector<uint8_t>,65>active;};vector<unique_ptr<Table>>tables;
 for(int e=0;e<P;e++){
  tables.push_back(make_unique<Table>());auto&t=*tables[e];auto&s=scenarios[e];int v=s.target;for(auto&r:t.F)r.fill(INF);t.B[N].fill(0);t.F[0][0]=0;t.active[0].push_back(0);
  for(int j=0;j<N;j++)for(int z=0;z<K;z++)if(length[z]>j&&start[v][z]<=j){int l=length[z],q=s.pos[z];for(int h:t.active[j]){int add=0;if(q>=h){int moved=s.cnt[l][q]-s.cnt[l][h],upper=s.cnt[j+1][q]-s.cnt[j+1][h];add=moved+(upper>moved);}int nh=max(q+1,h),value=t.F[j][h]+add;if(t.F[l][nh]==INF)t.active[l].push_back(nh);if(value<t.F[l][nh])t.F[l][nh]=value;}}
 }
 struct Proposal{long long cost;int boundary;vector<PrefixPaper>papers;};vector<Proposal>proposals;
 for(int L=63;L>=1;L--){
  const int Q=level[L].size(),x=L-1;vector<U64>data=A;for(auto&a:data)a=exchanged(a,x);auto design=papers;bool known=true;
  for(int k=0;k<K;k++){if(length[k]>L)design[k].color=exchanged(design[k].color,x);else if(length[k]==L){if(representative[k]<0)known=false;else design[k].color=data[representative[k]]&PrefixDP::low(L);}}
  vector<int>bypass(P,INF),through(P*Q);vector<array<int,65>>route(P*Q);
  for(int e=0;e<P;e++){
   auto&s=scenarios[e];auto&t=*tables[e];auto&F=t.F;auto&B=t.B;int v=s.target;array<int,193>swapped_suffix;swapped_suffix.fill(INF);B[L].fill(INF);
   auto add=[&](int j,int h,int z){int q=s.pos[z],len=length[z];if(q<h)return 0;int moved=s.cnt[len][q]-s.cnt[len][h],upper=s.cnt[j+1][q]-s.cnt[j+1][h];return moved+(upper>moved);};
   for(int z=0;z<K;z++)if(length[z]>L){int len=length[z],q=s.pos[z];U64 diff=(data[v]^design[z].color)&PrefixDP::low(len);int after=diff?64-__builtin_clzll(diff):0;
    if(start[v][z]<=L||after<=L)for(int h=0;h<=K;h++){int value=add(L,h,z)+B[len][max(q+1,h)];if(start[v][z]<=L&&value<B[L][h])B[L][h]=value;if(after<=L)swapped_suffix[h]=min(swapped_suffix[h],value);}
    if(start[v][z]<L)for(int j=start[v][z];j<L;j++)for(int h:t.active[j])bypass[e]=min(bypass[e],F[j][h]+add(j,h,z)+B[len][max(q+1,h)]);
   }
   for(int q=0;q<Q;q++){int z=level[L][q],position=s.pos[z];auto&rr=route[e*Q+q];rr.fill(INF);for(int j=L-1;j>=0;j--){int value=rr[j+1];for(int h:t.active[j])value=min(value,F[j][h]+add(j,h,z)+swapped_suffix[max(position+1,h)]);rr[j]=value;}
    U64 diff=(data[v]^design[z].color)&PrefixDP::low(L);int st=diff?64-__builtin_clzll(diff):0;through[e*Q+q]=rr[st];
   }
  }
  if(!known||!Q)continue;
  struct Node{int child[2]{-1,-1},sample=0;long long weight=0;};vector<Node>trie(1);vector<array<int,65>>path(P);
  for(int v=0;v<P;v++){int node=0;for(int d=1;d<=L;d++){int bit=data[v]>>(L-d)&1;if(trie[node].child[bit]<0){int next=trie.size();trie[node].child[bit]=next;trie.emplace_back();}node=trie[node].child[bit];trie[node].sample=v;path[v][d]=node;}}
  for(int pass=0;pass<2;pass++)for(int q=0;q<Q;q++){
   int z=level[L][q];for(auto&node:trie)node.weight=0;long long oldcost=0;
   for(int e=0;e<P;e++){int v=scenarios[e].target,base=bypass[e];for(int other=0;other<Q;other++)if(other!=q)base=min(base,through[e*Q+other]);oldcost+=min(base,through[e*Q+q]);long long previous=0;
    for(int d=1;d<=L;d++){int value=route[e*Q+q][L-d];long long gain=value>=INF?0:(base>=INF?1000000000000LL+10000-value:max(0,base-value));trie[path[v][d]].weight+=gain-previous;previous=gain;}
   }
   long long best=LLONG_MIN;int sample=-1;for(int node=0;node<int(trie.size());node++){auto&v=trie[node];bool leaf=true;for(int bit=0;bit<2;bit++)if(v.child[bit]>=0){trie[v.child[bit]].weight+=v.weight;leaf=false;}if(leaf&&v.weight>best)best=v.weight,sample=v.sample;}
   U64 color=data[sample]&PrefixDP::low(L);long long newcost=0;bool valid=true;
   for(int e=0;e<P;e++){int v=scenarios[e].target,base=bypass[e];for(int other=0;other<Q;other++)if(other!=q)base=min(base,through[e*Q+other]);U64 diff=(data[v]^color)&PrefixDP::low(L);int st=diff?64-__builtin_clzll(diff):0,value=min(base,route[e*Q+q][st]);newcost+=value;valid&=value<INF;}
   if(valid&&newcost<oldcost){design[z].color=color;for(int e=0;e<P;e++){U64 diff=(data[scenarios[e].target]^color)&PrefixDP::low(L);int st=diff?64-__builtin_clzll(diff):0;through[e*Q+q]=route[e*Q+q][st];}}
  }
  long long total=0;bool valid=true;for(int e=0;e<P;e++){int value=bypass[e];for(int q=0;q<Q;q++)value=min(value,through[e*Q+q]);total+=value;valid&=value<INF;}
  if(valid&&total<baseline){
#ifdef ROUND5_VERIFY_JOINT
   PrefixDP check(N,design);check.public_data(data);long long checked=0;vector<int>order(K);
   for(auto&s:scenarios){for(int k=0;k<K;k++)order[s.pos[k]]=k;int distance;check.solve(order,data[s.target],distance,false);checked+=distance;}
   if(checked!=total){cerr<<"JOINT CACHE ERROR L="<<L<<" predicted="<<total<<" exact="<<checked<<"\n";abort();}
#endif
   proposals.push_back({total,L,move(design)});
  }
 }
 sort(proposals.begin(),proposals.end(),[](auto&a,auto&b){return a.cost<b.cost;});
 auto evaluate=[&](const vector<U64>&data,const vector<PrefixPaper>&design){
  PrefixDP dp(64,design);dp.public_data(data);vector<int>order(K);iota(order.begin(),order.end(),0);auto future=selection_teacher(data,design);mt19937 rng(392837);long long total=0;int previous=-1;
  for(int r=0;r<128+evaluation_states*8;r++){int target=rng()%P;while(target==previous)target=rng()%P;previous=target;for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);int moves;auto goal=dp.solve(order,data[target],moves);vector<char>selected(K);for(int k:goal)selected[k]=1;
   order=direct_final(order,length,selected,&future);U64 bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
   if(r>=128&&(r-128)%8==7){dp.prepare(order);for(int v=0;v<P;v++)if(v!=target){int cost;dp.solve_prepared(data[v],cost,false);total+=cost;}}
  }return total;
 };
 long long oldscore=evaluate(A,papers),bestscore=oldscore;int winner=-1;
 for(int i=0;i<min(finalists,int(proposals.size()));i++){auto&proposal=proposals[i];auto data=A;for(auto&a:data)a=exchanged(a,proposal.boundary-1);long long score=evaluate(data,proposal.papers);cerr<<"joint window="<<proposal.boundary-1<<" frozen="<<proposal.cost-baseline<<" rollout="<<score-oldscore<<"\n";if(score<bestscore)bestscore=score,winner=i;}
 if(winner>=0){int x=proposals[winner].boundary-1;for(auto&a:A)a=exchanged(a,x);papers=move(proposals[winner].papers);swap(cols[x],cols[x+1]);swap(flip[x],flip[x+1]);}
 cerr<<"joint selected="<<winner<<" proposals="<<proposals.size()<<" gain="<<double(oldscore-bestscore)/(evaluation_states*(P-1))<<" CPU="<<double(clock()-began)/CLOCKS_PER_SEC<<"\n";
}

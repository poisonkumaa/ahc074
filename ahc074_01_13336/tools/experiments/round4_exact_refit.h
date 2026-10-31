#pragma once
// Coordinate refit against exact legal move costs on public-data snapshots.
// Snapshots and their target products are generated internally before real orders.
inline void exact_state_refit(const vector<U64>&A,vector<PrefixPaper>&papers,int passes,int bottom,int strength,int repetitions=1,int regularizer=0){
 const int N=64,K=papers.size(),P=A.size(),M=P*repetitions,INF=100000,unit=regularizer?32:1;
 vector<int>length;vector<vector<int>>level(N+1);for(int k=0;k<K;k++){length.push_back(papers[k].len);level[length[k]].push_back(k);}
 struct Scenario{int target;array<int,192>pos;array<array<uint8_t,193>,66>cnt;};
 vector<Scenario>scenarios(M);
 {
  PrefixDP dp(N,papers);dp.public_data(A);vector<int>order(K);iota(order.begin(),order.end(),0);
  auto future=selection_teacher(A,papers);mt19937 rng(1237189);vector<int>targets(P);iota(targets.begin(),targets.end(),0);shuffle(targets.begin(),targets.end(),rng);
  for(int r=0;r<128+M;r++){
   if(r>128&&(r-128)%P==0)shuffle(targets.begin(),targets.end(),rng);
   int t=r<128?rng()%P:targets[(r-128)%P];
   if(r>=128){auto&s=scenarios[r-128];s.target=t;for(int i=0;i<K;i++)s.pos[order[i]]=i;for(int l=0;l<=N+1;l++){s.cnt[l][0]=0;for(int i=0;i<K;i++)s.cnt[l][i+1]=s.cnt[l][i]+(length[order[i]]>=l);}}
   for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);
   int moves;auto chosen=dp.solve(order,A[t],moves);vector<char>selected(K);for(int k:chosen)selected[k]=true;
   order=direct_final(order,length,selected,&future);U64 bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
  }
 }
 vector<array<int,192>>start(P);auto update=[&](int k){for(int v=0;v<P;v++){U64 d=(A[v]^papers[k].color)&PrefixDP::low(length[k]);start[v][k]=d?64-__builtin_clzll(d):0;}};for(int k=0;k<K;k++)update(k);
 int accepted=0;long long total_gain=0;auto began=clock();
 for(int pass=0;pass<passes;pass++)for(int l=N;l>=bottom;l--){
  int Q=level[l].size();if(!Q)continue;
  vector<int>bypass(M),through(M*Q);vector<array<int,65>>route(M*Q);
  for(int e=0;e<M;e++){
   auto&s=scenarios[e];int v=s.target;
   int F[65][193],B[65][193],active[65][193],count[65]{};fill(&F[0][0],&F[0][0]+65*193,INF);fill(&B[0][0],&B[0][0]+65*193,INF);
   F[0][0]=0;active[0][count[0]++]=0;
   vector<int>cand[64];for(int k=0;k<K;k++)for(int j=start[v][k];j<length[k];j++)cand[j].push_back(k);
   auto add=[&](int j,int h,int z){int q=s.pos[z],len=length[z];if(q<h)return regularizer;int moved=s.cnt[len][q]-s.cnt[len][h],upper=s.cnt[j+1][q]-s.cnt[j+1][h];return unit*(moved+(upper>moved))+regularizer;};
   for(int j=0;j<N;j++)for(int z:cand[j])if(length[z]!=l){int len=length[z],q=s.pos[z];for(int ai=0;ai<count[j];ai++){int h=active[j][ai],nh=max(q+1,h),val=F[j][h]+add(j,h,z);if(F[len][nh]==INF)active[len][count[len]++]=nh;F[len][nh]=min(F[len][nh],val);}}
   bypass[e]=*min_element(F[N],F[N]+K+1);
   fill(B[N],B[N]+K+1,0);
   for(int j=N-1;j>=l;j--)for(int z:cand[j]){int len=length[z],q=s.pos[z];for(int h=0;h<=K;h++)B[j][h]=min(B[j][h],add(j,h,z)+B[len][max(q+1,h)]);}
   for(int q=0;q<Q;q++){int z=level[l][q],pos=s.pos[z];auto&rr=route[e*Q+q];rr.fill(INF);
    for(int j=l-1;j>=0;j--){int value=rr[j+1];for(int ai=0;ai<count[j];ai++){int h=active[j][ai];value=min(value,F[j][h]+add(j,h,z)+B[l][max(pos+1,h)]);}rr[j]=value;}
    through[e*Q+q]=rr[start[v][z]];
   }
  }
  struct Node{int child[2]{-1,-1},sample=0;long long weight=0;};vector<Node>trie(1);vector<array<int,65>>path(P);
  for(int v=0;v<P;v++){int x=0;for(int d=1;d<=l;d++){int b=A[v]>>(l-d)&1;if(trie[x].child[b]<0){int ni=trie.size();trie[x].child[b]=ni;trie.emplace_back();}x=trie[x].child[b];path[v][d]=x;trie[x].sample=v;}}
  for(int q=0;q<Q;q++){
   int z=level[l][q];for(auto&node:trie)node.weight=0;long long oldcost=0;
   for(int e=0;e<M;e++){int v=scenarios[e].target,base=bypass[e];for(int other=0;other<Q;other++)if(other!=q)base=min(base,through[e*Q+other]);oldcost+=min(base,through[e*Q+q]);long long previous=0;
    for(int d=1;d<=l;d++){int value=route[e*Q+q][l-d];long long gain=value>=INF?0:(base>=INF?1000000000000LL+strength*(10000-value):strength*max(0,base-value));trie[path[v][d]].weight+=gain-previous;previous=gain;}
   }
   long long best=LLONG_MIN;int sample=-1;
   for(int x=0;x<int(trie.size());x++){auto&node=trie[x];bool leaf=true;for(int b=0;b<2;b++)if(node.child[b]>=0){trie[node.child[b]].weight+=node.weight;leaf=false;}if(leaf&&node.weight>best)best=node.weight,sample=node.sample;}
   U64 candidate=A[sample]&PrefixDP::low(l);long long newcost=0;bool valid=true;
   for(int e=0;e<M;e++){int v=scenarios[e].target,base=bypass[e];for(int other=0;other<Q;other++)if(other!=q)base=min(base,through[e*Q+other]);U64 d=(A[v]^candidate)&PrefixDP::low(l);int st=d?64-__builtin_clzll(d):0,value=min(base,route[e*Q+q][st]);newcost+=value;valid&=value<INF;}
   if(valid&&newcost<oldcost){papers[z].color=candidate;update(z);accepted++;total_gain+=oldcost-newcost;for(int e=0;e<M;e++)through[e*Q+q]=route[e*Q+q][start[scenarios[e].target][z]];}
  }
 }
 cerr<<"exact_refit accepted="<<accepted<<" gain="<<double(total_gain)/M<<" CPU="<<double(clock()-began)/CLOCKS_PER_SEC<<"\n";
}

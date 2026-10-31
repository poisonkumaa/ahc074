#pragma once
// Robust coordinate optimization: each public target has several independently
// generated paper-access prices. These prices do not use judge orders.
inline void noisy_refit(const std::vector<uint64_t>&A,std::vector<PrefixPaper>&papers,int passes,int scenarios,int noise,int importance=16){
 const int P=A.size(),K=papers.size(),N=64,M=P*scenarios,INF=100000;
 std::mt19937 random(29813651);std::vector<std::array<int,192>>cost(M),start(P);
 std::vector<std::vector<int>>level(65);for(int k=0;k<K;k++)level[papers[k].len].push_back(k);
 for(int e=0;e<M;e++)for(int k=0;k<K;k++)cost[e][k]=4+(e<P?0:(random()%3==0?noise:0));
 auto update=[&](int k){for(int v=0;v<P;v++){uint64_t d=(A[v]^papers[k].color)&PrefixDP::low(papers[k].len);start[v][k]=d?64-__builtin_clzll(d):0;}};for(int k=0;k<K;k++)update(k);
 std::vector<std::array<int,65>>F(M),B(M),prefix(M);
 auto calculate=[&](){long long objective=0;for(int e=0;e<M;e++){int v=e%P;auto&f=F[e];auto&b=B[e];f.fill(INF);b.fill(INF);f[0]=0;b[N]=0;
  for(int l=1;l<=N;l++)for(int k:level[l])for(int j=start[v][k];j<l;j++)f[l]=std::min(f[l],f[j]+cost[e][k]);
  for(int l=N;l>0;l--)for(int k:level[l])for(int j=start[v][k];j<l;j++)b[j]=std::min(b[j],b[l]+cost[e][k]);
  if(f[N]>=INF)throw std::runtime_error("noisy_refit unreachable");objective+=f[N];
 }return double(objective)/M;};
 double initial=calculate();int changes=0,attempts=0;
 struct Node{int child[2]{-1,-1},sample=0;long long weight=0;};
 for(int pass=0;pass<passes;pass++)for(int l=N;l>0;l--){
  if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>1.15)break;
  int Q=level[l].size();if(!Q)continue;calculate();std::vector<Node>trie(1);std::vector<std::array<int,65>>path(P);
  std::vector<int>bypass(M,INF);std::vector<std::array<int,9>>through(M);
  for(int e=0;e<M;e++){int v=e%P;prefix[e][l]=INF;for(int j=l-1;j>=0;j--)prefix[e][j]=std::min(prefix[e][j+1],F[e][j]);
   for(int u=l+1;u<=N;u++)for(int k:level[u])if(start[v][k]<l)bypass[e]=std::min(bypass[e],prefix[e][start[v][k]]+cost[e][k]+B[e][u]);
   for(int q=0;q<Q;q++){int k=level[l][q];through[e][q]=std::min(INF,prefix[e][start[v][k]]+cost[e][k]+B[e][l]);}
  }
  for(int v=0;v<P;v++){int x=0;for(int d=1;d<=l;d++){int b=(A[v]>>(l-d))&1;if(trie[x].child[b]<0){int ni=trie.size();trie[x].child[b]=ni;trie.emplace_back();}x=trie[x].child[b];path[v][d]=x;trie[x].sample=v;}}
  for(int q=0;q<Q;q++){
   int k=level[l][q];for(auto&node:trie)node.weight=0;
   for(int e=0;e<M;e++){int v=e%P,base=bypass[e];for(int other=0;other<Q;other++)if(other!=q)base=std::min(base,through[e][other]);
    long long previous=0;
    for(int d=1;d<=l;d++){
     int route=std::min(INF,prefix[e][l-d]+cost[e][k]+B[e][l]);
     long long gain=route>=INF?0:(base>=INF?1000000LL+importance*(1000-route):importance*std::max(0,base-route));
     trie[path[v][d]].weight+=gain-previous+1;previous=gain;
    }
   }
   long long best=LLONG_MIN;int sample=-1;
   for(int x=0;x<int(trie.size());x++){auto&node=trie[x];bool leaf=true;for(int b=0;b<2;b++)if(node.child[b]>=0){trie[node.child[b]].weight+=node.weight;leaf=false;}if(leaf&&node.weight>best)best=node.weight,sample=node.sample;}
   uint64_t candidate=A[sample]&PrefixDP::low(l);bool valid=true;
   for(int e=0;e<M&&valid;e++){int v=e%P;uint64_t diff=(A[v]^candidate)&PrefixDP::low(l);int st=diff?64-__builtin_clzll(diff):0;
    int route=std::min(bypass[e],prefix[e][st]+cost[e][k]+B[e][l]);for(int other=0;other<Q;other++)if(other!=q)route=std::min(route,through[e][other]);valid=route<INF;
   }
   attempts++;if(!valid)continue;
   if(candidate!=papers[k].color){papers[k].color=candidate;update(k);changes++;for(int e=0;e<M;e++)through[e][q]=std::min(INF,prefix[e][start[e%P][k]]+cost[e][k]+B[e][l]);}
  }
 }
 std::cerr<<"noisy "<<initial<<" -> "<<calculate()<<" changed="<<changes<<" / "<<attempts<<"\n";
}

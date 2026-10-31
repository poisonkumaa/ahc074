#pragma once
// Maximize log sum exp(-beta * number_of_records) over every valid display.
// Every path increases its covered prefix, so a paper can occur at most once.
inline void soft_refit(const std::vector<uint64_t>& A,std::vector<PrefixPaper>& papers,int passes,double beta,int minlevel=1){
 const int P=A.size(),K=papers.size(),N=64;const double step=std::exp(-beta);
 std::vector<std::vector<int>>level(65),start(P,std::vector<int>(K));
 for(int k=0;k<K;k++)level[papers[k].len].push_back(k);
 auto update=[&](int k){for(int v=0;v<P;v++){uint64_t diff=(papers[k].color^A[v])&PrefixDP::low(papers[k].len);start[v][k]=diff?64-__builtin_clzll(diff):0;}};
 for(int k=0;k<K;k++)update(k);
 std::vector<std::array<double,65>>F(P),B(P),prefix(P);
 auto calculate=[&](){double objective=0;for(int v=0;v<P;v++){auto&f=F[v];auto&b=B[v];f.fill(0);b.fill(0);f[0]=1;b[N]=1;
  for(int l=1;l<=N;l++)for(int k:level[l])for(int j=start[v][k];j<l;j++)f[l]+=step*f[j];
  for(int l=N;l>0;l--)for(int k:level[l])for(int j=start[v][k];j<l;j++)b[j]+=step*b[l];
  if(!(f[N]>0))throw std::runtime_error("soft_refit unreachable barcode");objective+=std::log(f[N]);
 }return objective;};
 double initial=calculate();int changed=0,attempts=0;
 struct Node{int child[2]{-1,-1},sample=0;double weight=0;};
 for(int pass=0;pass<passes;pass++)for(int l=N;l>=minlevel;l--){
  if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>1.25)break;
  int Q=level[l].size();if(!Q)continue;calculate();std::vector<Node>trie(1);std::vector<std::array<int,65>>path(P);
  std::vector<double>bypass(P);std::vector<std::vector<double>>through(P,std::vector<double>(Q));
  for(int v=0;v<P;v++){
   prefix[v][l]=0;for(int j=l-1;j>=0;j--)prefix[v][j]=prefix[v][j+1]+F[v][j];
   for(int u=l+1;u<=N;u++)for(int k:level[u])if(start[v][k]<l)bypass[v]+=step*prefix[v][start[v][k]]*B[v][u];
   for(int q=0;q<Q;q++)through[v][q]=step*prefix[v][start[v][level[l][q]]]*B[v][l];
   int node=0;for(int d=1;d<=l;d++){int bit=(A[v]>>(l-d))&1;
    if(trie[node].child[bit]<0){int ni=trie.size();trie[node].child[bit]=ni;trie.emplace_back();}
    node=trie[node].child[bit];path[v][d]=node;trie[node].sample=v;
   }
  }
  for(int q=0;q<Q;q++){
   for(auto&node:trie)node.weight=0;
   for(int v=0;v<P;v++){
    double base=bypass[v];for(int other=0;other<Q;other++)if(other!=q)base+=through[v][other];
    double previous=0;
    for(int d=1;d<=l;d++){
     double value=step*prefix[v][l-d]*B[v][l],score=0;
     if(value>0)score=base>0?std::log1p(value/base):1000000+std::log(value);
     trie[path[v][d]].weight+=score-previous;previous=score;
    }
   }
   double best=-1e100;int sample=-1;
   for(int x=0;x<int(trie.size());x++){auto&node=trie[x];bool leaf=true;for(int b=0;b<2;b++)if(node.child[b]>=0){trie[node.child[b]].weight+=node.weight;leaf=false;}
    if(leaf&&node.weight>best){best=node.weight;sample=node.sample;}
   }
   int k=level[l][q];uint64_t candidate=A[sample]&PrefixDP::low(l);bool valid=true;
   for(int v=0;v<P&&valid;v++){uint64_t diff=(A[v]^candidate)&PrefixDP::low(l);int st=diff?64-__builtin_clzll(diff):0;
    double value=bypass[v]+step*prefix[v][st]*B[v][l];for(int other=0;other<Q;other++)if(other!=q)value+=through[v][other];valid=value>0;
   }
   attempts++;if(!valid)continue;
   if(candidate!=papers[k].color){papers[k].color=candidate;update(k);changed++;for(int v=0;v<P;v++)through[v][q]=step*prefix[v][start[v][k]]*B[v][l];}
  }
 }
 std::cerr<<"soft "<<initial<<" -> "<<calculate()<<" changed="<<changed<<" / "<<attempts<<" beta="<<beta<<"\n";
}

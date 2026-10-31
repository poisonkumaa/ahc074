#pragma once
inline void train_greedy_allocation(const vector<U64>&A,vector<PrefixPaper>&papers,int maxextra,int importance,double cpu_limit,int lengthpenalty,int minlen,int alpha){
 const int P=A.size(),N=64,INF=100000;
 papers.clear();for(int l=1;l<=N;l++)papers.push_back({l,PrefixDP::low(l-1)});papers.push_back({N,PrefixDP::low(N)});
 vector<vector<int>>level(65);for(int k=0;k<int(papers.size());k++)level[papers[k].len].push_back(k);
 vector<array<int,192>>start(P);vector<array<int,65>>F(P),B(P),bypass(P);vector<int>cost(65,4);
 auto update=[&](int k){for(int v=0;v<P;v++){U64 d=(A[v]^papers[k].color)&PrefixDP::low(papers[k].len);start[v][k]=d?64-__builtin_clzll(d):0;}};for(int k=0;k<int(papers.size());k++)update(k);
 auto distances=[&](){for(int l=1;l<=N;l++)cost[l]=4+alpha*(int(level[l].size())-1);int total=0;for(int v=0;v<P;v++){auto&f=F[v];auto&b=B[v];f.fill(INF);b.fill(INF);f[0]=0;b[N]=0;for(int l=1;l<=N;l++)for(int k:level[l])for(int j=start[v][k];j<l;j++)f[l]=min(f[l],f[j]+cost[l]);for(int l=N;l>0;l--)for(int k:level[l])for(int j=start[v][k];j<l;j++)b[j]=min(b[j],b[l]+cost[l]);total+=f[N];bypass[v].fill(INF);for(int k=0;k<int(papers.size());k++){int l=papers[k].len,pre=INF;for(int j=start[v][k];j<l;j++){pre=min(pre,f[j]);if(j+1<l)bypass[v][j+1]=min(bypass[v][j+1],pre+cost[l]+b[l]);}}}return total;};
 struct Node{int child[2]{-1,-1},weight=0,best=0,sample=0,chosen=0;};
 vector<vector<Node>>trie(65);vector<vector<array<int,64>>>path(65);vector<int>extras(65);
 for(int l=1;l<=N;l++){trie[l].emplace_back();path[l].resize(P);for(int v=0;v<P;v++){int x=0;for(int d=1;d<=l;d++){int b=(A[v]>>(l-d))&1;if(trie[l][x].child[b]<0){int z=trie[l].size();trie[l][x].child[b]=z;trie[l].emplace_back();}x=trie[l][x].child[b];trie[l][x].sample=v;path[l][v][d-1]=x;}}}
 int initial=distances(),last=initial;
 while(int(papers.size())<192){
  if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>cpu_limit)break;
  distances();int bestscore=INT_MIN,bestl=-1,bestv=-1;
  for(int l=minlen;l<=N;l++){
   if(extras[l]>=maxextra||level[l].size()>=8)continue;auto&nodes=trie[l];for(auto&node:nodes)node.weight=0;
   int penalty=0;for(int v=0;v<P;v++){
    int adjusted=min(bypass[v][l],F[v][l]+B[v][l]+alpha);penalty+=F[v][N]-adjusted;int pre=INF,prev=0;
    for(int d=1;d<=l;d++){pre=min(pre,F[v][l-d]);int benefit=max(0,adjusted-(pre+cost[l]+alpha+B[v][l]));nodes[path[l][v][d-1]].weight+=importance*(benefit-prev)+1;prev=benefit;}
   }
   for(int x=int(nodes.size())-1;x>=0;x--){auto&node=nodes[x];int a=node.child[0],b=node.child[1];if(a<0&&b<0){node.best=node.weight;node.chosen=node.sample;}else{int z=a;if(z<0||(b>=0&&nodes[b].best>nodes[z].best))z=b;node.best=node.weight+nodes[z].best;node.chosen=nodes[z].chosen;}}
   int score=nodes[0].best+importance*penalty-lengthpenalty*extras[l];
   if(score>bestscore){bestscore=score;bestl=l;bestv=nodes[0].chosen;}
  }
  if(bestl<0)break;int k=papers.size();papers.push_back({bestl,A[bestv]&PrefixDP::low(bestl)});level[bestl].push_back(k);extras[bestl]++;update(k);
 }
 last=distances();stable_sort(papers.begin(),papers.end(),[](auto a,auto b){return a.len<b.len;});cerr<<"Greedy allocation "<<double(initial)/P<<" -> "<<double(last)/P<<" papers="<<papers.size()<<" CPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";for(int l=1;l<=N;l++)if(extras[l])cerr<<l<<":"<<extras[l]<<" ";cerr<<"\n";
}

#pragma once
#include <bits/stdc++.h>
inline void exchange_prefix_dictionary(const std::vector<uint64_t>& A,std::vector<PrefixPaper>&papers,int iterations,int importance,int tries=3){
 const int P=A.size(),N=64,INF=1000;using Bits=std::array<uint64_t,3>;
 std::vector<std::vector<int>>level(N+1),start;std::vector<std::array<int,65>>F(P),B(P),common(P);std::vector<std::array<Bits,65>>mandatory(P);
 auto rebuild=[&](){
  for(auto &v:level)v.clear();int K=papers.size();start.assign(P,std::vector<int>(K));for(int k=0;k<K;k++)level[papers[k].len].push_back(k);
  for(int v=0;v<P;v++){
   for(int k=0;k<K;k++){uint64_t d=(papers[k].color^A[v])&PrefixDP::low(papers[k].len);start[v][k]=d?64-__builtin_clzll(d):0;}
   auto &f=F[v],&b=B[v],&c=common[v];f.fill(INF);b.fill(INF);c.fill(0);f[0]=0;b[N]=0;for(auto &x:mandatory[v])x={0,0,0};
   for(int l=1;l<=N;l++)for(int k:level[l]){
    c[l]=std::max(c[l],l-start[v][k]);int best=INF;Bits m{};
    for(int j=start[v][k];j<l;j++)if(f[j]<best){best=f[j];m=mandatory[v][j];}else if(f[j]==best)for(int z=0;z<3;z++)m[z]&=mandatory[v][j][z];
    if(best>=INF)continue;m[k>>6]|=1ULL<<(k&63);
    if(best+1<f[l])f[l]=best+1,mandatory[v][l]=m;else if(best+1==f[l])for(int z=0;z<3;z++)mandatory[v][l][z]&=m[z];
   }
   for(int l=N;l>0;l--)for(int k:level[l])for(int j=start[v][k];j<l;j++)b[j]=std::min(b[j],b[l]+1);
  }
 };
 auto removal_loss=[&](int k){int loss=0,l=papers[k].len;for(int v=0;v<P;v++){
   int other=0;for(int z:level[l])if(z!=k)other=std::max(other,l-start[v][z]);loss+=common[v][l]-other;
   if(!(mandatory[v][N][k>>6]>>(k&63)&1))continue;auto f=F[v];
   for(int upper=l;upper<=N;upper++){f[upper]=INF;for(int z:level[upper])if(z!=k)for(int j=start[v][z];j<upper;j++)f[upper]=std::min(f[upper],f[j]+1);}
   if(f[N]>=INF)return 1000000000;loss+=importance*(f[N]-F[v][N]);
  }return loss;};
 auto addition=[&](){
  struct Node{int child[2]{-1,-1},weight=0,sample=0;};int best=-1;PrefixPaper result{};
  for(int l=1;l<=N;l++){
   std::vector<Node>trie(1);
   for(int v=0;v<P;v++){int x=0,bestprefix=INF,previous=0;for(int d=1;d<=l;d++){
    int bit=A[v]>>(l-d)&1;if(trie[x].child[bit]<0){int ni=trie.size();trie[x].child[bit]=ni;trie.emplace_back();}x=trie[x].child[bit];trie[x].sample=v;
    bestprefix=std::min(bestprefix,F[v][l-d]);int gain=importance*std::max(0,F[v][N]-(bestprefix+1+B[v][l]))+std::max(0,d-common[v][l]);
    trie[x].weight+=gain-previous;previous=gain;
   }}
   for(int x=0;x<int(trie.size());x++){
    auto &p=trie[x];if(p.child[0]<0&&p.child[1]<0&&p.weight>best){best=p.weight;result={l,A[p.sample]&PrefixDP::low(l)};}
    for(int b=0;b<2;b++)if(p.child[b]>=0)trie[p.child[b]].weight+=p.weight;
   }
  }return std::pair{best,result};
 };
 rebuild();double initial=0;for(auto f:F)initial+=double(f[N])/P;int accepted=0;
 for(int it=0;it<iterations;it++){
  std::vector<std::pair<int,int>>remove;if(papers.size()<192)remove.push_back({0,-1});else{for(int k=0;k<int(papers.size());k++)remove.push_back({removal_loss(k),k});std::sort(remove.begin(),remove.end());}
  auto original=papers;int bestgain=0;std::vector<PrefixPaper>bestpapers;
  for(int t=0;t<std::min(tries,int(remove.size()));t++){
   auto[loss,k]=remove[t];if(loss>=1000000000)continue;papers=original;if(k>=0)papers.erase(papers.begin()+k);rebuild();auto[gain,added]=addition();
   if(gain-loss>bestgain){bestgain=gain-loss;bestpapers=papers;bestpapers.push_back(added);}
  }
  if(bestpapers.empty()){papers=original;rebuild();break;}papers=bestpapers;rebuild();accepted++;
 }
 double final=0;for(auto f:F)final+=double(f[N])/P;std::cerr<<"exchange "<<initial<<" -> "<<final<<" steps="<<accepted<<"\n";
 std::stable_sort(papers.begin(),papers.end(),[](auto a,auto b){return a.len<b.len;});
}

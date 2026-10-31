#pragma once
inline void train_backbone_dictionary(const std::vector<uint64_t>& A,std::vector<PrefixPaper>&papers,int n,int perlevel,int passes,int importance){
 const int P=A.size(),K=192,INF=1000;
 papers.clear();for(int l=1;l<=n;l++)papers.push_back({l,PrefixDP::low(l-1)});papers.push_back({n,PrefixDP::low(n)});int fixed=papers.size();papers.resize(K,{0,0});
 std::vector<std::vector<int>>level(n+1),extra(n+1);std::vector<int>spare;
 for(int k=0;k<K;k++)if(k<fixed)level[papers[k].len].push_back(k);else spare.push_back(k);
 std::vector<std::array<int,192>> start(P);std::vector<std::array<int,65>>fw(P),bw(P);
 auto update=[&](int k){int l=papers[k].len;for(int v=0;v<P;v++){uint64_t d=(A[v]^papers[k].color)&PrefixDP::low(l);start[v][k]=d?64-__builtin_clzll(d):0;}};
 for(int k=0;k<K;k++)update(k);
 auto distances=[&](){
  int total=0;for(int v=0;v<P;v++){
   auto&f=fw[v];auto&b=bw[v];f.fill(INF);b.fill(INF);f[0]=0;b[n]=0;
   for(int l=1;l<=n;l++)for(int k:level[l])for(int j=start[v][k];j<l;j++)f[l]=std::min(f[l],f[j]+1);
   for(int l=n;l>0;l--)for(int k:level[l])for(int j=start[v][k];j<l;j++)b[j]=std::min(b[j],b[l]+1);
   total+=f[n];
  }return double(total)/P;
 };
 double initial=distances();
 struct Node{int child[2]{-1,-1},weight=0,sample=0;std::array<int,33>dp,split;Node(){dp.fill(-100000000);dp[0]=0;split.fill(0);}};
 for(int pass=0;pass<passes;pass++)for(int l=n;l>0;l--){
  int amount=std::min(perlevel,int(extra[l].size()+spare.size()));if(!amount)continue;distances();std::vector<Node>trie(1);
  for(int v=0;v<P;v++){
   auto excluded=fw[v];excluded[l]=INF;
   for(int k:level[l])if(k<fixed)for(int j=start[v][k];j<l;j++)excluded[l]=std::min(excluded[l],excluded[j]+1);
   for(int u=l+1;u<=n;u++){excluded[u]=INF;for(int k:level[u])for(int j=start[v][k];j<u;j++)excluded[u]=std::min(excluded[u],excluded[j]+1);}
   int bypass=excluded[n],node=0,bestprefix=INF,prev=0;
   for(int d=1;d<=l;d++){
    int b=A[v]>>(l-d)&1;if(trie[node].child[b]<0){int next=trie.size();trie[node].child[b]=next;trie.emplace_back();}node=trie[node].child[b];trie[node].sample=v;
    bestprefix=std::min(bestprefix,fw[v][l-d]);int benefit=std::max(0,bypass-(bestprefix+1+bw[v][l]));trie[node].weight+=importance*(benefit-prev)+1;prev=benefit;
   }
  }
  for(int node=int(trie.size())-1;node>=0;node--){auto&v=trie[node];int a=v.child[0],b=v.child[1];if(a<0&&b<0){v.dp[1]=v.weight;continue;}
   for(int q=1;q<=amount;q++)for(int left=0;left<=q;left++){int right=q-left;if((a<0&&left)||(b<0&&right))continue;int value=(a<0?0:trie[a].dp[left])+(b<0?0:trie[b].dp[right])+v.weight;if(value>v.dp[q])v.dp[q]=value,v.split[q]=left;}
  }
  int use=amount;while(use>1&&trie[0].dp[use]<0)use--;std::vector<uint64_t>pattern;
  std::function<void(int,int)>collect=[&](int node,int count){if(!count)return;auto&v=trie[node];if(v.child[0]<0&&v.child[1]<0){pattern.push_back(A[v.sample]&PrefixDP::low(l));return;}int left=v.split[count];if(left)collect(v.child[0],left);if(count-left)collect(v.child[1],count-left);};collect(0,use);
  while(extra[l].size()<pattern.size()){int k=spare.back();spare.pop_back();extra[l].push_back(k);level[l].push_back(k);papers[k].len=l;}
  for(int q=0;q<int(pattern.size());q++){int k=extra[l][q];papers[k].color=pattern[q];update(k);}
 }
 std::cerr<<"backbone "<<initial<<" -> "<<distances()<<" papers="<<K-spare.size()<<" CPU="<<double(clock())/CLOCKS_PER_SEC<<"\n";
 std::vector<PrefixPaper>p;for(auto x:papers)if(x.len)p.push_back(x);std::stable_sort(p.begin(),p.end(),[](auto a,auto b){return a.len<b.len;});papers=p;
}

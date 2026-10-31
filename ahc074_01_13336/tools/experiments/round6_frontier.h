#pragma once
static int round6_frontier_amount=8,round6_frontier_refit=4,round6_frontier_repetitions=0;
inline void train_frontier_dictionary(const vector<U64>&A,vector<PrefixPaper>&papers,int maxq){
 const int P=A.size(),N=64,K=192,NEG=-100000000;
 vector<int>frontier(P,N);papers.clear();
 auto fallback=[&](const vector<int>&f,bool append){
  bool need[65][2]{};int count=0;
  for(int v=0;v<P;v++)for(int j=f[v];j>0;){int bit=(A[v]>>(j-1))&1;need[j][bit]=true;U64 color=PrefixDP::low(bit?j:j-1),diff=(A[v]^color)&PrefixDP::low(j);j=diff?64-__builtin_clzll(diff):0;}
  for(int l=1;l<=N;l++)for(int b=0;b<2;b++)if(need[l][b]){count++;if(append)papers.push_back({l,PrefixDP::low(b?l:l-1)});}
  return count;
 };
 int fallback_at=-1;
 for(int l=N;l>0;l--){
  vector<int>group;for(int v=0;v<P;v++)if(frontier[v]==l)group.push_back(v);if(group.empty())continue;
  struct Node{int child[2]{-1,-1},weight=0,sample=0,leaves=0;vector<int>dp,split;};vector<Node>trie(1);
  for(int v:group){int x=0;for(int d=0;d<l;d++){int bit=(A[v]>>(l-1-d))&1;if(trie[x].child[bit]<0){int y=trie.size();trie[x].child[bit]=y;trie.emplace_back();}x=trie[x].child[bit];trie[x].weight++;trie[x].sample=v;}}
  int limit=min({maxq,K-int(papers.size()),int(group.size())});
  for(int x=int(trie.size())-1;x>=0;x--){auto&node=trie[x];node.dp.assign(limit+1,NEG);node.split.assign(limit+1,0);node.dp[0]=0;int a=node.child[0],b=node.child[1];
   if(a<0&&b<0){node.leaves=1;if(limit)node.dp[1]=node.weight;continue;}
   int ca=a<0?0:trie[a].leaves,cb=b<0?0:trie[b].leaves;node.leaves=min(limit,ca+cb);
   for(int q=1;q<=node.leaves;q++)for(int left=max(x==0&&a>=0?1:0,q-cb);left<=min(q-(x==0&&b>=0?1:0),ca);left++){int right=q-left,value=(a<0?0:trie[a].dp[left])+(b<0?0:trie[b].dp[right])+node.weight;if(value>node.dp[q])node.dp[q]=value,node.split[q]=left;}
  }
  vector<PrefixPaper>chosen;vector<int>next;
  for(int q=limit;q>0;q--){if(trie[0].dp[q]<0)continue;vector<PrefixPaper>candidate;
   function<void(int,int)>collect=[&](int x,int count){if(!count)return;auto&node=trie[x];if(node.child[0]<0&&node.child[1]<0){candidate.push_back({l,A[node.sample]&PrefixDP::low(l)});return;}int left=node.split[count];if(left)collect(node.child[0],left);if(count-left)collect(node.child[1],count-left);};collect(0,q);
   auto f=frontier;for(int v:group){int best=l;for(auto p:candidate){U64 diff=(A[v]^p.color)&PrefixDP::low(l);best=min(best,diff?64-__builtin_clzll(diff):0);}assert(best<l);f[v]=best;}
   if(papers.size()+candidate.size()+fallback(f,false)<=K){chosen=move(candidate);next=move(f);break;}
  }
  if(chosen.empty()){fallback_at=l;fallback(frontier,true);fill(frontier.begin(),frontier.end(),0);break;}
  for(auto p:chosen)papers.push_back(p);frontier=move(next);
 }
 stable_sort(papers.begin(),papers.end(),[](auto a,auto b){return a.len<b.len;});
 assert(papers.size()<=192);for(int f:frontier)assert(f==0);
 cerr<<"Frontier papers="<<papers.size()<<" fallback="<<fallback_at<<" counts=";for(int l=1;l<=64;l++){int q=0;for(auto p:papers)q+=p.len==l;if(q)cerr<<l<<":"<<q<<",";}cerr<<"\n";
}

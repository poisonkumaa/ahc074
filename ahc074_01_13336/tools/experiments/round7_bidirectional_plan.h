#pragma once
#include "round6_general_future.h"
#include "round7_bidirectional_reorder.h"
struct BidirectionalPlanner {
 struct Edge{uint16_t next;uint8_t paper;};
 int K;vector<BidirectionalPaper>papers;vector<U64>masks,A,future;vector<int>order,quality;
 vector<vector<vector<Edge>>>candidate;
 BidirectionalPlanner(vector<U64>data,vector<BidirectionalPaper>p):K(p.size()),papers(move(p)),A(move(data)),future(K),order(K){
  for(auto c:papers){masks.push_back(c.right?~PrefixDP::low(64-c.len):PrefixDP::low(c.len));quality.push_back(c.len);}iota(order.begin(),order.end(),0);
  candidate.resize(A.size(),vector<vector<Edge>>(65*65));
  for(int v=0;v<int(A.size());v++)for(int l=0;l<64;l++)for(int r=l+1;r<=64;r++){
   U64 gap=PrefixDP::low(r)&~PrefixDP::low(l);auto&out=candidate[v][l*65+r];
   for(int k=0;k<K;k++){U64 own=masks[k]&gap;if(!own||((A[v]^papers[k].color)&own))continue;int nl=l,nr=r;if(papers[k].right)nr=max(l,64-papers[k].len);else nl=min(r,papers[k].len);out.push_back({uint16_t(nl*65+nr),uint8_t(k)});}
  }
 }
 U64 display()const{U64 covered=0,color=0;for(int k:order){color|=papers[k].color&masks[k]&~covered;covered|=masks[k];}return color;}
 vector<char>visible()const{vector<char>vis(K);U64 covered=0;for(int k:order){vis[k]=bool(masks[k]&~covered);covered|=masks[k];}return vis;}
 vector<pair<int,int>>solve(int target,int round,double access,bool merge_goal,int dedicated=0){
  int count[64]{},pos[192],depth[192],above[192][64];for(int i=0;i<K;i++){int k=order[i];pos[k]=i;depth[k]=K;for(int j=0;j<64;j++){above[k][j]=count[j];if(masks[k]>>j&1){depth[k]=min(depth[k],count[j]);count[j]++;}}}
  int cost[65*65],chosen[65*65],next[65*65];fill(cost,cost+65*65,1000000000);for(int j=0;j<=64;j++)cost[j*65+j]=0;
  for(int width=1;width<=64;width++)for(int l=0;l+width<=64;l++){
   int r=l+width,s=l*65+r;
   for(auto e:candidate[target][s]){int k=e.paper,value=cost[e.next]+1024+int(access*1024*depth[k])-2*__builtin_popcountll(future[k]);if(value<cost[s])cost[s]=value,chosen[s]=k,next[s]=e.next;}
  }
  vector<int>goal;for(int s=64;s/65!=s%65;s=next[s]){assert(cost[s]<1000000000);goal.push_back(chosen[s]);}
  if(merge_goal&&goal.size()>1){int last=goal.back();goal.pop_back();vector<int>left,right;for(int k:goal)(papers[k].right?right:left).push_back(k);goal.clear();int x=0,y=0;while(x<int(left.size())||y<int(right.size())){if(y==int(right.size())||(x<int(left.size())&&pos[left[x]]<pos[right[y]]))goal.push_back(left[x++]);else goal.push_back(right[y++]);}goal.push_back(last);}
  if(dedicated&&goal.size()<=20){auto ops=dedicated==1?bidirectional_reorder(order,masks,papers,goal,A[target]):bidirectional_retrieve(order,masks,papers,goal,A[target],dedicated==3);vector<char>selected(K);for(int k:goal)selected[k]=true;U64 bit=1ULL<<(round%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);return ops;}
  vector<U64>regions,witness;U64 covered=0;vector<char>selected(K);
  for(int k:goal){U64 own=masks[k]&~covered;assert(own);regions.push_back(own);int best=-1;for(int j=0;j<64;j++)if(own>>j&1)if(best<0||above[k][j]<above[k][best])best=j;witness.push_back(1ULL<<best);covered|=masks[k];selected[k]=true;}
  assert(covered==~0ULL);vector<int>length(K);
  for(int k=0;k<K;k++)for(int i=int(goal.size())-1;i>=0;i--)if(masks[k]&regions[i]){length[k]=2*i+2+bool(masks[k]&witness[i]);break;}
  for(int i=0;i<int(goal.size());i++)assert(length[goal[i]]==2*i+3);
  vector<pair<int,int>>ops;
  while(GeneralPrefixFuture::fixed_distance(order,length,selected)){
   auto vis=visible();auto[from,to]=GeneralPrefixFuture::best_move(order,length,selected,vis,quality,.5,&future);assert(from>=0&&vis[order[from]]);int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);ops.push_back({k,to});
  }
  assert(ops.size()<=384&&display()==A[target]);U64 bit=1ULL<<(round%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);return ops;
 }
};

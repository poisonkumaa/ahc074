#pragma once
#include <algorithm>
#include <array>
#include <utility>
#include <vector>
// The target is the permutation returned by direct_final for the same goal.
inline bool direct_moves(std::vector<int>&order,const std::vector<int>&length,const std::vector<char>&selected,const std::vector<int>&target,std::vector<std::pair<int,int>>&operations){
 const int K=order.size(),N=*std::max_element(length.begin(),length.end());int owner[65],pos[192],rank[192],bad[192]{};bool moving[192]{};std::fill(owner,owner+65,-1);
 for(int k=0;k<K;k++)if(selected[k])owner[length[k]]=k;int root=-1;for(int l=N;l>=0;l--){if(owner[l]>=0)root=owner[l];owner[l]=root;}
 for(int i=0;i<K;i++){pos[order[i]]=i;rank[target[i]]=i;}
 int suffix=N+1,remaining=0;for(int i=K-1;i>=0;i--){int k=order[i];moving[k]=length[k]>=suffix;remaining+=moving[k];if(selected[k])suffix=std::min(suffix,length[k]);}
 for(int k=0;k<K;k++)if(!selected[k]&&!moving[k]&&pos[k]<pos[owner[length[k]]])bad[owner[length[k]]]++;
 for(int k=0;k<K;k++)if(selected[k]&&!moving[k]&&bad[k]){moving[k]=true;remaining++;}
 int previous=-1;for(int k:order)if(!moving[k]){if(rank[k]<=previous)return false;previous=rank[k];}
 auto result=order;std::vector<std::pair<int,int>>ops;ops.reserve(remaining);
 while(remaining--){
  int from=-1,highest=0;for(int i=0;i<K;i++){int k=result[i];if(length[k]>highest){highest=length[k];if(moving[k]){from=i;break;}}}
  if(from<0)return false;int k=result[from],next=-1;for(int r=rank[k]+1;r<K;r++)if(!moving[target[r]]){next=target[r];break;}
  int to=K-1;if(next>=0){to=std::find(result.begin(),result.end(),next)-result.begin();if(from<to)to--;}
  if(to==from)return false;result.erase(result.begin()+from);result.insert(result.begin()+to,k);moving[k]=false;ops.push_back({k,to});
 }
 if(result!=target)return false;order=std::move(result);operations=std::move(ops);return true;
}

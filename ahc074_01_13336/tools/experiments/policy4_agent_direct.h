#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <climits>
#include <vector>
namespace GlobalPrefix {
inline std::vector<int> final_order(const std::vector<int>&order,const std::vector<int>&length,const std::vector<char>&selected,const std::vector<uint64_t>*future=nullptr){
 using namespace std;
 int K=order.size(),N=*max_element(length.begin(),length.end()),owner[65],root=-1;fill(owner,owner+65,-1);for(int k=0;k<K;k++)if(selected[k])owner[length[k]]=k;for(int l=N;l>=0;l--){if(owner[l]>=0)root=owner[l];owner[l]=root;}
 vector<char>mandatory(K);int suffix=N+1;for(int i=K-1;i>=0;i--){int k=order[i];mandatory[k]=length[k]>=suffix;if(selected[k])suffix=min(suffix,length[k]);}
 vector<vector<int>>children(K);vector<int>insert;int pos[192];for(int i=0;i<K;i++){int k=order[i];pos[k]=i;if(selected[k])continue;if(mandatory[k])insert.push_back(k);else children[owner[length[k]]].push_back(k);}
 stable_sort(insert.begin(),insert.end(),[&](int a,int b){return length[a]<length[b];});
 double power[193];power[0]=1;for(int i=1;i<=K;i++)power[i]=power[i-1]*.5;
 for(int k:insert){auto&v=children[owner[length[k]]];int M=v.size(),kl=length[k];uint64_t below[192]{},remain[193]{};int inversions[192]{},lower_after[193]{};
  for(int i=M-1;i>=0;i--){int x=v[i];for(int j=i+1;j<M;j++){int y=v[j];if(length[y]<length[x])inversions[x]++;if(future&&length[y]<=length[x])below[x]|=(*future)[y];}remain[i]=remain[i+1]|(future&&length[x]<=kl?(*future)[x]:0ULL);lower_after[i]=lower_after[i+1]+(length[x]<kl);}
  double gain=0,best=1e100;int larger=0,bestinv=INT_MAX,at=0;
  for(int i=0;i<=M;i++){double q=gain+(future?double(__builtin_popcountll(remain[i])):1-power[lower_after[i]]);int inv=larger+lower_after[i];if(q<best-1e-10||(abs(q-best)<=1e-10&&inv<bestinv)){best=q;bestinv=inv;at=i;}
   if(i<M){int x=v[i];if(length[x]>kl)larger++;if(future){if(length[x]>=kl)gain+=__builtin_popcountll((*future)[k]&~below[x]);}else if(length[x]>kl)gain+=power[inversions[x]]*.5;}
  }
  v.insert(v.begin()+at,k);
 }
 vector<int>out;out.reserve(K);for(int l=1;l<=N;l++){int k=owner[l];if(k>=0&&selected[k]&&length[k]==l){out.push_back(k);for(int x:children[k])out.push_back(x);}}return out;
}
}

#pragma once
// The last selected paper is retained so the secondary reward can use pairs.
struct PairGoalDP {
 int n,k;vector<PrefixPaper>a;vector<array<int,193>>cnt,dp,parent;
 vector<vector<int>>levels;
 PairGoalDP(int n_,const vector<PrefixPaper>&a_):n(n_),k(a_.size()),a(a_),cnt(n+2),dp(k+1),parent(k+1),levels(n+1){for(int z=0;z<k;z++)levels[a[z].len].push_back(z);levels[0].push_back(k);}
 vector<int> solve(const vector<int>&p,U64 target,int&best,const vector<U64>&future,int alpha){
  int pos[192],reward[192];for(int i=0;i<k;i++)pos[p[i]]=i;
  for(int z=0;z<k;z++)reward[z]=16+2*__builtin_popcountll(future[z]);
  for(int l=0;l<=n+1;l++){cnt[l][0]=0;for(int i=0;i<k;i++)cnt[l][i+1]=cnt[l][i]+(a[p[i]].len>=l);}
  for(auto&r:dp)r.fill(1000000000);dp[k][0]=0;
  for(int j=0;j<n;j++){
   int cand[192],num=0;for(int z=0;z<k;z++)if(a[z].len>j&&!((a[z].color^target)&(PrefixDP::low(a[z].len)^PrefixDP::low(j))))cand[num++]=z;
   if(!num)continue;
   for(int previous:levels[j]){
    int active[193],count=0;for(int h=0;h<=k;h++)if(dp[previous][h]<1000000000)active[count++]=h;
    if(!count)continue;
    for(int ci=0;ci<num;ci++){
     int z=cand[ci],l=a[z].len,q=pos[z],gain=reward[z];if(previous<k)gain+=alpha*__builtin_popcountll(future[previous]&future[z]);
     for(int ai=0;ai<count;ai++){
      int h=active[ai],add=0;if(q>=h){int moved=cnt[l][q]-cnt[l][h],upper=cnt[j+1][q]-cnt[j+1][h];add=moved+(upper>moved);}
      int nh=max(q+1,h),value=dp[previous][h]+add*65536-gain;
      if(value<dp[z][nh]){dp[z][nh]=value;parent[z][nh]=previous*(k+1)+h;}
     }
    }
   }
  }
  int last=-1,h=-1,value=1000000000;for(int z:levels[n])for(int q=0;q<=k;q++)if(dp[z][q]<value)value=dp[z][q],last=z,h=q;
  if(last<0)abort();best=(value+65535)/65536;vector<int>ans;
  while(last<k){ans.push_back(last);int code=parent[last][h];last=code/(k+1);h=code%(k+1);}
  reverse(ans.begin(),ans.end());return ans;
 }
};

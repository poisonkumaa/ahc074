#pragma once
#include <bits/stdc++.h>
using namespace std;using U64=uint64_t;
struct PrefixPaper {int len;U64 color;};
struct PrefixDP {
 int n,k;vector<PrefixPaper>a;
 vector<array<int,193>> cnt;
 PrefixDP(int n_,vector<PrefixPaper>a_):n(n_),k(a_.size()),a(move(a_)),cnt(n+2){}
 static U64 low(int n){return n==64?~0ULL:(1ULL<<n)-1;}
 vector<int> solve(const vector<int>&p,U64 target,int &best){
  int pos[192];for(int i=0;i<k;i++)pos[p[i]]=i;
  for(int l=0;l<=n+1;l++){cnt[l][0]=0;for(int i=0;i<k;i++)cnt[l][i+1]=cnt[l][i]+(a[p[i]].len>=l);}
  vector<array<int,193>>dp(n+1),parent(n+1),which(n+1);
  for(auto&r:dp)r.fill(1000000);dp[0][0]=0;
  // A secondary preference keeps more visible records after a shortest transition.
  for(int j=0;j<n;j++){
   vector<int>cand;for(int z=0;z<k;z++){int l=a[z].len;if(l>j&&!((a[z].color^target)&(low(l)^low(j))))cand.push_back(z);}
   for(int z:cand){int l=a[z].len,q=pos[z];
    for(int h=0;h<=k;h++)if(dp[j][h]<1000000){
     int add=0;if(q>=h){int moved=cnt[l][q]-cnt[l][h],upper=cnt[j+1][q]-cnt[j+1][h];add=moved+(upper>moved);}
     int nh=max(q+1,h),value=dp[j][h]+add*256-1;
     if(value<dp[l][nh]){dp[l][nh]=value;parent[l][nh]=j*(k+1)+h;which[l][nh]=z;}
    }
   }
  }
  int h=min_element(dp[n].begin(),dp[n].begin()+k+1)-dp[n].begin(),j=n;
  best=(dp[n][h]+255)/256;vector<int>ans;
  while(j){int z=which[j][h],prev=parent[j][h];ans.push_back(z);j=prev/(k+1);h=prev%(k+1);}
  reverse(ans.begin(),ans.end());return ans;
 }
};

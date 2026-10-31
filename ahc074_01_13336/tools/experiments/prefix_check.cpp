#include <bits/stdc++.h>
using namespace std;
struct Paper {int len,color;};
int dpdist(const vector<int>&p,const vector<Paper>&a,int n,int target){
 int k=p.size(); vector<int>pos(k);for(int i=0;i<k;i++)pos[p[i]]=i;
 vector<vector<int>> dp(n+1,vector<int>(k+1,999));dp[0][0]=0;
 for(int j=0;j<n;j++)for(int h=-1;h<k;h++)if(dp[j][h+1]<999)for(int z=0;z<k;z++){
  int l=a[z].len,q=pos[z];if(l<=j||((a[z].color^target)&(((1<<l)-1)^((1<<j)-1))))continue;
  int add=0;bool bad=0;
  if(q>h)for(int t=h+1;t<q;t++){int b=a[p[t]].len;if(b>=l)add++;else if(b>j)bad=1;}
  dp[l][max(q,h)+1]=min(dp[l][max(q,h)+1],dp[j][h+1]+add+bad);
 }
 return *min_element(dp[n].begin(),dp[n].end());
}
int main(int argc,char**argv){
 int n=argc>1?atoi(argv[1]):3,k=2*n;vector<Paper>a;mt19937 rng(1234);
 for(int j=1;j<=n;j++){int c=rng()&((1<<j)-1);a.push_back({j,c});a.push_back({j,c^(1<<(j-1))});}
 vector<vector<int>>ps;vector<int>p(k);iota(p.begin(),p.end(),0);do{ps.push_back(p);}while(next_permutation(p.begin(),p.end()));
 map<vector<int>,int>id;for(int s=0;s<(int)ps.size();s++)id[ps[s]]=s;
 vector<vector<int>>rev(ps.size());vector<int>display(ps.size());
 for(int s=0;s<(int)ps.size();s++){
  int high=0;for(int i=0;i<k;i++){auto x=a[ps[s][i]];if(x.len<=high)continue;display[s]|=x.color&(((1<<x.len)-1)^((1<<high)-1));high=x.len;
   for(int d=0;d<k;d++)if(d!=i){auto q=ps[s];int z=q[i];q.erase(q.begin()+i);q.insert(q.begin()+d,z);rev[id[q]].push_back(s);}
  }
 }
 int errors=0;
 for(int t=0;t<(1<<n);t++){
  vector<int>dist(ps.size(),999),q;for(int s=0;s<(int)ps.size();s++)if(display[s]==t)dist[s]=0,q.push_back(s);
  for(int h=0;h<(int)q.size();h++)for(int v:rev[q[h]])if(dist[v]>dist[q[h]]+1)dist[v]=dist[q[h]]+1,q.push_back(v);
  for(int s=0;s<(int)ps.size();s++){int v=dpdist(ps[s],a,n,t);if(v!=dist[s]){if(errors++<10){cerr<<"target="<<t<<" dp="<<v<<" exact="<<dist[s]<<" perm=";for(int x:ps[s])cerr<<x<<",";cerr<<"\n";}}}
 }
 cerr<<"states="<<ps.size()<<" errors="<<errors<<"\n";
}

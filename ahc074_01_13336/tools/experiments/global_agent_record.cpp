#include <bits/stdc++.h>
using namespace std; using U64=uint64_t;
int dist(const vector<int>&p,U64 z,int n){
 bool moved[65]{};int low=n+1,ans=0;
 for(int i=n;i>=0;i--){int x=p[i];if(x>low){moved[x]=1;ans++;}if(x==n||((z>>x)&1))low=min(low,x);}
 int pos[65];for(int i=0;i<=n;i++)pos[p[i]]=i;
 int prev=-1;
 for(int x=0;x<=n;x++)if(x==n||((z>>x)&1)){
  if(!moved[x]){for(int j=prev+1;j<x;j++)if(!moved[j]&&pos[j]<pos[x]){ans++;break;}}
  prev=x;
 }
 return ans;
}
U64 shown(const vector<int>&p,int n){int hi=-1;U64 z=0;for(int x:p)if(x>hi){hi=x;if(x<n)z|=1ULL<<x;}return z;}
double futurecost(const vector<int>&p,int n){
 U64 before[65]{},low[65];int pos[65];U64 seen=0;
 for(int j=0;j<=n;j++)low[j]=j==64?~0ULL:(1ULL<<j)-1;
 double r=0;
 for(int i=0;i<=n;i++){int j=p[i];pos[j]=i;before[j]=seen;if(j<64)seen|=1ULL<<j;int q=__builtin_popcountll(low[j]&~before[j]);r+=1-ldexp(1.,-q);}
 for(int z=1;z<=n;z++){
  int hi=-1;double probability=0;
  for(int i=pos[z]-1;i>=0;i--){int j=p[i];if(j>=z||j<hi)continue;
   probability+=ldexp(1.,__builtin_popcountll(before[j]&low[j])-z);
   if(hi>=0)probability-=ldexp(1.,__builtin_popcountll(before[j]&low[hi])-z);
   hi=j;
  }
  r+=probability*(z==n?1.:.5);
 }
 return r;
}
void moveit(vector<int>&p,int i,int d){int k=p[i];p.erase(p.begin()+i);p.insert(p.begin()+d,k);}
void check(int n){
 vector<vector<int>>ps; vector<int>p(n+1);iota(p.begin(),p.end(),0); do{ps.push_back(p);}while(next_permutation(p.begin(),p.end()));
 map<vector<int>,int>ids;for(int s=0;s<(int)ps.size();s++)ids[ps[s]]=s;
 vector<vector<int>>rev(ps.size());for(int s=0;s<(int)ps.size();s++){int hi=-1;for(int i=0;i<=n;i++){int x=ps[s][i];if(x<hi)continue;hi=x;for(int d=0;d<=n;d++)if(i!=d){auto q=ps[s];moveit(q,i,d);rev[ids[q]].push_back(s);}}}
 int errors=0;
 for(U64 z=0;z<(1ULL<<n);z++){vector<int>ds(ps.size(),999),q;for(int s=0;s<(int)ps.size();s++)if(shown(ps[s],n)==z)ds[s]=0,q.push_back(s);for(int h=0;h<(int)q.size();h++)for(int s:rev[q[h]])if(ds[s]>ds[q[h]]+1)ds[s]=ds[q[h]]+1,q.push_back(s);for(int s=0;s<(int)ps.size();s++)if(ds[s]!=dist(ps[s],z,n)){if(errors++<10){cerr<<"bad "<<z<<" true="<<ds[s]<<" calc="<<dist(ps[s],z,n)<<" perm=";for(int k:ps[s])cerr<<k<<",";cerr<<"\n";}}}
 int fe=0;for(auto&p:ps){double ex=0;for(U64 z=0;z<(1ULL<<n);z++)ex+=dist(p,z,n);ex/=1<<n;if(abs(ex-futurecost(p,n))>1e-8){if(fe++<5)cerr<<"future bad "<<ex<<" vs "<<futurecost(p,n)<<"\n";}}
 cerr<<"states="<<ps.size()<<" errors="<<errors<<" future_errors="<<fe<<"\n";
}
int main(int argc,char**argv){
 if(argc>1&&string(argv[1])=="check"){check(argc>2?atoi(argv[2]):5);return 0;}
 int n,K,P,R;cin>>n>>K>>P>>R;vector<U64>a(P);for(int v=0;v<P;v++){string s;cin>>s;for(int j=0;j<n;j++)if(s[j]=='0')a[v]|=1ULL<<j;}
 vector<int>p(n+1);iota(p.begin(),p.end(),0);int total=0,mx=0,rounds=argc>1?atoi(argv[1]):R;
 for(int r=0;r<rounds;r++){
  int v;cin>>v;U64 z=a[v];int start=dist(p,z,n),cur=start,moves=0;
  while(shown(p,n)!=z){int bi=-1,bd=-1;double best=1e9;int hi=-1;
   for(int i=0;i<=n;i++){int k=p[i];if(k<hi)continue;hi=k;
    for(int d=0;d<=n;d++)if(d!=i){auto q=p;moveit(q,i,d);if(dist(q,z,n)!=cur-1)continue;double f=futurecost(q,n);if(f<best){best=f;bi=i;bd=d;}}
   }
   if(bi<0){cerr<<"STALLED "<<r<<" cur="<<cur<<"\n";return 1;}
   moveit(p,bi,bd);cur--;moves++;
  }
  total+=moves;mx=max(mx,moves);if(r%64==0)cerr<<r<<" avg="<<double(total)/(r+1)<<" last="<<moves<<" future="<<futurecost(p,n)<<"\n";
 }
 cerr<<"total="<<total<<" avg="<<double(total)/rounds<<" max="<<mx<<"\n";
}



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
pair<int,int> fastmove(const vector<int>&p,U64 z,int n,bool verify=false){
 int nxt[65],pos[65],suf[67],q[65]{},bad0[65]{};bool wanted[65],m0[65]{},before0[65]{};double pw[66];pw[0]=1;for(int i=1;i<=65;i++)pw[i]=pw[i-1]*.5;
 int nr=n;for(int j=n;j>=0;j--){wanted[j]=j==n||((z>>j)&1);if(wanted[j])nr=j;nxt[j]=nr;}for(int i=0;i<=n;i++)pos[p[i]]=i;
 suf[n+1]=n+1;int curr=0;for(int i=n;i>=0;i--){int k=p[i];m0[k]=k>suf[i+1];curr+=m0[k];suf[i]=min(suf[i+1],wanted[k]?k:n+1);}
 for(int k=0;k<=n;k++)if(!wanted[k]){before0[k]=pos[k]<pos[nxt[k]];if(before0[k]&&!m0[k])bad0[nxt[k]]++;}
 for(int k=0;k<=n;k++)if(wanted[k]&&!m0[k]&&bad0[k])curr++;
 double f0=0;for(int i=0;i<=n;i++){int k=p[i];for(int j=i+1;j<=n;j++)q[k]+=p[j]<k;f0+=1-pw[q[k]];}
 int bi=-1,bd=-1,hi=-1;double bf=1e9;
 for(int i=0;i<=n;i++){
  int k=p[i];if(k<hi)continue;if(k==n&&hi==n-1)continue;hi=k;
  for(int dir:{-1,1}){
   bool mm[65],before[65];int bad[65];copy(m0,m0+n+1,mm);copy(before0,before0+n+1,before);copy(bad0,bad0+n+1,bad);int dc=curr,qk=q[k],suffix=suf[i+1];double f=f0;
   auto extra=[&](int g){return !mm[g]&&bad[g]>0;};
   auto setm=[&](int x,bool val){if(mm[x]==val)return;int g=nxt[x];dc-=extra(g);dc+=int(val)-int(mm[x]);if(!wanted[x]&&before[x])bad[g]+=int(mm[x])-int(val);mm[x]=val;dc+=extra(g);};
   auto setbefore=[&](int x,bool val){if(before[x]==val)return;int g=nxt[x];dc-=extra(g);if(!mm[x])bad[g]+=int(val)-int(before[x]);before[x]=val;dc+=extra(g);};
   for(int d=i+dir;d>=0&&d<=n;d+=dir){
    int x=p[d];
    if(dir==1){if(wanted[k]&&x>k)setm(x,true);setm(k,k>suf[d+1]);if(!wanted[x]&&nxt[x]==k)setbefore(x,true);if(!wanted[k]&&nxt[k]==x)setbefore(k,false);int old=qk;if(x<k)qk--;else f+=pw[q[x]+1];f+=pw[old]-pw[qk];}
    else{if(wanted[k]&&x>k)setm(x,x>suffix);if(wanted[x]&&k>x)setm(k,true);if(!wanted[x]&&nxt[x]==k)setbefore(x,false);if(!wanted[k]&&nxt[k]==x)setbefore(k,true);if(wanted[x])suffix=min(suffix,x);int old=qk;if(x<k)qk++;else f-=pw[q[x]];f+=pw[old]-pw[qk];}
    if(verify){auto pp=p;moveit(pp,i,d);if(dist(pp,z,n)!=dc){cerr<<"fast wrong "<<i<<" "<<d<<" z="<<z<<" calc="<<dc<<" ex="<<dist(pp,z,n)<<"\n";throw runtime_error("bad");}}
    if(dc==curr-1&&f<bf-1e-9){bf=f;bi=i;bd=d;}
   }
  }
 }
 return {bi,bd};
}
void check(int n){
 vector<vector<int>>ps; vector<int>p(n+1);iota(p.begin(),p.end(),0); do{ps.push_back(p);}while(next_permutation(p.begin(),p.end()));
 map<vector<int>,int>ids;for(int s=0;s<(int)ps.size();s++)ids[ps[s]]=s;
 vector<vector<int>>rev(ps.size());for(int s=0;s<(int)ps.size();s++){int hi=-1;for(int i=0;i<=n;i++){int x=ps[s][i];if(x<hi)continue;hi=x;for(int d=0;d<=n;d++)if(i!=d){auto q=ps[s];moveit(q,i,d);rev[ids[q]].push_back(s);}}}
 int errors=0;
 for(U64 z=0;z<(1ULL<<n);z++){vector<int>ds(ps.size(),999),q;for(int s=0;s<(int)ps.size();s++)if(shown(ps[s],n)==z)ds[s]=0,q.push_back(s);for(int h=0;h<(int)q.size();h++)for(int s:rev[q[h]])if(ds[s]>ds[q[h]]+1)ds[s]=ds[q[h]]+1,q.push_back(s);for(int s=0;s<(int)ps.size();s++)if(ds[s]!=dist(ps[s],z,n)){if(errors++<10){cerr<<"bad "<<z<<" true="<<ds[s]<<" calc="<<dist(ps[s],z,n)<<" perm=";for(int k:ps[s])cerr<<k<<",";cerr<<"\n";}}}
 int fe=0;for(auto&p:ps){double ex=0;for(U64 z=0;z<(1ULL<<n);z++){ex+=dist(p,z,n);fastmove(p,z,n,true);}ex/=1<<n;if(abs(ex-futurecost(p,n))>1e-8){if(fe++<5)cerr<<"future bad "<<ex<<" vs "<<futurecost(p,n)<<"\n";}}
 cerr<<"states="<<ps.size()<<" errors="<<errors<<" future_errors="<<fe<<"\n";
}
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 if(argc>1&&string(argv[1])=="check"){check(argc>2?atoi(argv[2]):5);return 0;}
 int n,K,P,R;cin>>n>>K>>P>>R;vector<U64>a(P);for(int v=0;v<P;v++){string s;cin>>s;for(int j=0;j<n;j++)if(s[j]=='0')a[v]|=1ULL<<j;}
 vector<int>cols(n),fl(n);iota(cols.begin(),cols.end(),0);int cd[64][64]{};for(int x=0;x<n;x++)for(int y=0;y<n;y++)for(U64 z:a)cd[x][y]+=((z>>x)^(z>>y))&1;
 auto score=[&](){int v=0;for(int i=1;i<n;i++){int d=cd[cols[i-1]][cols[i]];v+=(fl[i-1]^fl[i])?P-d:d;}return v;};
 mt19937 rnd(152237);int cost=score(),best=cost;auto bc=cols,bf=fl;for(int it=0;it<1000000;it++){int x=rnd()%n,y=rnd()%n;if(x>y)swap(x,y);bool inv=rnd()%3==0;if(inv)fl[x]^=1;else reverse(cols.begin()+x,cols.begin()+y+1),reverse(fl.begin()+x,fl.begin()+y+1);int nc=score();double temp=8*pow(.002,double(it)/1000000);if(nc<=cost||generate_canonical<double,32>(rnd)<exp((cost-nc)/temp)){cost=nc;if(cost<best)best=cost,bc=cols,bf=fl;}else if(inv)fl[x]^=1;else reverse(cols.begin()+x,cols.begin()+y+1),reverse(fl.begin()+x,fl.begin()+y+1);}
 cols=bc;fl=bf;for(U64&z:a){U64 old=z;z=0;for(int j=0;j<n;j++)z|=(((old>>cols[j])&1)^fl[j])<<j;}
 for(int k=0;k<K;k++){string s(n,'.');if(k<=n)for(int j=0;j<n&&j<=k;j++)s[cols[j]]=char('0'+((j<k)^fl[j]));cout<<s<<'\n';}cout.flush();
 vector<int>p(n+1);iota(p.begin(),p.end(),0);int total=0,mx=0,rounds=argc>1?atoi(argv[1]):R;
 for(int r=0;r<rounds;r++){
  int v;if(!(cin>>v)||v<0||v>=P)return 0;U64 z=a[v];int start=dist(p,z,n),cur=start,moves=0;vector<pair<int,int>>ops;
  while(shown(p,n)!=z){auto [bi,bd]=fastmove(p,z,n);
   if(bi<0){bi=find(p.begin(),p.end(),n-1)-p.begin();bd=find(p.begin(),p.end(),n)-p.begin();if(bi>bd){cerr<<"STALLED "<<r<<"\n";return 1;}}
   ops.push_back({p[bi],bd});moveit(p,bi,bd);cur--;moves++;
  }
  total+=moves;mx=max(mx,moves);cout<<ops.size()<<'\n';for(auto [k,d]:ops)cout<<k<<' '<<d<<'\n';cout.flush();
 }
 cerr<<"total="<<total<<" avg="<<double(total)/rounds<<" max="<<mx<<"\n";
}



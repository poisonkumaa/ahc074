#include <bits/stdc++.h>
using namespace std;
using U64=uint64_t;
struct Random{U64 x;U64 next(){x+=0x9e3779b97f4a7c15ULL;U64 z=x;z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;z=(z^(z>>27))*0x94d049bb133111ebULL;return z^(z>>31);}int pick(int n){return next()%n;}double real(){return (next()>>11)*0x1.0p-53;}};
struct Model{
 int n,b,ns,V,ne,allowmissing=0;vector<array<uint8_t,9>>perm;vector<vector<int>>nx;vector<int>fr,to;
 Model(int nn,int bb):n(nn),b(bb),V(1<<b),ne(n*(n-1)){
  array<uint8_t,9>p{};iota(p.begin(),p.begin()+n,0);do{perm.push_back(p);}while(next_permutation(p.begin(),p.begin()+n));ns=perm.size();nx.assign(ns,vector<int>(ne));
  for(int i=0;i<n;i++)for(int j=0;j<n;j++)if(i!=j)fr.push_back(i),to.push_back(j);
  for(int s=0;s<ns;s++)for(int e=0;e<ne;e++){auto q=perm[s];int i=fr[e],j=to[e],k=q[i];if(i<j)for(int x=i;x<j;x++)q[x]=q[x+1];else for(int x=i;x>j;x--)q[x]=q[x-1];q[j]=k;nx[s][e]=rank(q);}
 }
 int rank(const array<uint8_t,9>&p){int r=0;for(int i=0;i<n;i++){int z=0;for(int j=i+1;j<n;j++)z+=p[j]<p[i];r=r*(n-i)+z;}return r;}
 double eval(const vector<string>&p,int sims=16000){
  vector<int>m(n),c(n),disp(ns),vis(ns),seen(V);for(int k=0;k<n;k++)for(int j=0;j<b;j++)if(p[k][j]!='.'){m[k]|=1<<j;if(p[k][j]=='1')c[k]|=1<<j;}
  for(int s=0;s<ns;s++){int covered=0;for(int i=0;i<n;i++){int k=perm[s][i],v=m[k]&~covered;if(v)vis[s]|=1<<i;disp[s]|=c[k]&v;covered|=m[k];}seen[disp[s]]=1;}
  int valid=accumulate(seen.begin(),seen.end(),0);if(V-valid>allowmissing)return 1000;
  if(V>128)abort();
  vector<vector<int>>adj(ns);for(int s=0;s<ns;s++)for(int e=0;e<ne;e++)if(vis[s]>>fr[e]&1)adj[s].push_back(nx[s][e]);
  vector<vector<uint8_t>>dist(V,vector<uint8_t>(ns,255));vector<int>quality(ns);
  vector<array<U64,2>>reach(ns),newreach(ns);for(int s=0;s<ns;s++){reach[s][disp[s]>>6]|=1ULL<<(disp[s]&63);dist[disp[s]][s]=0;}
  int remaining=(valid-1)*ns;
  for(int d=1;remaining;d++){
   if(d>20)return 1000;
   for(int s=0;s<ns;s++){
    U64 x=reach[s][0],y=reach[s][1];for(int u:adj[s]){x|=reach[u][0];y|=reach[u][1];}newreach[s]={x,y};
    U64 dx=x&~reach[s][0],dy=y&~reach[s][1];int c=__builtin_popcountll(dx)+__builtin_popcountll(dy);remaining-=c;quality[s]+=d*c;
    while(dx){int t=__builtin_ctzll(dx);dist[t][s]=d;dx&=dx-1;}while(dy){int t=64+__builtin_ctzll(dy);dist[t][s]=d;dy&=dy-1;}
   }reach.swap(newreach);
  }
  vector<vector<int>>goal(V,vector<int>(ns,-1));
  auto solve=[&](auto&&self,int s,int t)->int{
   int &res=goal[t][s];if(res>=0)return res;if(disp[s]==t)return res=s;
   int bv=INT_MAX,bg=-1;for(int u:adj[s])if(dist[t][u]+1==dist[t][s]){int g=self(self,u,t);if(quality[g]<bv)bv=quality[g],bg=g;}return res=bg;
  };
  Random rng{812398123};int s=0,total=0;for(int i=0;i<sims+2000;i++){int t;do{t=rng.pick(V);}while(!seen[t]);if(i>=2000)total+=dist[t][s];s=solve(solve,s,t);}return double(total)/sims;
 }
};
int main(int argc,char**argv){int n=argc>1?atoi(argv[1]):7,b=argc>2?atoi(argv[2]):6,iters=argc>3?atoi(argv[3]):1000;bool dag=argc>4&&string(argv[4])=="dag",cycle=argc>4&&string(argv[4])=="cycle";Model md(n,b);vector<string>p={"0....0","10...0",".10.00","111000",".11100","11..10",".....1"};if(b>6){for(auto&s:p)s+=string(b-6,'0');for(int i=6;i<b;i++){string s(b,'.');s[i]='1';p.push_back(s);}}while((int)p.size()<n)p.push_back(string(b,'.'));
 if(dag){p.assign(n,string(b,'.'));for(int i=0;i<n;i++)for(int j=0;j<b;j++){if(j<i)p[i][j]='0';else if(j==i)p[i][j]='1';}}
 else if(cycle){p.assign(n,string(b,'.'));for(int i=0;i<n;i++){p[i][i]='1';p[(i+1)%n][i]='0';}md.allowmissing=2;}
 else if(argc>4){ifstream in(argv[4]);for(auto&s:p)in>>s;}
 Random rng{12398123};double cur=md.eval(p),best=cur;auto bp=p;cerr<<"initial="<<cur<<"\n";
 for(int it=0;it<iters;it++){
  auto cp=p;int changes=rng.pick(8)==0?2:1;
  for(int z=0;z<changes;z++){
   if(dag){int i=1+rng.pick(n-1),j=rng.pick(min(i,b));cp[i][j]=(cp[i][j]=='0'?'.':'0');}
   else cp[rng.pick(n)][rng.pick(b)]=".01"[rng.pick(3)];
  }
  double sc=md.eval(cp);double temp=(dag?.012:.04)*pow(.01,double(it)/max(1,iters));if(sc<cur||rng.real()<exp((cur-sc)/temp)){p=cp;cur=sc;}
  if(sc<best-1e-9){best=sc;bp=cp;cerr<<it<<" best="<<best<<"\n";for(auto&s:bp)cerr<<s<<"\n";ofstream out("tools/experiments/templates_agent_best_"+string(dag?"dag_":cycle?"cycle_":"")+to_string(b)+"_"+to_string(n)+".txt");for(auto&s:bp)out<<s<<"\n";}
  if(it%100==0)cerr<<"step="<<it<<" current="<<cur<<"\n";
 }
 cerr<<"FINAL "<<md.eval(bp,1000000)<<"\n";for(auto&s:bp)cout<<s<<"\n";
}

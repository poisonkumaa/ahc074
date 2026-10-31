#include <bits/stdc++.h>
using namespace std;using U=uint64_t;int pc(U x){return __builtin_popcountll(x);}mt19937 rng(712367);
struct Card{U m,b;};
int main(int argc,char**argv){
 int N,K,P,R;cin>>N>>K>>P>>R;vector<U>A(P);string s;for(U&a:A){cin>>s;for(int j=0;j<N;j++)if(s[j]=='1')a|=1ULL<<j;}
 int iters=argc>1?atoi(argv[1]):1000000;
 vector<array<int,8>>groups(K);vector<vector<int>>mem(P);vector<int>ids;
 for(int i=0;i<8*K;i++)ids.push_back(i%P);shuffle(ids.begin(),ids.end(),rng);
 vector<Card>C(K);auto calc=[&](int i){auto g=groups[i];U b=A[g[0]],m=~(b^A[g[1]])&~(b^A[g[2]]);for(int z=3;z<8;z++)m&=~(b^A[g[z]]);return Card{m,b&m};};
 for(int i=0;i<K;i++){for(int j=0;j<8;j++){groups[i][j]=ids[i*8+j];mem[ids[i*8+j]].push_back(i);}C[i]=calc(i);}
 auto cost=[&](int p){U covered=0;for(int i:mem[p])covered|=C[i].m;int n=64-pc(covered);return n*n;};
 double cur=0;for(int p=0;p<P;p++)cur+=cost(p);double best=cur;auto bestG=groups;
 for(int it=0;it<iters;it++){
  int x=rng()%K,y=rng()%K,ix=rng()%8,iy=rng()%8;if(x==y)continue;
  int a=groups[x][ix],b=groups[y][iy];if(a==b)continue;
  bool dup=false;for(int t:groups[x])if(t==b)dup=true;for(int t:groups[y])if(t==a)dup=true;if(dup)continue;
  vector<int>ps;for(int t:groups[x])ps.push_back(t);for(int t:groups[y])ps.push_back(t);sort(ps.begin(),ps.end());ps.erase(unique(ps.begin(),ps.end()),ps.end());
  int before=0;for(int p:ps)before+=cost(p);
  auto cx=C[x],cy=C[y];swap(groups[x][ix],groups[y][iy]);
  for(int &v:mem[a])if(v==x){v=y;break;}for(int &v:mem[b])if(v==y){v=x;break;}
  C[x]=calc(x);C[y]=calc(y);int after=0;for(int p:ps)after+=cost(p);
  double temp=60*pow(0.015,double(it)/iters);
  if(after<=before || double(rng())/rng.max()<exp((before-after)/temp)){cur+=after-before;if(cur<best)best=cur,bestG=groups;}
  else {swap(groups[x][ix],groups[y][iy]);for(int &v:mem[a])if(v==y){v=x;break;}for(int &v:mem[b])if(v==x){v=y;break;}C[x]=cx;C[y]=cy;}
 }
 groups=bestG;for(int i=0;i<K;i++)C[i]=calc(i);
 double sum=0;int fail=0,remain=0,mx=0;double size=0;for(auto c:C)size+=pc(c.m);
 for(U a:A){U u=~0ULL;int n=0;while(u){int best=-1,score=0;for(int i=0;i<K;i++)if(((a^C[i].b)&C[i].m&u)==0){int t=pc(C[i].m&u);if(t>score)score=t,best=i;}if(best<0)break;u&=~C[best].m;n++;}sum+=n;fail+=u!=0;remain+=pc(u);mx=max(mx,n);}
 cerr<<"cost="<<best<<" size="<<size/K<<" mean="<<sum/P<<" fails="<<fail<<" remain="<<remain<<" max="<<mx<<endl;
 for(auto c:C){for(int j=0;j<N;j++)cout<<(!(c.m>>j&1)?'.':c.b>>j&1?'1':'0');cout<<'\n';}
}

#include "prefix_dp.h"
#include "global_agent_weighted.h"
struct Paper2{int x,y;U64 cx,cy;};
int main(int argc,char**argv){int M=argc>1?atoi(argv[1]):8,extra=argc>2?atoi(argv[2]):2*M;int NN,K,P,R;cin>>NN>>K>>P>>R;vector<U64>all(P);for(auto &v:all){string s;cin>>s;for(int j=0;j<NN;j++)if(s[j]=='1')v|=1ULL<<j;}P=min(P,8*M);vector<U64>X(P),Y(P);for(int v=0;v<P;v++)X[v]=all[v]&PrefixDP::low(M),Y[v]=(all[v]>>M)&PrefixDP::low(M);
 vector<PrefixPaper>a,b;for(int l=1;l<=M;l++){a.push_back({l,0});a.push_back({l,PrefixDP::low(l)});b.push_back({l,0});b.push_back({l,PrefixDP::low(l)});}train_prefix_dictionary(X,a,M,16,1,8);train_prefix_dictionary(Y,b,M,16,1,8);
 vector<Paper2>papers;for(auto p:a)papers.push_back({p.len,0,p.color,0});for(auto p:b)papers.push_back({0,p.len,0,p.color});int side=M+1,S=side*side,INF=1000;
 vector<vector<int>>F(P,vector<int>(S)),B(P,vector<int>(S)),Q(P,vector<int>(S));vector<int>old(P);
 auto dist=[&](){long long total=0;for(int v=0;v<P;v++){vector<int>sx,sy;for(auto p:papers){U64 dx=(p.cx^X[v])&PrefixDP::low(p.x),dy=(p.cy^Y[v])&PrefixDP::low(p.y);sx.push_back(dx?64-__builtin_clzll(dx):0);sy.push_back(dy?64-__builtin_clzll(dy):0);}auto &f=F[v],&b=B[v];fill(f.begin(),f.end(),INF);fill(b.begin(),b.end(),INF);f[0]=0;b[S-1]=0;for(int x=0;x<=M;x++)for(int y=0;y<=M;y++){int s=x*side+y;for(int k=0;k<(int)papers.size();k++)if(x>=sx[k]&&y>=sy[k]){auto p=papers[k];int nx=max(x,p.x),ny=max(y,p.y),t=nx*side+ny;if(t!=s)f[t]=min(f[t],f[s]+1);}}
   for(int x=M;x>=0;x--)for(int y=M;y>=0;y--){int s=x*side+y;for(int k=0;k<(int)papers.size();k++)if(x>=sx[k]&&y>=sy[k]){auto p=papers[k];int nx=max(x,p.x),ny=max(y,p.y),t=nx*side+ny;if(t!=s)b[s]=min(b[s],b[t]+1);}}old[v]=f[S-1];total+=old[v];}return double(total)/P;};
 cerr<<"base "<<dist()<<"\n";
 for(int it=0;it<extra;it++){
  int best=-1;Paper2 winner{};
  for(int lx=0;lx<=M;lx++)for(int ly=0;ly<=M;ly++)if(lx||ly){
   for(int v=0;v<P;v++){auto &q=Q[v];for(int x=M;x>=0;x--)for(int y=M;y>=0;y--){int s=x*side+y,t=max(x,lx)*side+max(y,ly);int z=t==s?INF:F[v][s]+1+B[v][t];if(x<M)z=min(z,q[s+side]);if(y<M)z=min(z,q[s+1]);q[s]=z;}}
   for(int sample=0;sample<P;sample++){int gain=0;for(int v=0;v<P;v++){U64 dx=(X[v]^X[sample])&PrefixDP::low(lx),dy=(Y[v]^Y[sample])&PrefixDP::low(ly);int sx=dx?64-__builtin_clzll(dx):0,sy=dy?64-__builtin_clzll(dy):0;gain+=max(0,old[v]-Q[v][sx*side+sy]);}if(gain>best){best=gain;winner={lx,ly,X[sample]&PrefixDP::low(lx),Y[sample]&PrefixDP::low(ly)};}}
  }
  papers.push_back(winner);double score=dist();cerr<<it<<" "<<winner.x<<","<<winner.y<<" gain="<<best<<" avg="<<score<<"\n";
 }
 cerr<<"FINAL "<<dist()<<" CPU "<<double(clock())/CLOCKS_PER_SEC<<"\n";for(auto p:papers)cout<<p.x<<" "<<p.y<<" "<<p.cx<<" "<<p.cy<<"\n";
}

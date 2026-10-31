#pragma once
#include <bits/stdc++.h>
using namespace std;
struct AugGraph {
 using U=uint64_t;
 struct Edge{uint32_t next:22,paper:4,pos:4;};
 int B,K,V,W,S,extras,limit;
 vector<string> pattern;
 vector<array<uint8_t,12>> perm;
 vector<Edge>edges;vector<int>off;
 vector<uint16_t>display,uq;
 vector<uint8_t>dist;
 vector<int>pc,extra_id,power;int fac[8];
 inline int distance(int s,int t) const {return (dist[size_t(s)*(V/2)+(t>>1)]>>((t&1)*4))&15;}
 inline void setdist(int s,int t,int d){auto &v=dist[size_t(s)*(V/2)+(t>>1)];int z=(t&1)*4;v=(v&~(15<<z))|(d<<z);}
 int rankp(const array<uint8_t,12>&a) const {
  int core=0,mask=127,left=7,seen=0,code=0;
  for(int i=0;i<K;i++){
   int x=a[i];if(x<7){core+=pc[mask&((1<<x)-1)]*fac[--left];mask^=1<<x;}
   else code+=pc[seen&((1<<x)-1)]*power[x-7];
   seen|=1<<x;
  }
  int e=extra_id[code];return e<0?-1:core*extras+e;
 }
 AugGraph(int bits,int maxdepth=2,bool improved=true):B(bits),K(bits+1),V(1<<bits),W(V/64),limit(maxdepth){
  if(B<6||B>10)abort();
  pattern=improved?vector<string>{"0....0","10...0","110100","11.000","111100","11..10",".....1"}:vector<string>{"0....0","10...0","110..0","111000","111100","111.10",".....1"};
  for(auto&s:pattern)s+=string(B-6,'0');
  for(int j=6;j<B;j++){string s(B,'.');s[j]='1';for(int k=j+1;k<B;k++)s[k]='0';pattern.push_back(s);}
  pc.resize(1<<K);for(int i=1;i<(1<<K);i++)pc[i]=pc[i/2]+(i&1);
  fac[0]=1;for(int i=1;i<8;i++)fac[i]=fac[i-1]*i;
  int codes=1;for(int j=0;j<B-6;j++){power.push_back(codes);codes*=K;}
  extra_id.assign(codes,-1);vector<vector<int>>pos;vector<int>now;
  function<void(int,int,int)>gen=[&](int depth,int cap,int code){
   if(depth==B-6){extra_id[code]=pos.size();pos.push_back(now);return;}
   for(int v=0;v<=cap;v++){now.push_back(v);gen(depth+1,v+1,code+v*power[depth]);now.pop_back();}
  };
  gen(0,limit,0);extras=pos.size();S=5040*extras;
  perm.reserve(S);array<uint8_t,12>a{};iota(a.begin(),a.begin()+7,0);
  do {for(auto&ps:pos){auto p=a;for(int j=0;j<B-6;j++){int n=7+j,d=ps[j];for(int i=n;i>d;i--)p[i]=p[i-1];p[d]=n;}perm.push_back(p);}}while(next_permutation(a.begin(),a.begin()+7));
  display.resize(S);uq.resize(S);off.resize(S+1);edges.reserve(size_t(S)*32);
  int masks[12]{},colors[12]{};for(int k=0;k<K;k++)for(int j=0;j<B;j++)if(pattern[k][j]!='.'){masks[k]|=1<<j;if(pattern[k][j]=='1')colors[k]|=1<<j;}
  for(int s=0;s<S;s++){
   if(rankp(perm[s])!=s)abort();off[s]=edges.size();int covered=0,vis=0;
   for(int i=0;i<K;i++){int k=perm[s][i],ex=masks[k]&~covered;if(ex)vis|=1<<k;display[s]|=ex&colors[k];covered|=masks[k];}
   for(int i=0;i<K;i++)if(vis>>perm[s][i]&1)for(int d=0;d<K;d++)if(i!=d){
    if(i&&d==i-1&&(vis>>perm[s][i-1]&1))continue;
    auto p=perm[s];int k=p[i];if(i<d)for(int j=i;j<d;j++)p[j]=p[j+1];else for(int j=i;j>d;j--)p[j]=p[j-1];p[d]=k;
    int ns=rankp(p);if(ns>=0)edges.push_back({(uint32_t)ns,(uint32_t)k,(uint32_t)d});
   }
  }
  off[S]=edges.size();
  vector<U>cur(size_t(S)*W),nxt(size_t(S)*W);dist.assign(size_t(S)*(V/2),255);
  for(int s=0;s<S;s++){cur[size_t(s)*W+display[s]/64]|=U(1)<<(display[s]%64);setdist(s,display[s],0);}
  for(int d=1;d<=14;d++){
   bool done=true;
   for(int s=0;s<S;s++){
    U reach[16];const U*src=cur.data()+size_t(s)*W;U all=~U(0);
    for(int k=0;k<W;k++)reach[k]=src[k],all&=reach[k];
    if(all!=~U(0))for(int e=off[s];e<off[s+1];e++){const U*v=cur.data()+size_t(edges[e].next)*W;for(int k=0;k<W;k++)reach[k]|=v[k];}
    all=~U(0);
    for(int k=0;k<W;k++){
     U added=reach[k]&~src[k];while(added){int bit=__builtin_ctzll(added);setdist(s,k*64+bit,d);uq[s]+=d;added&=added-1;}
     nxt[size_t(s)*W+k]=reach[k];all&=reach[k];
    }
    if(all!=~U(0))done=false;
   }
   cur.swap(nxt);if(done)break;if(d==14)throw runtime_error("restricted graph unreachable or distance exceeds nibble");
  }
 }
};
struct AugPolicy {
 bool fast=false;
 AugGraph&g;vector<int>f;vector<uint16_t>quality;unordered_map<uint32_t,uint16_t>cache;
 AugPolicy(AugGraph&graph,vector<int>frequency={}):g(graph),f(move(frequency)),quality(g.S,65535){cache.reserve(100000);}
 uint16_t qual(int s){auto&v=quality[s];if(v!=65535)return v;if(f.empty())return v=g.uq[s];int sum=0;for(int t=0;t<g.V/2;t++){int d=g.dist[size_t(s)*(g.V/2)+t];sum+=f[2*t]*(d&15)+f[2*t+1]*(d>>4);}return v=sum;}
 uint16_t calc(int t,int s){int d=g.distance(s,t);if(!d)return qual(s);uint32_t key=(uint32_t(s)<<g.B)|t;auto it=cache.find(key);if(it!=cache.end())return it->second;uint16_t val=65535;for(int e=g.off[s];e<g.off[s+1];e++){auto op=g.edges[e];if(g.distance(op.next,t)+1==d)val=min(val,calc(t,op.next));}cache.emplace(key,val);return val;}
 AugGraph::Edge step(int t,int s){int d=g.distance(s,t);if(fast){int best=INT_MAX,be=-1;for(int e=g.off[s];e<g.off[s+1];e++){auto op=g.edges[e];if(g.distance(op.next,t)+1!=d)continue;int q=quality[op.next]==65535?g.uq[op.next]*256/g.V:quality[op.next];if(q<best){best=q;be=e;}}if(be<0)abort();return g.edges[be];}int val=calc(t,s);for(int e=g.off[s];e<g.off[s+1];e++){auto op=g.edges[e];if(g.distance(op.next,t)+1==d&&calc(t,op.next)==val)return op;}abort();}
};

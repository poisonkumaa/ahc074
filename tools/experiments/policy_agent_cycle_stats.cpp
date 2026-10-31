// AHC074 v3 -- C++17, reactive solver.
// Uses only the 256 initial barcodes when optimizing the paper design.
// Future orders are read one at a time, only after the previous answer is flushed.
// All moves follow exact shortest paths in precomputed legal state graphs.
// No external files, libraries, command-line arguments, or environment variables.

#include <bits/stdc++.h>
using namespace std;
using U64 = uint64_t;
struct Random {
 U64 x;
 U64 next(){ x += 0x9e3779b97f4a7c15ULL; U64 z=x; z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL; z=(z^(z>>27))*0x94d049bb133111ebULL; return z^(z>>31); }
 int pick(int n){return int(next()%n);} double real(){return (next()>>11)*0x1.0p-53;}
};
// A vertex is a permutation of one group's papers.
// An edge is one LEGAL visible-paper removal and insertion.
struct Graph {
 struct Edge { uint16_t next; uint8_t paper,position; };
 int bits,K,states,values,choices;
 vector<string> pattern;
 vector<array<uint8_t,8>> perm;
 vector<vector<Edge>> edges;
 vector<uint8_t> display, visible;
 vector<vector<uint8_t>> distance;
 vector<vector<uint16_t>> bfs;
 vector<double> transition;
 Graph(vector<string> p):bits(p[0].size()),K(p.size()),values(1<<bits),choices(K*(K-1)),pattern(move(p)) { build(); }
 int rank(const array<uint8_t,8>&a) const {
  int r=0; for(int i=0;i<K;i++){int less=0;for(int j=i+1;j<K;j++)less+=a[j]<a[i];r=r*(K-i)+less;} return r;
 }
 void build(){
  array<uint8_t,8> a{};iota(a.begin(),a.begin()+K,0);
  do {perm.push_back(a);}while(next_permutation(a.begin(),a.begin()+K));
  states=perm.size();edges.resize(states);display.assign(states,0);visible.assign(states,0);
  vector<int>mask(K),color(K);
  for(int k=0;k<K;k++)for(int j=0;j<bits;j++)if(pattern[k][j]!='.'){mask[k]|=1<<j;if(pattern[k][j]=='1')color[k]|=1<<j;}
  vector<vector<uint16_t>>reverse(states);
  for(int s=0;s<states;s++){
   int covered=0;
   for(int i=0;i<K;i++){int k=perm[s][i],v=mask[k]&~covered;if(v)visible[s]|=1<<k;display[s]|=color[k]&v;covered|=mask[k];}
   edges[s].reserve(choices);
   for(int i=0;i<K;i++)for(int d=0;d<K;d++)if(i!=d){
    auto p=perm[s];int k=p[i];if(i<d)for(int j=i;j<d;j++)p[j]=p[j+1];else for(int j=i;j>d;j--)p[j]=p[j-1];p[d]=k;
    int v=rank(p);edges[s].push_back({(uint16_t)v,(uint8_t)k,(uint8_t)d});
    if(visible[s]>>k&1)reverse[v].push_back((uint16_t)s);
   }
  }
  distance.assign(values,vector<uint8_t>(states,255));bfs.resize(values);
  for(int t=0;t<values;t++){
   auto &d=distance[t];auto &q=bfs[t];q.reserve(states);
   for(int s=0;s<states;s++)if(display[s]==t){d[s]=0;q.push_back(s);}
   for(size_t head=0;head<q.size();head++){int v=q[head];for(int u:reverse[v])if(d[u]==255){d[u]=d[v]+1;q.push_back((uint16_t)u);}}
   if(!q.empty()&&(int)q.size()!=states)throw runtime_error("Unreachable display in template");
  }
 }
// Among shortest paths, minimize the expected cost from the FINAL state.
 vector<uint8_t> make_policy(const vector<int>&frequency) const {
  vector<int> quality(states,0),cost(states);
  for(int t=0;t<values;t++)if(frequency[t])for(int s=0;s<states;s++)quality[s]+=frequency[t]*distance[t][s];
  vector<uint8_t> policy(values*states,255);
  for(int t=0;t<values;t++){
   if(!frequency[t])continue;
   for(int s:bfs[t]){
    int d=distance[t][s];if(!d){cost[s]=quality[s];continue;}
    int best=INT_MAX,be=-1;
    for(int e=0;e<choices;e++){
     const auto &v=edges[s][e];
     if(!(visible[s]>>v.paper&1)||distance[t][v.next]+1!=d)continue;
     if(cost[v.next]<best){best=cost[v.next];be=e;}
    }
    if(be<0)throw runtime_error("Missing legal shortest-path edge");
    cost[s]=best;policy[t*states+s]=(uint8_t)be;
   }
  }
  return policy;
 }
 // Estimate transition costs using internally generated uniform targets.
 // These are not, and cannot access, the future orders from the judge.
 void build_transition(){
  vector<int>fr(values,1);for(int t=0;t<values;t++)if(distance[t][0]==255)fr[t]=0;auto policy=make_policy(fr);
  vector<int> visits(states,0),total(values,0);Random rnd{0x45123a1bcULL};int s=0;
  for(int i=0;i<120000;i++){
   int t=rnd.pick(values);if(!fr[t]){i--;continue;}
   while(display[s]!=t){int e=policy[t*states+s];s=edges[s][e].next;}
   if(i>=2000){visits[s]++;total[t]++;}
  }
  transition.assign(values*values,0);
  for(int s=0;s<states;s++)if(visits[s]){
   int a=display[s];double w=double(visits[s])/total[a];
   for(int b=0;b<values;b++)if(fr[b])transition[a*values+b]+=w*distance[b][s];
  }
 }
};

int main(){
 for(int bits=4;bits<=8;bits++){
  vector<string> p(bits,string(bits,'.'));for(int i=0;i<bits;i++)p[i][i]='0',p[i][(i+1)%bits]='1';
  Graph g(p);g.build_transition();double avg=accumulate(g.transition.begin(),g.transition.end(),0.)/((g.values-2)*(g.values-2));int md=0;
  for(auto&v:g.distance)for(int x:v)if(x<255)md=max(md,x);
  cerr<<"bits "<<g.bits<<" states "<<g.states<<" uniformcost "<<avg<<" maxdistance "<<md<<endl;
 }
}

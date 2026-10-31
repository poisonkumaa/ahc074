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
 vector<array<uint8_t,7>> perm;
 vector<vector<Edge>> edges;
 vector<uint8_t> display, visible;
 vector<vector<uint8_t>> distance;
 vector<vector<uint16_t>> bfs;
 vector<double> transition;
 Graph(vector<string> p):bits(p[0].size()),K(p.size()),values(1<<bits),choices(K*(K-1)),pattern(move(p)) { build(); }
 int rank(const array<uint8_t,7>&a) const {
  int r=0; for(int i=0;i<K;i++){int less=0;for(int j=i+1;j<K;j++)less+=a[j]<a[i];r=r*(K-i)+less;} return r;
 }
 void build(){
  array<uint8_t,7> a{};iota(a.begin(),a.begin()+K,0);
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
   if((int)q.size()!=states)throw runtime_error("Unreachable display in template");
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
  auto policy=make_policy(vector<int>(values,1));
  vector<int> visits(states,0),total(values,0);Random rnd{0x45123a1bcULL};int s=0;
  for(int i=0;i<120000;i++){
   int t=rnd.pick(values);
   while(display[s]!=t){int e=policy[t*states+s];s=edges[s][e].next;}
   if(i>=2000){visits[s]++;total[t]++;}
  }
  transition.assign(values*values,0);
  for(int s=0;s<states;s++)if(visits[s]){
   int a=display[s];double w=double(visits[s])/total[a];
   for(int b=0;b<values;b++)transition[a*values+b]+=w*distance[b][s];
  }
 }
};
struct FastGroup {
 Graph *graph;
 int start,base,state=0,bits=0,flip=0;
 array<int,6> col{};
 array<uint8_t,256> code{};
 array<int,64> count{};
 double score=0;
 vector<uint8_t> policy;
};
int main(){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int N,K,P,R;if(!(cin>>N>>K>>P>>R)||N!=64||K!=192||P!=256)return 0;
 const clock_t started=clock();
 vector<string>A(P);vector<U64>data(P);
 for(int a=0;a<P;a++){cin>>A[a];for(int j=0;j<N;j++)if(A[a][j]=='1')data[a]|=1ULL<<j;}
 Graph six({"0....0","10...0",".10.00","111000",".11100","11..10",".....1"});
 Graph four({"1110","...1","0000","1.00","..1.","0100"});
 six.build_transition();four.build_transition();
 vector<FastGroup> groups(11);int paper=0;
 int belongs[64],local[64];
 for(int g=0;g<11;g++){
  auto &q=groups[g];q.graph=g<10?&six:&four;q.start=g*6;q.base=paper;paper+=q.graph->K;q.bits=q.graph->bits;
  for(int j=0;j<q.bits;j++){q.col[j]=q.start+j;belongs[q.start+j]=g;local[q.start+j]=j;}
  for(int a=0;a<P;a++){
   int code=0;for(int j=0;j<q.bits;j++)code|=((data[a]>>q.col[j])&1)<<j;
   q.code[a]=code;q.count[code]++;
  }
 }
 auto cost=[&](const FastGroup &g){
  int n=g.graph->values;const double*mat=g.graph->transition.data();double v=0;
  for(int a=0;a<n;a++){
   const double*row=mat+a*n;double s=0;
   for(int b=0;b<n;b++)s+=g.count[b]*row[b];
   v+=g.count[a]*s;
  }
  return v/(P*double(P));
 };
 double current=0;for(auto&g:groups){g.score=cost(g);current+=g.score;}
 auto bestgroups=groups;double best=current;
 Random rnd{data[0]^data[73]^0x61bb156e123ULL};
 const int STEPS=100000;
 int nit=0;
 for(int it=0;it<STEPS;it++){
  nit=it;
  if((it&255)==0&&double(clock()-started)/CLOCKS_PER_SEC>1.3)break;
  int x=rnd.pick(64),y=rnd.pick(64);bool inv=rnd.pick(5)==0;
  int g=belongs[x],h=inv?g:belongs[y],b=local[x],c=local[y];
  auto ng=groups[g],nh=groups[h];
  if(inv){
   ng.flip^=1<<b;ng.count.fill(0);
   for(int a=0;a<P;a++){ng.code[a]^=1<<b;ng.count[ng.code[a]]++;}
  }else if(g==h){
   if(b==c)continue;
   swap(ng.col[b],ng.col[c]);int diff=((ng.flip>>b)^(ng.flip>>c))&1;
   ng.flip^=(diff<<b)|(diff<<c);ng.count.fill(0);
   for(int a=0;a<P;a++){int d=((ng.code[a]>>b)^(ng.code[a]>>c))&1;ng.code[a]^=(d<<b)|(d<<c);ng.count[ng.code[a]]++;}
  }else{
   swap(ng.col[b],nh.col[c]);int diff=((ng.flip>>b)^(nh.flip>>c))&1;
   ng.flip^=diff<<b;nh.flip^=diff<<c;ng.count.fill(0);nh.count.fill(0);
   for(int a=0;a<P;a++){int d=((ng.code[a]>>b)^(nh.code[a]>>c))&1;ng.code[a]^=d<<b;nh.code[a]^=d<<c;ng.count[ng.code[a]]++;nh.count[nh.code[a]]++;}
  }
  ng.score=cost(ng);double delta=ng.score-groups[g].score;
  if(g!=h){nh.score=cost(nh);delta+=nh.score-groups[h].score;}
  double progress=double(it)/STEPS,temp=.02*pow(.01,progress);
  if(delta<=0||rnd.real()<exp(-delta/temp)){
   current+=delta;groups[g]=move(ng);if(g!=h)groups[h]=move(nh);
   if(current<best){best=current;bestgroups=groups;}
  }
 }
 groups=move(bestgroups);
 vector<string>design(K,string(N,'.'));
 for(auto &g:groups){
  const auto &gr=*g.graph;
  g.policy=gr.make_policy(vector<int>(g.count.begin(),g.count.begin()+gr.values));
  for(int k=0;k<gr.K;k++)for(int j=0;j<g.bits;j++){
   char c=gr.pattern[k][j];if(c!='.')design[g.base+k][g.col[j]]=char('0'+((c-'0')^((g.flip>>j)&1)));
  }
 }
 cerr<<"Cost="<<best<<" SA="<<nit<<" CPU="<<double(clock()-started)/CLOCKS_PER_SEC<<'\n';
 for(const auto &s:design)cout<<s<<'\n';cout.flush();
 for(int r=0;r<R;r++){
  int a;if(!(cin>>a)||a<0||a>=P)return 0;
  vector<pair<int,int>>ops;ops.reserve(64);
  for(auto&g:groups){
   const auto &gr=*g.graph;int t=g.code[a];
   while(gr.display[g.state]!=t){int e=g.policy[t*gr.states+g.state];if(e>=gr.choices)return 0;const auto&v=gr.edges[g.state][e];ops.emplace_back(g.base+v.paper,g.base+v.position);g.state=v.next;}
  }
  if(ops.size()>size_t(2*K))return 0;
  cout<<ops.size()<<'\n';for(auto[k,d]:ops)cout<<k<<' '<<d<<'\n';cout.flush();
 }
}

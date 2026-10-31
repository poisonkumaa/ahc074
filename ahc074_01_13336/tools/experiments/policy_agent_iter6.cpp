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
  vector<double> quality(states,0),cost(states),nextquality(states);
  int sum=accumulate(frequency.begin(),frequency.end(),0);
  for(int t=0;t<values;t++)if(frequency[t])for(int s=0;s<states;s++)quality[s]+=double(frequency[t])/sum*distance[t][s];
  vector<uint8_t> policy(values*states,255);
  const int ITER = 6;
  for(int it=0;it<ITER;it++){
   fill(nextquality.begin(),nextquality.end(),0);
   for(int t=0;t<values;t++){
    if(!frequency[t])continue;
    for(int s:bfs[t]){
     int d=distance[t][s];if(!d){cost[s]=quality[s];continue;}
     double best=1e100;int be=-1;
     for(int e=0;e<choices;e++){
      const auto &v=edges[s][e];
      if(!(visible[s]>>v.paper&1)||distance[t][v.next]+1!=d)continue;
      if(cost[v.next]<best-1e-9){best=cost[v.next];be=e;}
     }
     if(be<0)throw runtime_error("Missing legal shortest-path edge");
     cost[s]=best;policy[t*states+s]=(uint8_t)be;
    }
    double prob=double(frequency[t])/sum;
    for(int s=0;s<states;s++)nextquality[s]+=prob*(distance[t][s]+cost[s]);
   }
   double bias=nextquality[0];for(double &x:nextquality)x-=bias;
   quality.swap(nextquality);
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
struct Group { Graph *graph; int start,base,state=0;vector<int> freq;vector<uint8_t> policy; };
int main(){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int N,K,P,R;if(!(cin>>N>>K>>P>>R)||N!=64||K!=192||P!=256)return 0;
 const clock_t started = clock();
 vector<string>A(P);vector<U64> data(P);for(int a=0;a<P;a++){cin>>A[a];if((int)A[a].size()!=N)return 0;for(int j=0;j<N;j++)if(A[a][j]=='1')data[a]|=1ULL<<j;}
 // Fixed templates: 6 windows / 7 papers and 4 windows / 6 papers.
 Graph six({"0....0","10...0",".10.00","111000",".11100","11..10",".....1"});
 Graph four({"1110","...1","0000","1.00","..1.","0100"});
 six.build_transition();four.build_transition();
 vector<Group> groups;int paper=0;
 for(int g=0;g<10;g++){groups.push_back({&six,g*6,paper});paper+=six.K;}
 groups.push_back({&four,60,paper});paper+=four.K;
 array<int,64> cols,flip{};iota(cols.begin(),cols.end(),0);
 int belongs[64];for(int g=0;g<11;g++)for(int j=0;j<groups[g].graph->bits;j++)belongs[groups[g].start+j]=g;
 // Assignment and color inversion depend only on the initial barcode list.
 auto frequency=[&](int g){
  const auto &group=groups[g];int b=group.graph->bits;vector<int>f(1<<b,0);
  for(U64 a:data){int t=0;for(int j=0;j<b;j++){int p=group.start+j;t|=(((a>>cols[p])&1)^flip[p])<<j;}f[t]++;}return f;
 };
 auto cost=[&](int g,const vector<int>&f){
  const auto &gr=*groups[g].graph;double v=0;for(int a=0;a<gr.values;a++)if(f[a])for(int b=0;b<gr.values;b++)v+=double(f[a])*f[b]*gr.transition[a*gr.values+b];return v/(P*double(P));
 };
 vector<vector<int>>freq(11);vector<double>score(11);double current=0;
 for(int g=0;g<11;g++){freq[g]=frequency(g);score[g]=cost(g,freq[g]);current+=score[g];}
 auto bestcols=cols,bestflip=flip;double best=current;
 Random rnd{data[0]^data[73]^0x61bb156e123ULL};
 // Simulated annealing over window assignments and 0/1 inversions.
 // The timer only limits this optimization; legal graph construction is exact.
 const int STEPS=24000;
 for(int it=0;it<STEPS;it++){
  if ((it & 255) == 0 && double(clock()-started)/CLOCKS_PER_SEC > 0.75) break;
  int x=rnd.pick(64),y=rnd.pick(64);bool inv=rnd.pick(5)==0;int gx=belongs[x],gy=inv?gx:belongs[y];
  if(inv)flip[x]^=1;else{swap(cols[x],cols[y]);swap(flip[x],flip[y]);}
  auto fx=frequency(gx);double sx=cost(gx,fx);vector<int>fy;double sy=0;
  double delta=sx-score[gx];if(gx!=gy){fy=frequency(gy);sy=cost(gy,fy);delta+=sy-score[gy];}
  double progress=double(it)/STEPS,temp=.035*pow(.015,progress);
  if(delta<=0||rnd.real()<exp(-delta/temp)){
   current+=delta;freq[gx]=move(fx);score[gx]=sx;if(gx!=gy){freq[gy]=move(fy);score[gy]=sy;}
   if(current<best){best=current;bestcols=cols;bestflip=flip;}
  }else if(inv)flip[x]^=1;else{swap(cols[x],cols[y]);swap(flip[x],flip[y]);}
 }
 cols=bestcols;flip=bestflip;
 vector<string>design(K,string(N,'.'));
 for(int g=0;g<11;g++){
  auto &group=groups[g];const auto &gr=*group.graph;
  group.freq=frequency(g);group.policy=gr.make_policy(group.freq);
  for(int k=0;k<gr.K;k++)for(int j=0;j<gr.bits;j++){
   char c=gr.pattern[k][j];if(c=='.')continue;int p=group.start+j;
   design[group.base+k][cols[p]]=char('0'+((c-'0')^flip[p]));
  }
 }
 // Output the fixed design before reading the first order.
 for(const auto &s:design)cout<<s<<'\n';cout.flush();
 // Reactive phase: read exactly one order, answer it, and flush.
 for(int r=0;r<R;r++){
  int a;if(!(cin>>a)||a<0||a>=P)return 0;
  vector<pair<int,int>> ops;ops.reserve(64);
  for(auto &group:groups){
   const auto &gr=*group.graph;int target=0;
   for(int j=0;j<gr.bits;j++){int p=group.start+j;target|=(((data[a]>>cols[p])&1)^flip[p])<<j;}
   while(gr.display[group.state]!=target){
    int e=group.policy[target*gr.states+group.state];if(e>=gr.choices)return 0;
    const auto &edge=gr.edges[group.state][e];ops.emplace_back(group.base+edge.paper,group.base+edge.position);group.state=edge.next;
   }
  }
  if(ops.size()>size_t(2*K))return 0;
  cout<<ops.size()<<'\n';for(auto [k,d]:ops)cout<<k<<' '<<d<<'\n';cout.flush();
 }
 return 0;
}

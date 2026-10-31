#define main v3_main
#include "/mnt/c/Users/dokuk/Downloads/ahc074_v3_main.cpp"
#undef main
struct Frozen {
 Graph g; int base;vector<vector<uint8_t>> dd;vector<vector<uint16_t>> qq;vector<int> reset;vector<array<int,7>>shift;vector<uint8_t>pol;
 Frozen(vector<string> p,int b):g(p),base(b){
  int S=g.states,V=g.values;vector<vector<uint16_t>>rev(S);reset.resize(S);shift.resize(S);
  for(int s=0;s<S;s++){
   auto p=g.perm[s];int i=find(p.begin(),p.begin()+g.K,base)-p.begin();for(int j=i;j+1<g.K;j++)p[j]=p[j+1];p[g.K-1]=base;reset[s]=g.rank(p);
   for(int d=0;d<g.K;d++){auto q=p;for(int j=g.K-1;j>d;j--)q[j]=q[j-1];q[d]=base;shift[s][d]=g.rank(q);}
   for(auto e:g.edges[s])if(e.paper!=base&&(g.visible[s]>>e.paper&1))rev[e.next].push_back(s);
  }
  dd.assign(V,vector<uint8_t>(S,255));qq.resize(V);int bad=0;
  for(int t=0;t<V;t++){auto&d=dd[t];auto&q=qq[t];for(int s=0;s<S;s++)if(g.display[s]==t)d[s]=0,q.push_back(s);for(int h=0;h<(int)q.size();h++)for(int s:rev[q[h]])if(d[s]==255)d[s]=d[q[h]]+1,q.push_back(s);for(int s=0;s<S;s++)if(dd[t][reset[s]]==255)bad++;}
  cerr<<"frozen "<<g.bits<<" badreset="<<bad<<"\n";
  vector<int>quality(S),cost(S);for(int s=0;s<S;s++)for(int t=0;t<V;t++){int best=255;for(int d=0;d<g.K;d++)best=min(best,int(dd[t][shift[s][d]]));quality[s]+=best;}
  pol.assign(V*S,255);
  for(int t=0;t<V;t++)for(int s:qq[t]){int d=dd[t][s];if(!d){cost[s]=quality[s];continue;}int best=INT_MAX,be=-1;for(int e=0;e<g.choices;e++){auto v=g.edges[s][e];if(v.paper==base||!(g.visible[s]>>v.paper&1)||dd[t][v.next]+1!=d)continue;if(cost[v.next]<best)best=cost[v.next],be=e;}if(be<0)throw runtime_error("bad frozen policy");cost[s]=best;pol[t*S+s]=be;}
 }
};
#ifndef MERGED_LIBRARY_ONLY
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);int N,K,P,R;if(!(cin>>N>>K>>P>>R))return 0;vector<string>A(P);for(auto&s:A)cin>>s;
 Frozen six({"0....0","10...0",".10.00","111000",".11100","11..10",".....1"},4);
 Frozen four({"1110","...1","0000","1.00","..1.","0100"},2);
 struct B{Frozen*f;int col,state;vector<int>ids;};vector<B>bs;int np=1;
 vector<string>design(K,string(N,'.'));
 for(int b=0;b<11;b++){auto*f=b==10?&four:&six;int col=b*6;B v{f,col,0,{}};for(int k=0;k<f->g.K;k++){int id=k==f->base?0:np++;v.ids.push_back(id);for(int j=0;j<f->g.bits;j++)design[id][col+j]=f->g.pattern[k][j];}bs.push_back(v);}
 vector<int>order(K);iota(order.begin(),order.end(),0);
 // Actual order places shared paper first; infer each local permutation.
 for(auto&b:bs){array<uint8_t,7>p{};int c=0;for(int id:order){auto it=find(b.ids.begin(),b.ids.end(),id);if(it!=b.ids.end())p[c++]=it-b.ids.begin();}b.state=b.f->g.rank(p);}
 for(auto&s:design)cout<<s<<'\n';cout.flush();long total=0;int forced=0;
 auto apply=[&](int id,int d,vector<pair<int,int>>&ops){auto it=find(order.begin(),order.end(),id);order.erase(it);order.insert(order.begin()+d,id);ops.push_back({id,d});};
 for(int r=0;r<R;r++){
  int a;cin>>a;if(a<0)return 0;vector<pair<int,int>>ops;
  bool visible=false;for(auto&b:bs)visible|=bool(b.f->g.visible[b.state]>>b.f->base&1);
  if(!visible){auto&b=bs[0];int k=3,id=b.ids[k];int d=find(order.begin(),order.end(),0)-order.begin();int i=find(order.begin(),order.end(),id)-order.begin();if(i<d)d--;apply(id,d+1,ops);forced++;for(auto&bb:bs){array<uint8_t,7>p{};int c=0;for(int pid:order){auto it=find(bb.ids.begin(),bb.ids.end(),pid);if(it!=bb.ids.end())p[c++]=it-bb.ids.begin();}bb.state=bb.f->g.rank(p);}}
  vector<int>tg(11),depth(11),owner(K,-1);vector<array<int,7>>dc(11);
  int sum=0;for(int b=0;b<11;b++){auto&bb=bs[b];for(int j=0;j<bb.f->g.bits;j++)if(A[a][bb.col+j]=='1')tg[b]|=1<<j;for(int id:bb.ids)if(id)owner[id]=b;for(int d=0;d<bb.f->g.K;d++)dc[b][d]=bb.f->dd[tg[b]][bb.f->shift[bb.state][d]];sum+=dc[b][0];}
  int old=find(order.begin(),order.end(),0)-order.begin(),best=INT_MAX,bd=0;
  vector<int>removed=order;removed.erase(removed.begin()+old);
  for(int d=0;d<K;d++){int cost=sum+(d!=old);if(cost<best)best=cost,bd=d;if(d+1<K){int b=owner[removed[d]];if(b>=0){sum-=dc[b][depth[b]];depth[b]++;sum+=dc[b][depth[b]];}}}
  if(bd!=old)apply(0,bd,ops);
  fill(depth.begin(),depth.end(),0);for(int d=0;d<bd;d++){int b=owner[removed[d]];if(b>=0)depth[b]++;}for(int b=0;b<11;b++)bs[b].state=bs[b].f->shift[bs[b].state][depth[b]];
  for(auto&b:bs){auto&f=*b.f;auto&g=f.g;int t=0;for(int j=0;j<g.bits;j++)if(A[a][b.col+j]=='1')t|=1<<j;
   if(f.dd[t][b.state]==255){cerr<<"unreachable";return 1;}
   while(g.display[b.state]!=t){int e=f.pol[t*g.states+b.state];auto v=g.edges[b.state][e];int id=b.ids[v.paper];auto pp=g.perm[v.next];int local=0;while(pp[local]!=v.paper)local++;int d;if(local+1<g.K){int before=b.ids[pp[local+1]];d=find(order.begin(),order.end(),before)-order.begin();int i=find(order.begin(),order.end(),id)-order.begin();if(i<d)d--;}else{int after=b.ids[pp[local-1]];d=find(order.begin(),order.end(),after)-order.begin();int i=find(order.begin(),order.end(),id)-order.begin();if(i<d)d--;d++;}apply(id,d,ops);b.state=v.next;}
  }
  total+=ops.size();cout<<ops.size()<<'\n';for(auto [k,d]:ops)cout<<k<<' '<<d<<'\n';cout.flush();
 }
 cerr<<"total="<<total<<" forced="<<forced<<"\n";
}
#endif

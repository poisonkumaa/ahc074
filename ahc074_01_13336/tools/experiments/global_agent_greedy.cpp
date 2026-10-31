#define MERGED_LIBRARY_ONLY
#include "global_agent_merged.cpp"
struct Block{Frozen*f;int col,state;vector<int>ids;vector<uint8_t>policy;vector<int>quality;};
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);int N,K,P,R;if(!(cin>>N>>K>>P>>R))return 0;vector<string>A(P);for(auto&s:A)cin>>s;
 Frozen six({"0....0","10...0",".10.00","111000",".11100","11..10",".....1"},4);
 Frozen four({"1110","...1","0000","1.00","..1.","0100"},2);
 vector<Block>bs;int np=1;vector<string>design(K,string(N,'.'));vector<int>owner(K,-1);
 for(int b=0;b<11;b++){auto*f=b==10?&four:&six;int col=b*6;Block v{f,col,0,{},{},{}};for(int k=0;k<f->g.K;k++){int id=k==f->base?0:np++;v.ids.push_back(id);if(id)owner[id]=b;for(int j=0;j<f->g.bits;j++)design[id][col+j]=f->g.pattern[k][j];}v.quality.resize(f->g.states);for(int s=0;s<f->g.states;s++)for(int t=0;t<f->g.values;t++)v.quality[s]+=f->g.distance[t][s];bs.push_back(v);}
 vector<int>order(K);iota(order.begin(),order.end(),0);
 auto infer=[&](){for(auto&b:bs){array<uint8_t,7>p{};int c=0;for(int id:order){auto it=find(b.ids.begin(),b.ids.end(),id);if(it!=b.ids.end())p[c++]=it-b.ids.begin();}b.state=b.f->g.rank(p);}};infer();
 for(auto&s:design)cout<<s<<'\n';cout.flush();long total=0;int fallback=0;
 auto apply=[&](int id,int d,vector<pair<int,int>>&ops){auto it=find(order.begin(),order.end(),id);order.erase(it);order.insert(order.begin()+d,id);ops.push_back({id,d});};
 auto localmove=[&](int bi,int e,vector<pair<int,int>>&ops){auto&b=bs[bi];auto&g=b.f->g;auto v=g.edges[b.state][e];int id=b.ids[v.paper];auto pp=g.perm[v.next];int local=0;while(pp[local]!=v.paper)local++;int d;if(local+1<g.K){int before=b.ids[pp[local+1]];d=find(order.begin(),order.end(),before)-order.begin();int i=find(order.begin(),order.end(),id)-order.begin();if(i<d)d--;}else{int after=b.ids[pp[local-1]];d=find(order.begin(),order.end(),after)-order.begin();int i=find(order.begin(),order.end(),id)-order.begin();if(i<d)d--;d++;}apply(id,d,ops);b.state=v.next;};
 for(int r=0;r<R;r++){
  int a;cin>>a;if(a<0)return 0;vector<pair<int,int>>ops;vector<int>tg(11);
  for(int b=0;b<11;b++)for(int j=0;j<bs[b].f->g.bits;j++)if(A[a][bs[b].col+j]=='1')tg[b]|=1<<j;
  auto basebest=[&](bool frozen=false){
   vector<array<int,7>>dc(11);vector<int>depth(11);int sum=0;for(int b=0;b<11;b++){auto&bb=bs[b];for(int d=0;d<bb.f->g.K;d++)dc[b][d]=(frozen?bb.f->dd:bb.f->g.distance)[tg[b]][bb.f->shift[bb.state][d]];sum+=dc[b][0];}
   int old=find(order.begin(),order.end(),0)-order.begin(),best=INT_MAX,bd=0;vector<int>removed=order;removed.erase(removed.begin()+old);
   for(int d=0;d<K;d++){int cost=sum+(d!=old);if(cost<best)best=cost,bd=d;if(d+1<K){int b=owner[removed[d]];if(b>=0){sum-=dc[b][depth[b]];depth[b]++;sum+=dc[b][depth[b]];}}}return pair<int,int>{best,bd};
  };
  auto basemove=[&](int d){int old=find(order.begin(),order.end(),0)-order.begin();if(d!=old)apply(0,d,ops);infer();};
  int same=0;for(int step=0;step<70;step++){
   int current=0;bool visible=false;for(int b=0;b<11;b++)current+=bs[b].f->g.distance[tg[b]][bs[b].state],visible|=bool(bs[b].f->g.visible[bs[b].state]>>bs[b].f->base&1);if(!current)break;
   auto [bc,bd]=basebest();if(visible&&bc<current){basemove(bd);same=0;continue;}
   int bestb=-1,beste=-1;double best=1e9;auto savedorder=order;vector<int>savedstate;for(auto&b:bs)savedstate.push_back(b.state);
   for(int b=0;b<11;b++){auto&bb=bs[b];auto&g=bb.f->g;int st=bb.state,d=g.distance[tg[b]][st];if(!d)continue;int be=-1,bq=INT_MAX;
    for(int e=0;e<g.choices;e++){auto v=g.edges[st][e];if(v.paper==bb.f->base||!(g.visible[st]>>v.paper&1)||g.distance[tg[b]][v.next]+1!=d)continue;int q=bb.quality[v.next];if(q<bq)bq=q,be=e;}
    if(be>=0){vector<pair<int,int>>tmp;localmove(b,be,tmp);auto [nc,nd]=basebest();double q=nc+double(bq)/g.values*.01;if(q<best)best=q,bestb=b,beste=be;order=savedorder;for(int x=0;x<11;x++)bs[x].state=savedstate[x];}
   }
   if(bestb>=0){localmove(bestb,beste,ops);same=0;continue;}
   if(visible&&same<2){basemove(bd);same++;continue;}
   break;
  }
  int current=0;bool visible=false;for(int b=0;b<11;b++)current+=bs[b].f->g.distance[tg[b]][bs[b].state],visible|=bool(bs[b].f->g.visible[bs[b].state]>>bs[b].f->base&1);
  if(current){fallback++;if(!visible){auto&b=bs[0];int id=b.ids[3],d=find(order.begin(),order.end(),0)-order.begin(),i=find(order.begin(),order.end(),id)-order.begin();if(i<d)d--;apply(id,d+1,ops);infer();}auto [bc,bd]=basebest(true);basemove(bd);for(int b=0;b<11;b++){auto&bb=bs[b];auto&g=bb.f->g;while(g.display[bb.state]!=tg[b])localmove(b,bb.f->pol[tg[b]*g.states+bb.state],ops);}}
  total+=ops.size();cout<<ops.size()<<'\n';for(auto [k,d]:ops)cout<<k<<' '<<d<<'\n';cout.flush();
 }
 cerr<<"total="<<total<<" fallback="<<fallback<<"\n";
}

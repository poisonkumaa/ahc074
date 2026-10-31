from pathlib import Path
p=Path(__file__).parent
h=(p/'round6_tailplan_all.h').read_text()
h=h.replace('vector<PrefixPaper>papers;vector<Model>models;', 'vector<PrefixPaper>papers;vector<U64>barcode;vector<Model>models;')
h=h.replace('papers(p),mask(K)', 'papers(p),barcode(A),mask(K)')
start=h.index(' Choice choose(');end=h.index(' vector<pair<int,int>>execute(',start)
method=r''' Choice choose(const vector<int>&order,int product,const vector<U64>*future,bool evaluate=true){
  const int INF=1000000000;int dp[65][193],parent[65][193],which[65][193],active[65][193],ac[65]{},pos[192],reward[192],cnt[66][193],catcnt[8][193]{};
  for(int k=0;k<K;k++)reward[k]=16+(future?2*__builtin_popcountll((*future)[k]):0);for(int i=0;i<K;i++)pos[order[i]]=i;
  for(int l=0;l<=core+1;l++){cnt[l][0]=0;for(int i=0;i<K;i++)cnt[l][i+1]=cnt[l][i]+(papers[order[i]].len>=l);}
  for(int cat=0;cat<b;cat++)for(int i=0;i<K;i++)catcnt[cat][i+1]=catcnt[cat][i]+(papers[order[i]].len==core+cat+1);
  for(auto&row:dp)fill(row,row+K+1,INF);dp[0][0]=0;active[0][ac[0]++]=0;U64 target=barcode[product];
  for(int j=0;j<core;j++){sort(active[j],active[j]+ac[j]);for(int z:models[0].dp->candidate[product][j])if(papers[z].len<=core){int len=papers[z].len,q=pos[z];for(int ix=0;ix<ac[j];ix++){int h=active[j][ix],add=0;if(q>=h){int moved=cnt[len][q]-cnt[len][h],upper=cnt[j+1][q]-cnt[j+1][h];add=moved+(upper>moved);}int nh=max(q+1,h),value=dp[j][h]+add*65536-reward[z];if(dp[len][nh]==INF)active[len][ac[len]++]=nh;if(value<dp[len][nh])dp[len][nh]=value,parent[len][nh]=j*(K+1)+h,which[len][nh]=z;}}}
  int tailcost[8][193],tailowner[8][193];
  for(int cat=0;cat<b;cat++)for(int h=0;h<=K;h++){
   int owner=-1,cost=1000000,bestquality=-1;for(int q=0;q<K;q++){int z=order[q];if(papers[z].len!=core+cat+1||((papers[z].color^target)>>(core+cat)&1))continue;int cur=q<h?0:catcnt[cat][q]-catcnt[cat][h];if(cur<cost||(cur==cost&&reward[z]>bestquality))owner=z,cost=cur,bestquality=reward[z];}tailcost[cat][h]=cost;tailowner[cat][h]=owner;
  }
  Choice result;
  for(int mi=0;mi<int(models.size());mi++){
   int first=models[mi].permutation[0],best=INF,bestj=-1,besth=-1,bestz=-1;
   for(int z=0;z<K;z++)if(papers[z].len==core+first+1&&!(((papers[z].color^target)>>(core+first))&1)){
    U64 diff=(papers[z].color^target)&PrefixDP::low(core);int start=diff?64-__builtin_clzll(diff):0,q=pos[z];
    for(int j=start;j<=core;j++)for(int ix=0;ix<ac[j];ix++){int h=active[j][ix],add=0,quality=reward[z];if(q>=h){int moved=catcnt[first][q]-catcnt[first][h],upper=cnt[j+1][q]-cnt[j+1][h];add=moved+(upper>moved);}bool ok=true;for(int cat=0;cat<b;cat++)if(cat!=first){int k=tailowner[cat][h];if(k<0){ok=false;break;}add+=tailcost[cat][h];quality+=reward[k];}if(!ok)continue;int value=dp[j][h]+add*65536-quality;if(value<best)best=value,bestj=j,besth=h,bestz=z;}
   }
   if(bestz<0)continue;Choice c;c.model=mi;c.moves=(best+65535)/65536;c.selected.assign(K,false);c.selected[bestz]=true;for(int cat=0;cat<b;cat++)if(cat!=first)c.selected[tailowner[cat][besth]]=true;
   int j=bestj,h=besth;while(j){int z=which[j][h],previous=parent[j][h];c.selected[z]=true;j=previous/(K+1);h=previous%(K+1);}c.quality=0;for(int k=0;k<K;k++)if(c.selected[k])c.quality+=reward[k];
   if(evaluate){c.after=order;c.ops=execute(c.after,c,future);c.actual=c.ops.size();c.evaluated=true;}else c.actual=c.moves;
   if(c.actual<result.actual||(c.actual==result.actual&&c.quality>result.quality))result=move(c);
  }return result;
 }
'''
h=h[:start]+method+h[end:]
(p/'round6_tailplan_access.h').write_text(h)
s=(p/'round6_tailplan_all.cpp').read_text().replace('#include "round6_tailplan_all.h"','#include "round6_tailplan_access.h"')
(p/'round6_tailplan_access.cpp').write_text(s)

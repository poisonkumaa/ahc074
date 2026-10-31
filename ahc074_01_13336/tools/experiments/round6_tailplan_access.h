#pragma once
#include "round6_general_future.h"
struct Round6TailPlanner {
 struct Model {unique_ptr<PrefixDP>dp;vector<int>length;vector<U64>target;vector<int>permutation;};
 struct Choice {int moves=1000000,quality=-1,model=-1,actual=1000000;vector<char>selected;bool evaluated=false;vector<int>after;vector<pair<int,int>>ops;};
 int core,b,K,P;vector<PrefixPaper>papers;vector<U64>barcode;vector<Model>models;vector<U64>mask;
 Round6TailPlanner(const vector<U64>&A,const vector<PrefixPaper>&p,int tail,int limit):core(64-tail),b(tail),K(p.size()),P(A.size()),papers(p),barcode(A),mask(K){
  for(int k=0;k<K;k++)mask[k]=papers[k].len<=core?PrefixDP::low(papers[k].len):PrefixDP::low(core)|(1ULL<<(papers[k].len-1));
  vector<int>perm(b);iota(perm.begin(),perm.end(),0);
  do {
   vector<int>rank(b);for(int i=0;i<b;i++)rank[perm[i]]=i;
   Model m;m.permutation=perm;auto virtual_papers=p;
   for(int k=0;k<K;k++){int len=p[k].len;if(len>core)len=core+rank[len-core-1]+1;virtual_papers[k].len=len;m.length.push_back(len);}
   m.dp=make_unique<PrefixDP>(64,virtual_papers);m.dp->candidate.resize(P);m.target.resize(P);
   for(int v=0;v<P;v++){
    U64 target=A[v]&PrefixDP::low(core);for(int cat=0;cat<b;cat++)target|=((A[v]>>(core+cat))&1)<<(core+rank[cat]);m.target[v]=target;m.dp->product_index[target]=v;
    for(int k=0;k<K;k++){
     int len=p[k].len;if(len<=core){U64 diff=(A[v]^p[k].color)&PrefixDP::low(len);int start=diff?64-__builtin_clzll(diff):0;for(int j=start;j<len;j++)m.dp->candidate[v][j].push_back(k);}
     else {int cat=len-core-1,r=rank[cat];if(((A[v]^p[k].color)>>(len-1))&1)continue;
      if(r==0){U64 diff=(A[v]^p[k].color)&PrefixDP::low(core);int start=diff?64-__builtin_clzll(diff):0;for(int j=start;j<=core;j++)m.dp->candidate[v][j].push_back(k);}
      else m.dp->candidate[v][core+r].push_back(k);
     }
    }
   }
   models.push_back(move(m));if(limit&&int(models.size())>=limit)break;
  }while(next_permutation(perm.begin(),perm.end()));
 }
 Choice choose(const vector<int>&order,int product,const vector<U64>*future,bool evaluate=true){
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
 vector<pair<int,int>>execute(vector<int>&order,const Choice&c,const vector<U64>*future){
  if(c.evaluated){order=c.after;return c.ops;}vector<pair<int,int>>ops;auto&length=models[c.model].length;int remaining=GlobalPrefix::fixed_distance(order,length,c.selected);
  while(remaining){vector<char>visible(K);U64 covered=0;for(int k:order){visible[k]=(mask[k]&~covered)!=0;covered|=mask[k];}auto[from,to]=GeneralPrefixFuture::best_move(order,length,c.selected,visible,length,.5,future);if(from<0)throw runtime_error("tail actual step missing");int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);ops.push_back({k,to});int next=GlobalPrefix::fixed_distance(order,length,c.selected);if(next>=remaining)throw runtime_error("tail actual no decrease");remaining=next;}
  return ops;
 }
 vector<U64>teacher(){
  vector<int>order(K);iota(order.begin(),order.end(),0);vector<U64>future(K);mt19937 rng(912367);vector<int>targets(P);iota(targets.begin(),targets.end(),0);shuffle(targets.begin(),targets.end(),rng);
  for(int r=0;r<192;r++){int t=r<128?rng()%P:targets[r-128];auto c=choose(order,t,nullptr);if(c.model<0)throw runtime_error("tail teacher infeasible");if(r>=128)for(int k=0;k<K;k++)if(c.selected[k])future[k]|=1ULL<<(r-128);execute(order,c,nullptr);}
  return future;
 }
 bool check_display(const vector<int>&order,U64 target){U64 covered=0,color=0;for(int k:order){U64 visible=mask[k]&~covered;color|=papers[k].color&visible;covered|=mask[k];}return color==target;}
};
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);int b=argc>1?atoi(argv[1]):2,repetitions=argc>2?atoi(argv[2]):8,limit=argc>3?atoi(argv[3]):0;
 int n,K,P,R;if(!(cin>>n>>K>>P>>R)||n!=64||K!=192||P!=256)return 0;
 vector<U64>original(P);for(int v=0;v<P;v++){string x;cin>>x;for(int j=0;j<n;j++)if(x[j]=='1')original[v]|=1ULL<<j;}
 structure2_training_origin=clock();multi_training_budget=AHC_LEARNING_BUDGET;
 auto model=train_multi_model(original,12345);auto A=move(model.A);auto cols=move(model.cols);auto flip=move(model.flip);auto papers=move(model.papers);
 if(repetitions)exact_state_refit(A,papers,1,43,32,repetitions,0);
 Round6TailPlanner planner(A,papers,b,limit);vector<int>order(papers.size());iota(order.begin(),order.end(),0);
 for(int v=0;v<P;v++){auto c=planner.choose(order,v,nullptr,false);if(c.model<0)throw runtime_error("tail product infeasible");}
 auto future=planner.teacher();
 cerr<<"Tail orientations="<<planner.models.size()<<" PrepCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
 for(int k=0;k<K;k++){string s(n,'.');if(k<int(papers.size()))for(int j=0;j<n;j++)if(planner.mask[k]>>j&1)s[cols[j]]=char('0'+(((papers[k].color>>j)&1)^flip[j]));cout<<s<<'\n';}cout.flush();
 for(int r=0;r<R;r++){
  int product;if(!(cin>>product)||product<0||product>=P)return 0;auto c=planner.choose(order,product,&future);if(c.model<0)throw runtime_error("tail online infeasible");auto&length=planner.models[c.model].length;
  auto old=order;auto operations=planner.execute(order,c,&future);
  auto verify=old;for(auto[k,d]:operations){U64 covered=0;bool visible=false;for(int x:verify){if(x==k){visible=(planner.mask[x]&~covered)!=0;break;}covered|=planner.mask[x];}if(!visible)throw runtime_error("tail invisible move");auto it=find(verify.begin(),verify.end(),k);verify.erase(it);verify.insert(verify.begin()+d,k);}
  if(verify!=order||!planner.check_display(order,A[product]))throw runtime_error("tail incorrect display");
  U64 bit=1ULL<<(r%64);for(int k=0;k<int(papers.size());k++)future[k]=(future[k]&~bit)|(c.selected[k]?bit:0ULL);
  cout<<operations.size()<<'\n';for(auto[k,d]:operations)cout<<k<<' '<<d<<'\n';cout.flush();
 }
 cerr<<"FinishCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
}

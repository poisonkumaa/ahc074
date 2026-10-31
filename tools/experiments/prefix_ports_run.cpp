#pragma GCC optimize("O3")
#include "templates_agent_trim_dp.h"
#include "global_agent_prefix.h"
#include "prefix_allocate_train.h"
#include "templates_agent_trim_move.h"
U64 visible_output(const vector<int>&p,const vector<PrefixPaper>&a){int high=0;U64 r=0;for(int k:p){int l=a[k].len;if(l>high){r|=a[k].color&(PrefixDP::low(l)^PrefixDP::low(high));high=l;}}return r;}
vector<U64> trie_patterns(const vector<U64>&A,int l,int cap,int amount){
 struct Node{int child[2]{-1,-1},count=0,sample=0;array<int,5>dp,split;Node(){dp.fill(-1000000);dp[0]=0;split.fill(0);}};
 vector<Node>trie(1);int depth=min(l,cap);
 for(int v=0;v<(int)A.size();v++){int x=0;trie[x].count++;trie[x].sample=v;for(int d=0;d<depth;d++){int b=A[v]>>(l-1-d)&1;if(trie[x].child[b]<0){int y=trie.size();trie[x].child[b]=y;trie.emplace_back();}x=trie[x].child[b];trie[x].count++;trie[x].sample=v;}}
 for(int x=(int)trie.size()-1;x>=0;x--){auto&v=trie[x];int a=v.child[0],b=v.child[1];if(a<0&&b<0){v.dp[1]=v.count;continue;}
  for(int k=1;k<=amount;k++)for(int q=0;q<=k;q++){int r=k-q;if((a<0&&q)||(b<0&&r))continue;int z=(a<0?0:trie[a].dp[q])+(b<0?0:trie[b].dp[r])+v.count;if(z>v.dp[k])v.dp[k]=z,v.split[k]=q;}
 }
 int use=amount;while(use>1&&trie[0].dp[use]<0)use--;
 vector<U64>ans;function<void(int,int)>rec=[&](int x,int k){if(!k)return;auto&v=trie[x];if(v.child[0]<0&&v.child[1]<0){ans.push_back(A[v.sample]&PrefixDP::low(l));return;}int q=v.split[k];if(q)rec(v.child[0],q);if(k-q)rec(v.child[1],k-q);};rec(0,use);return ans;
}
int fixed_dist(const vector<int>&p,const vector<PrefixPaper>&a,const vector<int>&chosen){
 int k=p.size(),n=64;bool want[192]{},moved[192]{};int ceil[65];fill(ceil,ceil+65,-1);for(int z:chosen)want[z]=true;
 int prev=0;for(int z:chosen){for(int l=prev+1;l<=a[z].len;l++)ceil[l]=z;prev=a[z].len;}
 int lo=65,score=0,pos[192];for(int i=k-1;i>=0;i--){int x=p[i];pos[x]=i;moved[x]=a[x].len>=lo;score+=moved[x];if(want[x])lo=min(lo,a[x].len);}
 bool bad[192]{};for(int x=0;x<k;x++)if(!want[x]&&!moved[x]&&ceil[a[x].len]>=0){int z=ceil[a[x].len];if(pos[x]<pos[z])bad[z]=1;}
 for(int z:chosen)score+=!moved[z]&&bad[z];return score;
}
pair<int,int> slowmove(const vector<int>&p,const vector<PrefixPaper>&a,const vector<int>&chosen,int cur){
 int k=p.size(),high=0;pair<int,int>best={-1,-1};double bq=1e9;
 for(int i=0;i<k;i++){int x=p[i],len=a[x].len;if(len<=high)continue;high=len;
  for(int d=0;d<k;d++)if(i!=d){auto q=p;q.erase(q.begin()+i);q.insert(q.begin()+d,x);if(fixed_dist(q,a,chosen)!=cur-1)continue;
   double score=0;int mx=0,vis=0;for(int z:q){if(a[z].len>mx)vis++,mx=a[z].len;}
   score=-vis;if(score<bq)best={i,d},bq=score;
  }
 }
 return best;
}
int main(int argc,char**argv){
 int n,K,P,R;cin>>n>>K>>P>>R;vector<U64>A(P);for(int i=0;i<P;i++){string s;cin>>s;for(int j=0;j<n;j++)if(s[j]=='1')A[i]|=1ULL<<j;}
 int rounds=argc>1?atoi(argv[1]):R,mode=argc>2?atoi(argv[2]):0,mult=argc>3?atoi(argv[3]):3;mt19937_64 rng(12345);
 int cap=argc>6?atoi(argv[6]):64;double decay=argc>7?atof(argv[7]):.5;
 if(argc>5&&atoi(argv[5])){
  vector<int>cols(n),fl(n);iota(cols.begin(),cols.end(),0);int cd[64][64]{};for(int x=0;x<n;x++)for(int y=0;y<n;y++)for(U64 z:A)cd[x][y]+=((z>>x)^(z>>y))&1;
  bool absolute=atoi(argv[5])>=2;
  auto cost=[&](){int s=0;for(int i=1;i<n;i++){int d=cd[cols[i-1]][cols[i]];s+=absolute?min(d,P-d):(fl[i-1]^fl[i])?P-d:d;}return s;};
  int value=cost(),best=value;auto bc=cols,bf=fl;
  for(int it=0;it<300000;it++){int x=rng()%n,y=rng()%n;if(x>y)swap(x,y);bool inv=rng()%3==0&&!absolute;if(inv)fl[x]^=1;else reverse(cols.begin()+x,cols.begin()+y+1),reverse(fl.begin()+x,fl.begin()+y+1);int nv=cost();double temp=8*pow(.002,double(it)/300000);if(nv<=value||double(rng()>>11)*0x1p-53<exp((value-nv)/temp)){value=nv;if(value<best)best=value,bc=cols,bf=fl;}else if(inv)fl[x]^=1;else reverse(cols.begin()+x,cols.begin()+y+1),reverse(fl.begin()+x,fl.begin()+y+1);}
  if(absolute){bf[0]=0;for(int j=1;j<n;j++)bf[j]=bf[j-1]^(cd[bc[j-1]][bc[j]]>P/2);}
  if(atoi(argv[5])>=3){
   U64 masks[64][4]{};for(int i=0;i<n;i++)for(int v=0;v<P;v++)if(A[v]>>i&1)masks[i][v/64]|=1ULL<<(v%64);
   static short gain[64][64][64];
   for(int i=0;i<n;i++)for(int j=0;j<n;j++)if(i!=j){U64 mm[4][4];for(int t=0;t<4;t++)for(int w=0;w<4;w++)mm[t][w]=(t&2?masks[i][w]:~masks[i][w])&(t&1?masks[j][w]:~masks[j][w]);
    for(int k=0;k<n;k++)if(k!=i&&k!=j){int hist[8]{};for(int t=0;t<4;t++)for(int w=0;w<4;w++){hist[t*2]+=__builtin_popcountll(mm[t][w]&~masks[k][w]);hist[t*2+1]+=__builtin_popcountll(mm[t][w]&masks[k][w]);}
     int c[4],one[4],two[4];for(int t=0;t<4;t++){c[t]=hist[2*t]+hist[2*t+1];one[t]=c[t]+max(hist[2*t],hist[2*t+1]);two[t]=2*c[t];}
     int bo[2],bt[2];for(int t=0;t<2;t++){int u=2*t,v=u+1,s=c[u]+c[v];bo[t]=s+max(one[u],one[v]);bt[t]=s+max({two[u],two[v],one[u]+one[v]});}
     gain[k][j][i]=max(bo[0]+bt[1],bt[0]+bo[1]);
    }
   }
   cols=bc;auto sc=[&](){int q=0;for(int j=2;j<n;j++)q+=gain[cols[j-2]][cols[j-1]][cols[j]];return q;};int value=sc(),bv=value;
   for(int it=0;it<300000;it++){int x=rng()%n,y=rng()%n;if(x>y)swap(x,y);reverse(cols.begin()+x,cols.begin()+y+1);int nv=sc();double temp=12*pow(.002,double(it)/300000);if(nv>=value||double(rng()>>11)*0x1p-53<exp((nv-value)/temp)){value=nv;if(value>bv)bv=value,bc=cols;}else reverse(cols.begin()+x,cols.begin()+y+1);}
   bf[0]=0;for(int j=1;j<n;j++)bf[j]=bf[j-1]^(cd[bc[j-1]][bc[j]]>P/2);cerr<<"triple="<<bv<<"\n";
  }
  cerr<<"assignment="<<best<<"\n";
  for(U64&v:A){U64 old=v;v=0;for(int j=0;j<n;j++)v|=(((old>>bc[j])&1)^bf[j])<<j;}
 }
 vector<PrefixPaper>a;
 for(int l=1;l<=n;l++){
  if(mode==0){U64 c=rng();for(int q=0;q<mult;q++){U64 d=q==0?c:q==1?c^(1ULL<<(l-1)):rng();a.push_back({l,d&PrefixDP::low(l)});}}
  if(mode==1){for(int q=0;q<mult;q++)a.push_back({l,q==0?0:q==1?PrefixDP::low(l):rng()&PrefixDP::low(l)});}
  if(mode==2){
   vector<U64>pat;for(int q=0;q<mult;q++)pat.push_back(A[rng()%P]&PrefixDP::low(l));
   auto score=[&](const vector<U64>&pat){int s=0;for(U64 v:A){int best=0;for(U64 c:pat){U64 d=(v^c)&PrefixDP::low(l);int common=d?l-64+__builtin_clzll(d):l;best=max(best,min(common,cap));}s+=best;}return s;};
   for(int iter=0;iter<4;iter++)for(int q=0;q<mult;q++){int bs=-1;U64 bc=0;for(U64 c:A){pat[q]=c&PrefixDP::low(l);int s=score(pat);if(s>bs)bs=s,bc=pat[q];}pat[q]=bc;}
   bool b0=0,b1=0;for(U64 c:pat)if(c>>(l-1)&1)b1=1;else b0=1;if(!b0||!b1)pat[0]^=1ULL<<(l-1);
   for(U64 c:pat)a.push_back({l,c});
  }
  if(mode==3){auto pat=trie_patterns(A,l,16,mult);for(U64 c:pat)a.push_back({l,c});}
 }
 if(mode==4){
  vector<vector<vector<U64>>>plans(n+1,vector<vector<U64>>(5));int quality[65][5]{};
  for(int l=1;l<=n;l++)for(int q=2;q<=4;q++){
   plans[l][q]=trie_patterns(A,l,cap,q);
   for(U64 v:A){int best=0;for(U64 c:plans[l][q]){U64 d=(v^c)&PrefixDP::low(l);int common=d?l-64+__builtin_clzll(d):l;best=max(best,min(common,cap));}quality[l][q]+=best;}
  }
  int dp[65][193],prev[65][193];fill(&dp[0][0],&dp[0][0]+65*193,-1000000);dp[0][0]=0;
  for(int l=1;l<=n;l++)for(int used=0;used<=K;used++)if(dp[l-1][used]>=0)for(int q=2;q<=4;q++){
   int nu=used+plans[l][q].size();if(nu>K)continue;int value=dp[l-1][used]+quality[l][q];if(value>dp[l][nu])dp[l][nu]=value,prev[l][nu]=q;
  }
  int used=max_element(dp[n],dp[n]+K+1)-dp[n];vector<int>take(n+1);for(int l=n;l>=1;l--){int q=prev[l][used];take[l]=q;used-=plans[l][q].size();}
  for(int l=1;l<=n;l++)for(U64 c:plans[l][take[l]])a.push_back({l,c});
 }
 if(argc>4&&atoi(argv[4])){
  int aim=atoi(argv[4]);vector<vector<int>>start(P,vector<int>(a.size()));for(int v=0;v<P;v++)for(int z=0;z<(int)a.size();z++){U64 d=(a[z].color^A[v])&PrefixDP::low(a[z].len);start[v][z]=d?64-__builtin_clzll(d):0;}
  vector<char>deleted(a.size());vector<int>order(a.size());iota(order.begin(),order.end(),0);shuffle(order.begin(),order.end(),rng);int left=a.size();
  for(int z:order){if(left<=aim)break;deleted[z]=1;bool ok=true;
   for(int v=0;v<P&&ok;v++){bool can[65]{};can[0]=1;int latest=0;for(int q=0;q<(int)a.size();q++)if(!deleted[q]){int l=a[q].len;for(int j=start[v][q];j<l;j++)if(can[j]){can[l]=true;break;}}ok=can[n];}
   if(ok)left--;else deleted[z]=0;
  }
  vector<PrefixPaper>b;for(int z=0;z<(int)a.size();z++)if(!deleted[z])b.push_back(a[z]);a=b;cerr<<"kept="<<a.size()<<"\n";
 }
 train_prefix_dictionary(A,a,n,64,1,8,0,6,0);
 int trainsteps=argc>8?atoi(argv[8]):0,trainq=argc>9?atoi(argv[9]):32,regular=argc>10?atoi(argv[10]):0;
 if(trainsteps){
  PrefixDP model(n,a);vector<int>p(a.size()),len;for(auto x:a)len.push_back(x.len);iota(p.begin(),p.end(),0);
  vector<vector<int>>states;vector<int>target;
  for(int t=0;t<trainq+32;t++){
   int v=rng()%P,value;auto chosen=model.solve(p,A[v],value);vector<char>selected(a.size());for(int z:chosen)selected[z]=true;
   for(int i=0;i<value;i++){auto [from,to]=GlobalPrefix::best_move(p,len,selected,decay);int x=p[from];p.erase(p.begin()+from);p.insert(p.begin()+to,x);}
   if(t>=32){states.push_back(p);target.push_back(trainq==P?t-32:rng()%P);}
  }
  auto minseg=[&](int v){int f[65];fill(f,f+65,1000);f[0]=0;for(auto p:a){U64 d=(A[v]^p.color)&PrefixDP::low(p.len);int st=d?64-__builtin_clzll(d):0;for(int j=st;j<p.len;j++)f[p.len]=min(f[p.len],f[j]+1);}return f[n];};
  vector<int>costs(trainq);int value=0;for(int t=0;t<trainq;t++){model.solve(states[t],A[target[t]],costs[t]);if(regular)costs[t]+=regular*minseg(target[t]);value+=costs[t];}
  int best=value;auto bestpattern=a;int accepted=0;
  for(int it=0;it<trainsteps;it++){
   int z=rng()%a.size(),l=a[z].len,v=rng()%P;U64 old=a[z].color;
   U64 pat=A[v]&PrefixDP::low(l);int keep=min(l,2);U64 preserved=PrefixDP::low(l)^PrefixDP::low(l-keep);pat=(pat&~preserved)|(old&preserved);
   if(pat==old)continue;a[z].color=pat;model.a[z].color=pat;int score=value;vector<pair<int,int>>newcosts;
   for(int t=0;t<trainq;t++){
    U64 od=(old^A[target[t]])&PrefixDP::low(l),nd=(pat^A[target[t]])&PrefixDP::low(l);
    int os=od?64-__builtin_clzll(od):0,ns=nd?64-__builtin_clzll(nd):0;
    if(os==ns)continue;int cost;model.solve(states[t],A[target[t]],cost);if(regular)cost+=regular*minseg(target[t]);score+=cost-costs[t];newcosts.emplace_back(t,cost);
   }
   double temp=.6*pow(.03,double(it)/trainsteps);
   if(score<=value||double(rng()>>11)*0x1p-53<exp(double(value-score)/temp)){value=score;for(auto[t,c]:newcosts)costs[t]=c;accepted++;if(value<best){best=value;bestpattern=a;}}
   else a[z].color=old,model.a[z].color=old;
  }
  a=bestpattern;cerr<<"pattern training "<<best<<" / "<<trainq<<" accepted "<<accepted<<"\n";
 }
 PrefixDP dp(n,a);dp.maxspan=argc>11?atoi(argv[11]):12;int ports=argc>12?atoi(argv[12]):0,portscheme=argc>13?atoi(argv[13]):0;
 for(int q=0;q<ports;q++){int j=portscheme?n-1-(q+1)*(n-1)/(ports+2):(q+1)*(n-1)/(ports+1);if(portscheme>=2)j=n-2-(portscheme-1)*q;if(argc>14)j=atoi(argv[14])+portscheme*q;dp.portmask|=1ULL<<j;}
 if(dp.maxspan<0){
  dp.maxspan=64;for(int k=0;k<int(a.size());k++)dp.cutoff[k]=a[k].len-1;
  for(U64 target:A){int d[65],pr[65],paper[65];fill(d,d+65,1000);d[0]=0;
   for(int j=0;j<n;j++)for(int k=0;k<int(a.size());k++){int l=a[k].len;if(l>j&&!((a[k].color^target)&(PrefixDP::low(l)^PrefixDP::low(j)))&&d[j]+1<d[l])d[l]=d[j]+1,pr[l]=j,paper[l]=k;}
   for(int l=n;l;){int k=paper[l],j=pr[l];dp.cutoff[k]=min(dp.cutoff[k],j);l=j;}
  }
 }
 vector<U64>mask;int totalmask=0;for(int k=0;k<int(a.size());k++){auto x=a[k];U64 m=(PrefixDP::low(x.len)^PrefixDP::low(max(dp.cutoff[k],max(0,x.len-dp.maxspan))))&~dp.portmask;if(dp.portmask>>(x.len-1)&1)m|=1ULL<<(x.len-1);mask.push_back(m);totalmask+=__builtin_popcountll(mask.back());}cerr<<"opacity="<<totalmask<<"\n";
 vector<int>p(a.size()),length;for(auto x:a)length.push_back(x.len);iota(p.begin(),p.end(),0);int total=0;
 for(int r=0;r<rounds;r++){
  int v;cin>>v;int value;auto chosen=dp.solve(p,A[v],value);int ex=fixed_dist(p,a,chosen);if(value!=ex){cerr<<"DP mismatch "<<value<<" "<<ex<<"\n";return 1;}
  vector<char>selected(a.size());for(int z:chosen)selected[z]=true;
  int used=0;while(fixed_dist(p,a,chosen)){
   vector<char>visible(a.size());U64 covered=0;for(int k:p){visible[k]=bool(mask[k]&~covered);covered|=mask[k];}
   auto [from,to]=TrimPrefix::best_move(p,length,selected,visible,decay);if(from<0){cerr<<"STALLED "<<r<<" "<<used<<"\n";return 1;}int x=p[from];p.erase(p.begin()+from);p.insert(p.begin()+to,x);used++;
  }
  for(int j=0;j<n;j++)if(dp.portmask>>j&1){
   for(;;){int from=-1,to=-1;for(int i=0;i<int(p.size());i++)if(a[p[i]].len==j+1){if(from<0)from=i;if(!((a[p[i]].color^A[v])>>j&1)){to=i;break;}}if(to==from)break;if(to<0){cerr<<"PORT IMPOSSIBLE\n";return 1;}int k=p[from];p.erase(p.begin()+from);p.insert(p.begin()+to,k);used++;}
  }
  total+=used;U64 display=0,covered=0;for(int k:p){display|=a[k].color&mask[k]&~covered;covered|=mask[k];}if(display!=A[v]){cerr<<"DISPLAY WRONG "<<r<<"\n";return 1;}
  if(rounds<128&&r%16==0)cerr<<r<<" avg="<<double(total)/(r+1)<<" last="<<value<<" records="<<chosen.size()<<"\n";
 }
 cerr<<"TOTAL "<<total<<" avg="<<double(total)/rounds<<" CPU "<<double(clock())/CLOCKS_PER_SEC<<"\n";
}

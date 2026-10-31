#include "prefix_dp.h"
#include "global_agent_prefix.h"
U64 visible_output(const vector<int>&p,const vector<PrefixPaper>&a){int high=0;U64 r=0;for(int k:p){int l=a[k].len;if(l>high){r|=a[k].color&(PrefixDP::low(l)^PrefixDP::low(high));high=l;}}return r;}
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
 if(argc>5&&atoi(argv[5])){
  vector<int>cols(n),fl(n);iota(cols.begin(),cols.end(),0);int cd[64][64]{};for(int x=0;x<n;x++)for(int y=0;y<n;y++)for(U64 z:A)cd[x][y]+=((z>>x)^(z>>y))&1;
  auto cost=[&](){int s=0;for(int i=1;i<n;i++){int d=cd[cols[i-1]][cols[i]];s+=(fl[i-1]^fl[i])?P-d:d;}return s;};
  int value=cost(),best=value;auto bc=cols,bf=fl;
  for(int it=0;it<300000;it++){int x=rng()%n,y=rng()%n;if(x>y)swap(x,y);bool inv=rng()%3==0;if(inv)fl[x]^=1;else reverse(cols.begin()+x,cols.begin()+y+1),reverse(fl.begin()+x,fl.begin()+y+1);int nv=cost();double temp=8*pow(.002,double(it)/300000);if(nv<=value||double(rng()>>11)*0x1p-53<exp((value-nv)/temp)){value=nv;if(value<best)best=value,bc=cols,bf=fl;}else if(inv)fl[x]^=1;else reverse(cols.begin()+x,cols.begin()+y+1),reverse(fl.begin()+x,fl.begin()+y+1);}
  for(U64&v:A){U64 old=v;v=0;for(int j=0;j<n;j++)v|=(((old>>bc[j])&1)^bf[j])<<j;}
 }
 vector<PrefixPaper>a;
 for(int l=1;l<=n;l++){
  if(mode==0){U64 c=rng();for(int q=0;q<mult;q++){U64 d=q==0?c:q==1?c^(1ULL<<(l-1)):rng();a.push_back({l,d&PrefixDP::low(l)});}}
  if(mode==1){for(int q=0;q<mult;q++)a.push_back({l,q==0?0:q==1?PrefixDP::low(l):rng()&PrefixDP::low(l)});}
  if(mode==2){
   vector<U64>pat;for(int q=0;q<mult;q++)pat.push_back(A[rng()%P]&PrefixDP::low(l));
   auto score=[&](const vector<U64>&pat){int s=0;for(U64 v:A){int best=0;for(U64 c:pat){U64 d=(v^c)&PrefixDP::low(l);int common=d?l-64+__builtin_clzll(d):l;best=max(best,common);}s+=best;}return s;};
   for(int iter=0;iter<4;iter++)for(int q=0;q<mult;q++){int bs=-1;U64 bc=0;for(U64 c:A){pat[q]=c&PrefixDP::low(l);int s=score(pat);if(s>bs)bs=s,bc=pat[q];}pat[q]=bc;}
   bool b0=0,b1=0;for(U64 c:pat)if(c>>(l-1)&1)b1=1;else b0=1;if(!b0||!b1)pat[0]^=1ULL<<(l-1);
   for(U64 c:pat)a.push_back({l,c});
  }
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
 cerr<<"DESIGNCPU "<<double(clock())/CLOCKS_PER_SEC<<"\n";double timeDP=0,timeMoves=0;
 PrefixDP dp(n,a);vector<int>p(a.size()),length;for(auto x:a)length.push_back(x.len);iota(p.begin(),p.end(),0);int total=0;
 for(int r=0;r<rounds;r++){
  int v;cin>>v;int value;clock_t beforeDP=clock();auto chosen=dp.solve(p,A[v],value);timeDP+=double(clock()-beforeDP)/CLOCKS_PER_SEC;int ex=fixed_dist(p,a,chosen);if(value!=ex){cerr<<"DP mismatch "<<value<<" "<<ex<<"\n";return 1;}
  vector<char>selected(a.size());for(int z:chosen)selected[z]=true;
  clock_t beforeMoves=clock();total+=value;for(int i=0;i<value;i++){
   auto [from,to]=GlobalPrefix::best_move(p,length,selected,argc>6?atof(argv[6]):.5);if(from<0){cerr<<"STALLED "<<r<<" "<<i<<"\n";return 1;}int x=p[from];p.erase(p.begin()+from);p.insert(p.begin()+to,x);
  }
  timeMoves+=double(clock()-beforeMoves)/CLOCKS_PER_SEC;if(visible_output(p,a)!=A[v]){cerr<<"DISPLAY WRONG "<<r<<"\n";return 1;}
  if(r%16==0)cerr<<r<<" avg="<<double(total)/(r+1)<<" last="<<value<<" records="<<chosen.size()<<"\n";
 }
 cerr<<"TOTAL "<<total<<" avg="<<double(total)/rounds<<" CPU "<<double(clock())/CLOCKS_PER_SEC<<" DP="<<timeDP<<" moves="<<timeMoves<<"\n";
}

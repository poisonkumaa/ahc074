#define main old_backbone_main
#include "structure2_agent_backbone.cpp"
#undef main
#include "structure2_agent_general_move.h"
#include "templates_agent_trim_move.h"

int main(int argc,char**argv){
 int n,K,P,R;cin>>n>>K>>P>>R;vector<U64>A(P);for(auto &a:A){string s;cin>>s;for(int j=0;j<n;j++)if(s[j]=='1')a|=1ULL<<j;}
 mt19937_64 rng(12345);vector<int>cols(n),flip(n);iota(cols.begin(),cols.end(),0);
 int correlation[64][64]{};for(int x=0;x<n;x++)for(int y=0;y<n;y++)for(U64 v:A)correlation[x][y]+=((v>>x)^(v>>y))&1;
 auto cost=[&](){int s=0;for(int j=1;j<n;j++){int d=correlation[cols[j-1]][cols[j]];s+=min(d,P-d);}return s;};
 int current=cost(),best=current;auto bestcols=cols;
 for(int it=0;it<300000;it++){
  int x=rng()%n,y=rng()%n;if(x>y)swap(x,y);(void)(rng()%3);
  reverse(cols.begin()+x,cols.begin()+y+1);int next=cost();double temperature=8*pow(.002,double(it)/300000);
  if(next<=current||double(rng()>>11)*0x1p-53<exp((current-next)/temperature)){current=next;if(current<best)best=current,bestcols=cols;}
  else reverse(cols.begin()+x,cols.begin()+y+1);
 }
 cols=bestcols;for(int j=1;j<n;j++)flip[j]=flip[j-1]^(correlation[cols[j-1]][cols[j]]>P/2);
 for(U64&v:A){U64 old=v;v=0;for(int j=0;j<n;j++)v|=(((old>>cols[j])&1)^flip[j])<<j;}
 vector<PrefixPaper>papers;train_backbone_dictionary(A,papers,n,6,1,8);refit_all_prefix_papers(A,papers,1,8);
 K=papers.size();vector<U64>mask(K);vector<int>length(K),order(K);iota(order.begin(),order.end(),0);int opaque=0;
 for(int k=0;k<K;k++)mask[k]=PrefixDP::low(length[k]=papers[k].len),opaque+=length[k];
 vector<vector<int>>goals(P);vector<vector<char>>selected(P,vector<char>(K));
 for(int v=0;v<P;v++){
  int dp[65],pk[65],prev[65];fill(dp,dp+65,1000);dp[0]=0;
  for(int j=0;j<n;j++)for(int k=0;k<K;k++)if(papers[k].len>j&&!((papers[k].color^A[v])&(mask[k]^PrefixDP::low(j)))){
   int l=length[k];if(dp[j]+1<dp[l])dp[l]=dp[j]+1,prev[l]=j,pk[l]=k;
  }
  if(dp[n]>=1000){cerr<<"UNREACHABLE\n";return 1;}
  for(int j=n;j;j=prev[j])goals[v].push_back(pk[j]);reverse(goals[v].begin(),goals[v].end());for(int k:goals[v])selected[v][k]=1;
 }
 vector<pair<int,int>>cells;for(int k=0;k<K;k++)for(int j=0;j<length[k]-1;j++)cells.emplace_back(k,j);
 int mode=argc>1?atoi(argv[1]):1;
 if(mode==2)shuffle(cells.begin(),cells.end(),rng);
 if(mode==3)reverse(cells.begin(),cells.end());
 int removed=0;
 if(mode)for(auto[k,j]:cells){
  bool valid=true;
  for(int v=0;v<P&&valid;v++)if(selected[v][k]){
   int owner=-1,next=-1;for(int g:goals[v])if(mask[g]>>j&1){if(owner<0)owner=g;else {next=g;break;}}
   if(owner==k&&(next<0||((papers[next].color^A[v])>>j&1)))valid=false;
  }
  if(valid)mask[k]^=1ULL<<j,removed++;
 }
 cerr<<"OPAQUE "<<opaque<<" -> "<<opaque-removed<<"\n";
 vector<vector<int>>levels(P,vector<int>(K));
 double intrinsic=0;for(int v=0;v<P;v++){
  U64 covered=0,display=0;int rank=0;for(int g:goals[v]){U64 own=mask[g]&~covered;covered|=mask[g];display|=papers[g].color&own;rank++;for(int k=0;k<K;k++)if(mask[k]&own)levels[v][k]=2*rank+((mask[k]>>(length[g]-1))&1);}
  if(covered!=~0ULL||display!=A[v]){cerr<<"DESIGN BUG "<<v<<"\n";return 1;}intrinsic+=double(rank)/P;
 }
 long long total=0;int maxcount=0;int rounds=argc>2?min(R,atoi(argv[2])):R;
 for(int r=0;r<rounds;r++){
  int a;cin>>a;int used=0;
  while(true){
   U64 covered=0,display=0;vector<char>visible(K);for(int k:order){visible[k]=bool(mask[k]&~covered);display|=papers[k].color&mask[k]&~covered;covered|=mask[k];}if(display==A[a])break;
   auto[from,to]=GeneralPrefix::best_move(order,levels[a],selected[a],visible,length);
   if(from<0||used>=384){cerr<<"STUCK "<<r<<" "<<used<<"\n";return 1;}int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);used++;
  }
  total+=used;maxcount=max(maxcount,used);if(r%128==0)cerr<<r<<" avg="<<double(total)/(r+1)<<"\n";
 }
 cerr<<"TOTAL "<<total<<" avg="<<double(total)/rounds<<" intrinsic="<<intrinsic<<" max="<<maxcount<<" CPU="<<double(clock())/CLOCKS_PER_SEC<<"\n";
}

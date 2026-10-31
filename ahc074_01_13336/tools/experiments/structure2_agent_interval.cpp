#define main old_backbone_main
#include "structure2_agent_backbone.cpp"
#undef main
#include "templates_agent_trim_move.h"

struct IntervalDP:PrefixDP{
 vector<int>lower;
 IntervalDP(int n,vector<PrefixPaper>a,vector<int>lo):PrefixDP(n,a),lower(lo){}
 vector<int> solve(const vector<int>&p,U64 target,int &best){
  int pos[192];for(int i=0;i<k;i++)pos[p[i]]=i;
  for(int l=0;l<=n+1;l++){cnt[l][0]=0;for(int i=0;i<k;i++)cnt[l][i+1]=cnt[l][i]+(a[p[i]].len>=l);}
  vector<array<int,193>>dp(n+1),parent(n+1),which(n+1);for(auto&r:dp)r.fill(1000000);dp[0][0]=0;
  for(int j=0;j<n;j++){
   vector<int>active;for(int h=0;h<=k;h++)if(dp[j][h]<1000000)active.push_back(h);
   for(int z=0;z<k;z++){int l=a[z].len,q=pos[z];if(l<=j||lower[z]>j||((a[z].color^target)&(low(l)^low(j))))continue;
    for(int h:active){int add=0;if(q>=h){int moved=cnt[l][q]-cnt[l][h],upper=cnt[j+1][q]-cnt[j+1][h];add=moved+(upper>moved);}
     int nh=max(q+1,h),value=dp[j][h]+add*256-1;if(value<dp[l][nh])dp[l][nh]=value,parent[l][nh]=j*(k+1)+h,which[l][nh]=z;
    }
   }
  }
  int h=min_element(dp[n].begin(),dp[n].begin()+k+1)-dp[n].begin(),j=n;best=(dp[n][h]+255)/256;vector<int>ans;if(best>=1000)abort();
  while(j){int z=which[j][h],prev=parent[j][h];ans.push_back(z);j=prev/(k+1);h=prev%(k+1);}reverse(ans.begin(),ans.end());return ans;
 }
};

void learn_lower(const vector<U64>&A,const vector<PrefixPaper>&papers,vector<int>&lower,double penalty,int passes){
 const int P=A.size(),K=papers.size(),INF=1000;vector<array<int,65>>F(P),B(P);vector<vector<int>>start(P,vector<int>(K)),at(65);
 for(int k=0;k<K;k++){at[papers[k].len].push_back(k);for(int v=0;v<P;v++){U64 d=(A[v]^papers[k].color)&PrefixDP::low(papers[k].len);start[v][k]=d?64-__builtin_clzll(d):0;}}
 auto calc=[&](){int total=0;for(int v=0;v<P;v++){auto&f=F[v];auto&b=B[v];f.fill(INF);b.fill(INF);f[0]=0;b[64]=0;
  for(int l=1;l<=64;l++)for(int k:at[l])for(int j=max(lower[k],start[v][k]);j<l;j++)f[l]=min(f[l],f[j]+1);
  for(int l=64;l>0;l--)for(int k:at[l])for(int j=max(lower[k],start[v][k]);j<l;j++)b[j]=min(b[j],b[l]+1);total+=f[64];}return total;};
 int initial=calc(),current=initial;
 for(int pass=0;pass<passes;pass++)for(int k=K-1;k>=0;k--){
  calc();int l=papers[k].len;vector<int>cost(l);vector<char>valid(l,true);
  for(int v=0;v<P;v++){
   int pm[65];pm[l]=INF;for(int j=l-1;j>=0;j--)pm[j]=min(pm[j+1],F[v][j]);int without=INF;
   for(int u=l;u<=64;u++)for(int x:at[u])if(x!=k){int st=max(lower[x],start[v][x]);if(st<l)without=min(without,pm[st]+1+B[v][u]);}
   for(int lo=0;lo<l;lo++){int route=min(without,pm[max(lo,start[v][k])]+1+B[v][l]);cost[lo]+=route;if(route>=INF)valid[lo]=false;}
  }
  int choice=lower[k];double best=cost[choice]-penalty*choice;
  for(int lo=0;lo<l;lo++)if(valid[lo]&&cost[lo]-penalty*lo<best-1e-10)best=cost[lo]-penalty*lo,choice=lo;
  lower[k]=choice;current=cost[choice];
 }
 int cut=accumulate(lower.begin(),lower.end(),0);cerr<<"CUT intrinsic "<<double(initial)/P<<" -> "<<double(calc())/P<<" holes="<<cut<<"\n";
}

int main(int argc,char**argv){
 int n,K,P,R;cin>>n>>K>>P>>R;vector<U64>A(P);for(auto &a:A){string s;cin>>s;for(int j=0;j<n;j++)if(s[j]=='1')a|=1ULL<<j;}
 mt19937_64 rng(12345);vector<int>cols(n),flip(n);iota(cols.begin(),cols.end(),0);
 int correlation[64][64]{};for(int x=0;x<n;x++)for(int y=0;y<n;y++)for(U64 v:A)correlation[x][y]+=((v>>x)^(v>>y))&1;
 auto cost=[&](){int s=0;for(int j=1;j<n;j++){int d=correlation[cols[j-1]][cols[j]];s+=min(d,P-d);}return s;};int current=cost(),best=current;auto bestcols=cols;
 for(int it=0;it<300000;it++){int x=rng()%n,y=rng()%n;if(x>y)swap(x,y);(void)(rng()%3);auto edge=[&](int a,int b){int d=correlation[a][b];return min(d,P-d);};int next=current;if(x)next+=edge(cols[x-1],cols[y])-edge(cols[x-1],cols[x]);if(y<63)next+=edge(cols[x],cols[y+1])-edge(cols[y],cols[y+1]);
  reverse(cols.begin()+x,cols.begin()+y+1);double temperature=8*pow(.002,double(it)/300000);if(next<=current||double(rng()>>11)*0x1p-53<exp((current-next)/temperature)){current=next;if(current<best)best=current,bestcols=cols;}else reverse(cols.begin()+x,cols.begin()+y+1);}
 cols=bestcols;for(int j=1;j<n;j++)flip[j]=flip[j-1]^(correlation[cols[j-1]][cols[j]]>P/2);for(U64&v:A){U64 old=v;v=0;for(int j=0;j<n;j++)v|=(((old>>cols[j])&1)^flip[j])<<j;}
 vector<PrefixPaper>papers;train_backbone_dictionary(A,papers,n,6,1,8);refit_all_prefix_papers(A,papers,4,32);K=papers.size();vector<int>lower(K),length(K),order(K);iota(order.begin(),order.end(),0);
 double penalty=argc>1?atof(argv[1]):.1;int passes=argc>3?atoi(argv[3]):1;if(penalty>0)learn_lower(A,papers,lower,penalty,passes);
 vector<U64>mask(K);for(int k=0;k<K;k++)mask[k]=PrefixDP::low(length[k]=papers[k].len)^PrefixDP::low(lower[k]);
 IntervalDP dp(n,papers,lower);long long total=0;int maxcount=0,improvement=0;int rounds=argc>2?min(R,atoi(argv[2])):R;
 for(int r=0;r<rounds;r++){
  int a;cin>>a;int distance;auto goal=dp.solve(order,A[a],distance);vector<char>selected(K);for(int k:goal)selected[k]=1;int used=0;
  while(true){U64 covered=0,display=0;vector<char>visible(K);for(int k:order){visible[k]=bool(mask[k]&~covered);display|=papers[k].color&mask[k]&~covered;covered|=mask[k];}if(display==A[a])break;
   auto[from,to]=TrimPrefix::best_move(order,length,selected,visible);if(from<0||used>=384){cerr<<"STUCK "<<r<<"\n";return 1;}int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);used++;
  }total+=used;improvement+=distance-used;maxcount=max(maxcount,used);
 }
 cerr<<"TOTAL "<<total<<" avg="<<double(total)/rounds<<" max="<<maxcount<<" extra_saving="<<improvement<<" CPU="<<double(clock())/CLOCKS_PER_SEC<<"\n";
}

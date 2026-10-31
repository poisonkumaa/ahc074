#include "prefix_dp.h"
#include "templates_agent_trim_move.h"
#include "structure2_agent_general_move.h"
struct Card{int x,y;U64 mask,color;};
int main(int argc,char**argv){int N,K,P,R;cin>>N>>K>>P>>R;vector<U64>A(P);for(auto &a:A){string s;cin>>s;for(int j=0;j<N;j++)if(s[j]=='1')a|=1ULL<<j;}
 vector<Card>cards;ifstream design(argc>1?argv[1]:"tools/experiments/templates_agent_2d32_result.txt");int x,y;U64 cx,cy;while(design>>x>>y>>cx>>cy)cards.push_back({x,y,PrefixDP::low(x)|(PrefixDP::low(y)<<32),cx|(cy<<32)});K=cards.size();
 vector<vector<int>>goals(P),levels(P,vector<int>(K));vector<vector<char>>chosen(P,vector<char>(K));double intrinsic=0;
 for(int v=0;v<P;v++){
  int dp[33][33],pk[33][33],prev[33][33];fill(&dp[0][0],&dp[0][0]+1089,1000);dp[0][0]=0;
  vector<int>sx(K),sy(K);for(int k=0;k<K;k++){auto c=cards[k];U64 dx=(c.color^A[v])&PrefixDP::low(c.x),dy=((c.color^A[v])>>32)&PrefixDP::low(c.y);sx[k]=dx?64-__builtin_clzll(dx):0;sy[k]=dy?64-__builtin_clzll(dy):0;}
  for(int i=0;i<=32;i++)for(int j=0;j<=32;j++)if(dp[i][j]<1000)for(int k=0;k<K;k++)if(i>=sx[k]&&j>=sy[k]){int nx=max(i,cards[k].x),ny=max(j,cards[k].y);if((nx!=i||ny!=j)&&dp[i][j]+1<dp[nx][ny])dp[nx][ny]=dp[i][j]+1,pk[nx][ny]=k,prev[nx][ny]=i*33+j;}
  if(dp[32][32]>=1000){cerr<<"UNREACHABLE\n";return 1;}intrinsic+=double(dp[32][32])/P;
  int i=32,j=32;while(i||j){int k=pk[i][j],p=prev[i][j];goals[v].push_back(k);i=p/33;j=p%33;}reverse(goals[v].begin(),goals[v].end());
  U64 covered=0;int rank=0;for(int g:goals[v]){U64 own=cards[g].mask&~covered;covered|=cards[g].mask;rank++;chosen[v][g]=true;for(int k=0;k<K;k++)if(cards[k].mask&own)levels[v][k]=rank;}
 }
 vector<int>order(K),quality_length;int qm=argc>4?atoi(argv[4]):0;for(auto c:cards){int key=c.x+c.y;if(qm==1)key=c.y==0?c.x:c.x==0?64+c.y:128+c.x+c.y;if(qm==2)key=c.x==0?c.y:c.y==0?64+c.x:128+c.x+c.y;if(qm==3)key=65*c.x+c.y;if(qm==4)key=65*c.y+c.x;quality_length.push_back(key);}iota(order.begin(),order.end(),0);long long total=0;int maxcount=0;int rounds=argc>2?min(R,atoi(argv[2])):R;
 auto replan=[&](int v){
  vector<int>bx(K),by(K);int seenx[33]{},seeny[33]{};for(int k:order){bx[k]=seenx[cards[k].x];by[k]=seeny[cards[k].y];for(int t=1;t<=cards[k].x;t++)seenx[t]++;for(int t=1;t<=cards[k].y;t++)seeny[t]++;}
  int dp[33][33],pk[33][33],prev[33][33];fill(&dp[0][0],&dp[0][0]+1089,100000000);dp[0][0]=0;vector<int>sx(K),sy(K);
  for(int k=0;k<K;k++){auto c=cards[k];U64 dx=(c.color^A[v])&PrefixDP::low(c.x),dy=((c.color^A[v])>>32)&PrefixDP::low(c.y);sx[k]=dx?64-__builtin_clzll(dx):0;sy[k]=dy?64-__builtin_clzll(dy):0;}
  for(int i=0;i<=32;i++)for(int j=0;j<=32;j++)if(dp[i][j]<100000000)for(int k=0;k<K;k++)if(i>=sx[k]&&j>=sy[k]){
   int nx=max(i,cards[k].x),ny=max(j,cards[k].y);if(nx==i&&ny==j)continue;int retrieval=1000;if(nx>i)retrieval=min(retrieval,bx[k]);if(ny>j)retrieval=min(retrieval,by[k]);int value=dp[i][j]+256*(retrieval+1)+1;
   if(value<dp[nx][ny])dp[nx][ny]=value,pk[nx][ny]=k,prev[nx][ny]=i*33+j;
  }
  goals[v].clear();fill(chosen[v].begin(),chosen[v].end(),0);fill(levels[v].begin(),levels[v].end(),0);
  int i=32,j=32;while(i||j){int k=pk[i][j],p=prev[i][j];goals[v].push_back(k);i=p/33;j=p%33;}reverse(goals[v].begin(),goals[v].end());
  U64 covered=0;int rank=0;for(int g:goals[v]){U64 own=cards[g].mask&~covered;covered|=cards[g].mask;rank++;chosen[v][g]=true;for(int k=0;k<K;k++)if(cards[k].mask&own)levels[v][k]=rank;}
 };
 for(int r=0;r<rounds;r++){
  int a;cin>>a;int used=0;if(argc>3&&atoi(argv[3]))replan(a);auto original_order=order;
  bool greedy=argc>5&&atoi(argv[5]);vector<int>precedence(K*K);if(greedy){U64 co=0;for(int g:goals[a]){U64 own=cards[g].mask&~co;co|=cards[g].mask;for(int k=0;k<K;k++)precedence[g*K+k]=__builtin_popcountll(own&cards[k].mask&(cards[k].color^A[a]));}}
  while(TrimPrefix::fixed_distance(order,levels[a],chosen[a])){
   U64 covered=0,display=0;vector<char>visible(K);for(int k:order){visible[k]=bool(cards[k].mask&~covered);display|=cards[k].color&cards[k].mask&~covered;covered|=cards[k].mask;}if(display==A[a])break;
   pair<int,int>action{-1,-1};
   if(greedy){int best=0,tie=1000000;
    for(int i=0;i<K;i++){int k=order[i];if(!visible[k])continue;for(int dir:{-1,1}){int delta=0,inv=0;for(int d=i+dir;d>=0&&d<K;d+=dir){int x=order[d];delta+=dir*(precedence[k*K+x]-precedence[x*K+k]);inv+=dir*((quality_length[k]<quality_length[x])-(quality_length[k]>quality_length[x]));if(delta<best||(delta==best&&delta<0&&inv<tie))best=delta,tie=inv,action={i,d};}}}
    if(action.first<0||used>=150){order=original_order;used=0;greedy=false;continue;}
   }else action=GeneralPrefix::best_move(order,levels[a],chosen[a],visible,quality_length);
   auto[from,to]=action;if(from<0||used>=384){cerr<<"STUCK "<<r<<" "<<used<<"\n";return 1;}int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);used++;
  }
  U64 covered=0,display=0;for(int k:order){display|=cards[k].color&cards[k].mask&~covered;covered|=cards[k].mask;}if(display!=A[a]){cerr<<"WRONG "<<r<<"\n";return 1;}
  total+=used;maxcount=max(maxcount,used);if(r%128==0)cerr<<r<<" avg="<<double(total)/(r+1)<<"\n";
 }
 cerr<<"TOTAL "<<total<<" avg="<<double(total)/rounds<<" intrinsic="<<intrinsic<<" max="<<maxcount<<" CPU="<<double(clock())/CLOCKS_PER_SEC<<"\n";
}

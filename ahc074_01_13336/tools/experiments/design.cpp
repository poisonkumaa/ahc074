#include <bits/stdc++.h>
using namespace std;
using U=uint64_t;
struct Card { U m,b; };
mt19937 rng(712367);
int pc(U x){return __builtin_popcountll(x);}
int main(int argc,char**argv){
 int N,K,P,R;cin>>N>>K>>P>>R;vector<U>A(P);string s;
 for(U &a:A){cin>>s;for(int j=0;j<N;j++)if(s[j]=='1')a|=1ULL<<j;}
 int group=argc>1?atoi(argv[1]):3;
 vector<Card>C;vector<int> ids(P);iota(ids.begin(),ids.end(),0);
 while(C.size()<K){shuffle(ids.begin(),ids.end(),rng);for(int i=0;i+group<=P&&C.size()<K;i+=group){U m=~0ULL,b=A[ids[i]];for(int j=1;j<group;j++)m&=~(b^A[ids[i+j]]);C.push_back({m,b&m});}}
 double sum=0;int fail=0,remain=0,mx=0;
 for(U a:A){U u=~0ULL;int n=0;while(u){int best=-1,score=0;for(int i=0;i<K;i++){if(((a^C[i].b)&C[i].m&u)==0){int t=pc(C[i].m&u);if(t>score)score=t,best=i;}}if(best<0)break;u&=~C[best].m;n++;}sum+=n;fail+=u!=0;remain+=pc(u);mx=max(mx,n);}
 cerr<<"group="<<group<<" mean="<<sum/P<<" fails="<<fail<<" remain="<<remain<<" max="<<mx<<endl;
}

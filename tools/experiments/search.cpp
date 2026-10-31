#include <bits/stdc++.h>
using namespace std;
using U=uint64_t;
struct Card{U m,b;};
int pc(U x){return __builtin_popcountll(x);}
mt19937 rng(712367);
int main(int argc,char**argv){
 int N,K,P,R;cin>>N>>K>>P>>R;vector<U>A(P);string s;
 for(U &a:A){cin>>s;for(int j=0;j<N;j++)if(s[j]=='1')a|=1ULL<<j;}
 int mode=argc>1?atoi(argv[1]):0,sz=argc>2?atoi(argv[2]):8;
 vector<Card>C;
 if(mode==0)for(int j=0;j<N;j++){C.push_back({1ULL<<j,0});C.push_back({1ULL<<j,1ULL<<j});}
 if(mode==1)for(int j=0;j<N;j++)C.push_back({1ULL<<j,1ULL<<j});
 while(C.size()<K){U m=0,b=A[rng()%P];while(pc(m)<sz)m|=1ULL<<(rng()%N);if(mode==1)b=0;C.push_back({m,b&m});}
 vector<int>order(K);iota(order.begin(),order.end(),0);long long total=0;int errors=0,mx=0;
 for(int r=0;r<R;r++){
  int a;cin>>a;U target=A[a];int steps=0;
  while(steps<2*K){
   vector<U>preM(K+1),preB(K+1),sufB(K+1);
   for(int i=0;i<K;i++){auto c=C[order[i]];preB[i+1]=preB[i]|(c.b&~preM[i]);preM[i+1]=preM[i]|c.m;}
   if(preB[K]==target)break;
   for(int i=K-1;i>=0;i--){auto c=C[order[i]];sufB[i]=c.b|(sufB[i+1]&~c.m);}
   int old=pc(preB[K]^target),best=old,bi=-1,bj=-1;double bestTie=-1e9;
   for(int i=0;i<K;i++){
    auto c=C[order[i]];U visible=c.m&~preM[i];if(!visible)continue;
    // Removing a paper changes only its currently visible windows.
    U underneath=0,covered=0;
    for(int j=i+1;j<K;j++){
     auto d=C[order[j]];underneath|=d.b&~covered;covered|=d.m;
     U val=(preB[K]&~visible)|(visible&(underneath|(c.b&~covered)));
     int score=pc(val^target);double tie=pc(covered&visible)*0.01+j*0.00001;
     if(score<best||(score==best&&score<old&&tie>bestTie))best=score,bi=i,bj=j,bestTie=tie;
    }
    for(int j=0;j<i;j++){
     U changed=c.m&~preM[j];U val=(preB[K]&~changed)|(c.b&changed);
     int score=pc(val^target);double tie=pc(changed)*0.01-j*0.00001;
     if(score<best||(score==best&&score<old&&tie>bestTie))best=score,bi=i,bj=j,bestTie=tie;
    }
   }
   if(bi<0){for(int i=0;i<K;i++){auto c=C[order[i]];if((c.b^target)&c.m&~preM[i]){bi=i;bj=K-1;break;}} if(bi<0){errors++;break;}}
   int id=order[bi];order.erase(order.begin()+bi);order.insert(order.begin()+bj,id);steps++;
  }
  if(steps==2*K)errors++;total+=steps;mx=max(mx,steps);
 }
 cerr<<"mode="<<mode<<" size="<<sz<<" score="<<total+2*K*errors<<" ops="<<total<<" errors="<<errors<<" max="<<mx<<endl;
}

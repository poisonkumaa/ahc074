#include <bits/stdc++.h>
using namespace std;using U=uint64_t;int pc(U x){return __builtin_popcountll(x);}struct Card{U m,b;};mt19937 rng(712367);
int main(int argc,char**argv){int N,K,P,R;cin>>N>>K>>P>>R;vector<U>A(P);string s;for(U&a:A){cin>>s;for(int j=0;j<N;j++)if(s[j]=='1')a|=1ULL<<j;}
 int sz=argc>1?atoi(argv[1]):8,mode=argc>2?atoi(argv[2]):1;vector<Card>C;
 if(mode==0)for(int j=0;j<N;j++){C.push_back({1ULL<<j,0});C.push_back({1ULL<<j,1ULL<<j});}
 if(mode==1)for(int j=0;j<N;j++)C.push_back({1ULL<<j,1ULL<<j});
 while(C.size()<K){U m=0,b=A[rng()%P];while(pc(m)<sz)m|=1ULL<<(rng()%N);if(mode==1)b=0;C.push_back({m,b&m});}
 vector<int>o(K);iota(o.begin(),o.end(),0);long long total=0;int maxlen=0;auto start=chrono::steady_clock::now();
 for(int r=0;r<min(128,R);r++){int a;cin>>a;U t=A[a];vector<U>w(K);for(int i=0;i<K;i++)w[i]=(C[i].b^t)&C[i].m;
  auto eval=[&](int from,int to){U covered=0;int cost=0;auto add=[&](int k){if(w[k]&~covered)cost++;else covered|=C[k].m;};for(int i=0;i<K;i++){if(i==to&&from>to)add(o[from]);if(i!=from)add(o[i]);if(i==to&&from<to)add(o[from]);if(covered==~0ULL)break;}return cost;};
  int steps=0;
  while(true){int current=eval(-1,-1);if(current==0)break;int best=current,bi=-1,bj=-1;U covered=0;
   for(int i=0;i<K;i++){int k=o[i];U visible=C[k].m&~covered;covered|=C[k].m;if(!visible)continue;
    for(int j=0;j<K;j++){if(i==j)continue;int f=eval(i,j);if(f<best){best=f;bi=i;bj=j;}}
   }
   if(bi<0){cerr<<"FAILED"<<endl;return 1;}
   int k=o[bi];o.erase(o.begin()+bi);o.insert(o.begin()+bj,k);steps++;
  }total+=steps;maxlen=max(maxlen,steps);
 }
 cerr<<"size="<<sz<<" mode="<<mode<<" avg="<<total/128.<<" max="<<maxlen<<" sec="<<chrono::duration<double>(chrono::steady_clock::now()-start).count()<<endl;
}

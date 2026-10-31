#include <bits/stdc++.h>
using namespace std;
struct Edge{int from,i,j;};
int main(){vector<array<int,6>>perms;array<int,6>p={0,1,2,3,4,5};map<array<int,6>,int>ids;do{ids[p]=perms.size();perms.push_back(p);}while(next_permutation(p.begin(),p.end()));
 double best=10;int bestset=0;
 for(int subset=0;subset<256;subset++)if(__builtin_popcount((unsigned)subset)==6){
  vector<pair<int,int>>cards;for(int v=1;v<9;v++)if(subset>>(v-1)&1){int m=0,b=0,t=v;for(int j=0;j<2;j++){int x=t%3;t/=3;if(x)m|=1<<j;if(x==2)b|=1<<j;}cards.push_back({m,b});}
  vector<vector<Edge>>rev(720);vector<int>display(720);
  for(int s=0;s<720;s++){auto o=perms[s];int cov=0,b=0;for(int i=0;i<6;i++){auto [m,c]=cards[o[i]];b|=c&~cov;if(m&~cov){for(int j=0;j<6;j++)if(i!=j){auto q=o;if(i<j)rotate(q.begin()+i,q.begin()+i+1,q.begin()+j+1);else rotate(q.begin()+j,q.begin()+i,q.begin()+i+1);rev[ids[q]].push_back({s,i,j});}}cov|=m;}display[s]=b;}
  vector<double>V(720),newV(720);double gain=0;
  for(int it=0;it<80;it++){fill(newV.begin(),newV.end(),0);for(int target=0;target<4;target++){
   vector<double>d(720,1e6);priority_queue<pair<double,int>,vector<pair<double,int>>,greater<pair<double,int>>>pq;
   for(int s=0;s<720;s++)if(display[s]==target)d[s]=V[s],pq.push({d[s],s});
   while(!pq.empty()){auto [val,s]=pq.top();pq.pop();if(val!=d[s])continue;for(auto e:rev[s])if(d[e.from]>val+1+1e-10)d[e.from]=val+1,pq.push({val+1,e.from});}
   for(int s=0;s<720;s++)newV[s]+=d[s]/4;
  }gain=newV[0];for(double &v:newV)v-=gain;V.swap(newV);}
  cerr<<subset<<" "<<gain<<endl;if(gain<best)best=gain,bestset=subset;
 }
 cout<<"BEST "<<bestset<<" "<<best<<endl;
}

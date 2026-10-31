#include <bits/stdc++.h>
using namespace std;
struct Card{int m,b;};
struct Model{
 int bits;vector<array<int,6>>p;int nxt[720][6][6];
 Model(int b):bits(b){array<int,6>a={0,1,2,3,4,5};map<array<int,6>,int>ids;do{ids[a]=p.size();p.push_back(a);}while(next_permutation(a.begin(),a.end()));for(int s=0;s<720;s++)for(int i=0;i<6;i++)for(int j=0;j<6;j++){auto q=p[s];if(i<j)rotate(q.begin()+i,q.begin()+i+1,q.begin()+j+1);else if(i>j)rotate(q.begin()+j,q.begin()+i,q.begin()+i+1);nxt[s][i][j]=ids[q];}}
 double eval(array<Card,6>cards,int iterations){
  vector<vector<int>>rev(720);array<int,720>display;int exists=0;
  for(int s=0;s<720;s++){int co=0,b=0;for(int i=0;i<6;i++){auto c=cards[p[s][i]];b|=c.b&~co;if(c.m&~co)for(int j=0;j<6;j++)if(i!=j)rev[nxt[s][i][j]].push_back(s);co|=c.m;}display[s]=b;exists|=1<<b;}
  if(exists!=((1<<(1<<bits))-1))return 100;
  array<double,720>v{},nv;double g=0;
  for(int it=0;it<iterations;it++){nv.fill(0);for(int t=0;t<(1<<bits);t++){array<double,720>d;d.fill(1e6);priority_queue<pair<double,int>,vector<pair<double,int>>,greater<pair<double,int>>>q;for(int s=0;s<720;s++)if(display[s]==t)d[s]=v[s],q.push({v[s],s});while(!q.empty()){auto [z,s]=q.top();q.pop();if(z>d[s]+1e-9)continue;for(int f:rev[s])if(d[f]>z+1+1e-9)d[f]=z+1,q.push({z+1,f});}for(int s=0;s<720;s++)nv[s]+=d[s]/(1<<bits);}g=nv[0];for(double &x:nv)x-=g;v=nv;}
  return g;
 }
};
int main(int argc,char**argv){int bits=argc>1?atoi(argv[1]):3,iters=argc>2?atoi(argv[2]):1000;Model md(bits);mt19937 rng(1234567);array<Card,6>cards;for(int i=0;i<6;i++)cards[i]={1<<(i/2),i%2?1<<(i/2):0};if(bits==4){for(int i=0;i<4;i++)cards[i]={1<<i,1<<i};cards[4]={15,0};cards[5]={15,15};}double current=md.eval(cards,20),best=current;auto bestcards=cards;
 for(int it=0;it<iters;it++){auto cand=cards;int i=rng()%6,j=rng()%bits,v=rng()%3;cand[i].m&=~(1<<j);cand[i].b&=~(1<<j);if(v)cand[i].m|=1<<j;if(v==2)cand[i].b|=1<<j;double sc=md.eval(cand,12);double temp=0.04*pow(0.05,double(it)/iters);if(sc<current||double(rng())/rng.max()<exp((current-sc)/temp))cards=cand,current=sc;if(sc<best-1e-7){best=sc;bestcards=cand;cerr<<it<<" best="<<best<<endl;for(auto c:cand){for(int j=0;j<bits;j++)cerr<<(!(c.m>>j&1)?'.':c.b>>j&1?'1':'0');cerr<<" ";}cerr<<endl;}}
 cout<<md.eval(bestcards,100)<<endl;for(auto c:bestcards){for(int j=0;j<bits;j++)cout<<(!(c.m>>j&1)?'.':c.b>>j&1?'1':'0');cout<<endl;}
}

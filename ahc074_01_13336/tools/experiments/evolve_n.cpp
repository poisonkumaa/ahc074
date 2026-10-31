#include <bits/stdc++.h>
using namespace std;
struct Card{int m,b;};
struct Edge{int from;uint8_t action;};
struct Model{
 int n,bits,ns;vector<array<uint8_t,9>>p;vector<array<int,81>>nxt;
 vector<array<uint8_t,16>>policy;
 Model(int nn,int bb):n(nn),bits(bb){array<uint8_t,9>a{};iota(a.begin(),a.begin()+n,0);do{p.push_back(a);}while(next_permutation(a.begin(),a.begin()+n));ns=p.size();nxt.resize(ns);for(int s=0;s<ns;s++)for(int i=0;i<n;i++)for(int j=0;j<n;j++){auto q=p[s];if(i<j)rotate(q.begin()+i,q.begin()+i+1,q.begin()+j+1);else if(i>j)rotate(q.begin()+j,q.begin()+i,q.begin()+i+1);nxt[s][i*n+j]=rank(q);}}
 int rank(array<uint8_t,9>a){int r=0;for(int i=0;i<n;i++){int k=0;for(int j=i+1;j<n;j++)k+=a[j]<a[i];r=r*(n-i)+k;}return r;}
 double eval(vector<Card>cards,int iterations,bool save=false){
  vector<vector<Edge>>rev(ns);vector<int>display(ns);unsigned exists=0;
  for(int s=0;s<ns;s++){int co=0,b=0;for(int i=0;i<n;i++){auto c=cards[p[s][i]];b|=c.b&~co;if(c.m&~co)for(int j=0;j<n;j++)if(i!=j)rev[nxt[s][i*n+j]].push_back({s,(uint8_t)(i*n+j)});co|=c.m;}display[s]=b;exists|=1u<<b;}
  if(exists!=((1u<<(1<<bits))-1))return 100;
  vector<double>v(ns),nv(ns);double g=0;if(save)policy.resize(ns);
  for(int it=0;it<iterations;it++){fill(nv.begin(),nv.end(),0);for(int t=0;t<(1<<bits);t++){vector<double>d(ns,1e6);priority_queue<pair<double,int>,vector<pair<double,int>>,greater<pair<double,int>>>q;for(int s=0;s<ns;s++){if(save)policy[s][t]=255;if(display[s]==t)d[s]=v[s],q.push({v[s],s});}while(!q.empty()){auto [z,s]=q.top();q.pop();if(z>d[s]+1e-9)continue;for(auto e:rev[s])if(d[e.from]>z+1+1e-9){d[e.from]=z+1;q.push({z+1,e.from});if(save)policy[e.from][t]=e.action;}}for(int s=0;s<ns;s++)nv[s]+=d[s]/(1<<bits);}g=nv[0];for(double &x:nv)x-=g;v.swap(nv);}
  return g;
 }
 void export_policy(vector<Card>cards,string name){
  cerr<<"final="<<eval(cards,40,true)<<endl;
  map<int,int>ids;vector<int>states{0};ids[0]=0;
  struct Entry{int next;vector<int>moves;};vector<array<Entry,16>>entries;
  for(int at=0;at<(int)states.size();at++){int s=states[at];array<Entry,16>es;for(int t=0;t<16;t++){int cur=s;vector<int>moves;while(policy[cur][t]!=255){int act=policy[cur][t];moves.push_back(p[cur][act/n]*n+act%n);cur=nxt[cur][act];if(moves.size()>20){cerr<<"loop"<<endl;exit(1);}}if(!ids.count(cur)){ids[cur]=states.size();states.push_back(cur);}es[t]={ids[cur],moves};}entries.push_back(es);}
  cerr<<"closure="<<states.size()<<endl;
  ofstream out(name);out<<n<<" "<<states.size()<<"\n";for(auto c:cards)out<<c.m<<" "<<c.b<<"\n";for(auto es:entries){for(auto e:es){out<<e.next<<" "<<e.moves.size();for(int a:e.moves)out<<" "<<a;out<<"\n";}}
 }
};
int main(int argc,char**argv){int n=argc>1?atoi(argv[1]):7,iters=argc>2?atoi(argv[2]):1000;Model md(n,4);mt19937 rng(1234567);vector<Card>cards={{11,8},{15,13},{2,2},{15,9},{15,8},{8,0}};while(cards.size()<n)cards.push_back({1,1});if(argc>3){ifstream in(argv[3]);int nn,ss;in>>nn>>ss;cards.resize(nn);for(auto&c:cards)in>>c.m>>c.b;while(cards.size()<n)cards.push_back({1,1});}
 double current=md.eval(cards,32),best=current;auto bestcards=cards;cerr<<"initial="<<current<<endl;
 for(int it=0;it<iters;it++){auto cand=cards;int i=rng()%n;int changes=rng()%4==0?4:1;for(int z=0;z<changes;z++){int j=rng()%4,v=rng()%3;cand[i].m&=~(1<<j);cand[i].b&=~(1<<j);if(v)cand[i].m|=1<<j;if(v==2)cand[i].b|=1<<j;}if(rng()%8==0)shuffle(cand.begin(),cand.end(),rng);double sc=md.eval(cand,12);double temp=0.15*pow(0.01,double(it)/max(iters,1));if(sc<current||double(rng())/rng.max()<exp((current-sc)/temp))cards=cand,current=sc;if(sc<best-1e-7){best=sc;bestcards=cand;cerr<<it<<" best="<<best<<endl;}}
 string name="tools/experiments/policy"+to_string(n)+".txt";md.export_policy(bestcards,name);
}

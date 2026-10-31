#include <bits/stdc++.h>
using namespace std;using U64=uint64_t;
struct Rng{U64 x=9145123424;U64 next(){x^=x<<7;return x^=x>>9;}int pick(int n){return next()%n;}double real(){return (next()>>11)*0x1.0p-53;}}rnd;
struct Move{int k,d;};
int main(int argc,char**argv){int N=argc>1?atoi(argv[1]):8,K=3*N,P=4*N,D=argc>2?atoi(argv[2]):3,steps=argc>3?atoi(argv[3]):2000000;vector<U64>M(K),B(K),A(P);U64 all=N==64?~0ULL:(1ULL<<N)-1;for(int i=0;i<K;i++){int j=i/3;U64 lower=j?((1ULL<<j)-1):0;M[i]=(rnd.next()&lower)|(1ULL<<j);B[i]=rnd.next()&M[i];}for(auto&a:A)a=rnd.next()&all;
 vector<vector<Move>>path(P,vector<Move>(D));vector<int>id(K);iota(id.begin(),id.end(),0);
 auto vis=[&](const vector<int>&p){U64 covered=0,display=0;vector<int>visible;for(int k:p){U64 v=M[k]&~covered;if(v)visible.push_back(k);display|=B[k]&v;covered|=M[k];}return pair{display,visible};};
 auto apply=[&](vector<int>&p,Move m){int i=find(p.begin(),p.end(),m.k)-p.begin();p.erase(p.begin()+i);p.insert(p.begin()+m.d,m.k);};
 for(auto &pa:path){auto p=id;for(auto &m:pa){auto vv=vis(p).second;for(int tr=0;;tr++){m={vv[rnd.pick(vv.size())],rnd.pick(K)};auto q=p;apply(q,m);auto w=vis(q).second;if(find(w.begin(),w.end(),m.k)!=w.end()){p=q;break;}}}}
 auto eval=[&](int a){auto p=id;int bad=0;for(auto m:path[a]){auto vv=vis(p).second;if(find(vv.begin(),vv.end(),m.k)==vv.end())bad++;int oldpos=find(p.begin(),p.end(),m.k)-p.begin();apply(p,m);vv=vis(p).second;if(abs(oldpos-m.d)!=1&&find(vv.begin(),vv.end(),m.k)==vv.end())bad++;}return double(__builtin_popcountll(vis(p).first^A[a]))+3.0*bad;};
 auto replan=[&](int a){
  struct Node{vector<int>p;vector<Move>moves;int score;U64 hash;};
  vector<Node>beam={{id,{},__builtin_popcountll(vis(id).first^A[a]),0}};
  vector<Move>bestpath;int bv=100000;
  for(int depth=0;depth<D;depth++){
   vector<Node>next;unordered_set<U64>seen;
   for(auto &nd:beam){auto vv=vis(nd.p).second;for(int k:vv)for(int d=0;d<K;d++){
    auto np=nd.p;Move mv{k,d};int oldpos=find(np.begin(),np.end(),k)-np.begin();apply(np,mv);U64 h=0;for(int q:np)h=(h^(q+1))*0x9e3779b97f4a7c15ULL;if(!seen.insert(h).second)continue;
    auto [disp,nv]=vis(np);if(abs(oldpos-d)!=1&&find(nv.begin(),nv.end(),k)==nv.end())continue;
    int sc=__builtin_popcountll(disp^A[a]);auto moves=nd.moves;moves.push_back(mv);
    if(depth==D-1&&sc<bv)bv=sc,bestpath=moves;
    if(depth<D-1){int future=sc;for(int v:nv)future=min(future,__builtin_popcountll(((disp&~M[v])|B[v])^A[a]));sc+=10*future;}
    next.push_back({move(np),move(moves),sc,h});
   }}
   sort(next.begin(),next.end(),[](auto&a,auto&b){return tie(a.score,a.hash)<tie(b.score,b.hash);});if(next.size()>48)next.resize(48);beam=move(next);
  }
  if(!bestpath.empty()&&bv<eval(a))path[a]=bestpath;
 };
 for(int a=0;a<P;a++)replan(a);
 vector<double>score(P);double total=0;for(int a=0;a<P;a++)total+=score[a]=eval(a);double best=total;auto bm=M,bb=B;auto bp=path;cerr<<"initial="<<total<<"\n";
 for(int it=0;it<steps;it++){
  double t=.6*pow(.025,double(it%500000)/500000);
  if(rnd.pick(20)==0){int k=rnd.pick(K),j=rnd.pick(N),v=rnd.pick(3);
   if(rnd.pick(4)!=0){int a=rnd.pick(P);auto p=id;for(auto mv:path[a])apply(p,mv);U64 wrong=vis(p).first^A[a];if(wrong){int nth=rnd.pick(__builtin_popcountll(wrong));while(nth--)wrong&=wrong-1;j=__builtin_ctzll(wrong);for(int q:p)if(M[q]>>j&1){k=q;break;}v=rnd.pick(2)?0:1+int(A[a]>>j&1);}}
   U64 om=M[k],ob=B[k];M[k]&=~(1ULL<<j);B[k]&=~(1ULL<<j);if(v)M[k]|=1ULL<<j;if(v==2)B[k]|=1ULL<<j;vector<double>ns(P);double nt=0;for(int a=0;a<P;a++)nt+=ns[a]=eval(a);if(nt<=total||rnd.real()<exp((total-nt)/t)){score=ns;total=nt;}else M[k]=om,B[k]=ob;
  }else{int a=rnd.pick(P),z=rnd.pick(D);Move old=path[a][z];auto p=id;for(int j=0;j<z;j++)apply(p,path[a][j]);auto vv=vis(p).second;if(vv.empty())continue;Move nm={vv[rnd.pick(vv.size())],rnd.pick(K)};path[a][z]=nm;double ns=eval(a),delta=ns-score[a];if(delta<=0||rnd.real()<exp(-delta/t)){total+=delta;score[a]=ns;}else path[a][z]=old;}
  if(total<best){best=total;bm=M;bb=B;bp=path;if(best<20||int(best)%10==0)cerr<<it<<" best="<<best<<"\n";if(best==0)break;}
  if(it%100000==99999){for(int a=0;a<P;a++)replan(a);total=0;for(int a=0;a<P;a++)total+=score[a]=eval(a);if(total<best){best=total;bm=M;bb=B;bp=path;}}
  if(it%500000==499999){M=bm;B=bb;path=bp;total=0;for(int a=0;a<P;a++)total+=score[a]=eval(a);cerr<<"restart="<<it<<" best="<<best<<"\n";}
 }
 cerr<<"FINAL "<<best<<"\n";for(int k=0;k<K;k++){for(int j=0;j<N;j++)cout<<(!(bm[k]>>j&1)?'.':char('0'+(bb[k]>>j&1)));cout<<"\n";}for(int a=0;a<P;a++){for(int j=0;j<N;j++)cout<<char('0'+(A[a]>>j&1));cout<<" ";for(auto m:bp[a])cout<<m.k<<","<<m.d<<" ";cout<<"\n";}
}

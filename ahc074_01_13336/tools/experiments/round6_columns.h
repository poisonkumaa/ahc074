#pragma once
static int round6_column_steps=500,round6_column_weight=1,round6_column_repetitions=8;
inline void round6_optimize_columns(vector<U64>&A,vector<int>&cols,vector<int>&flip,vector<PrefixPaper>&papers,int steps,int residualweight){
 const int P=A.size(),K=papers.size(),LOW=32,INF=1000;vector<int>representative(K,-1),weight(P,residualweight);array<U64,65>mask{};for(int l=0;l<=64;l++)mask[l]=PrefixDP::low(l);
 for(int k=0;k<K;k++){int l=papers[k].len;if(papers[k].color==PrefixDP::low(l-1)||(l==64&&papers[k].color==~0ULL))continue;for(int v=0;v<P;v++)if((A[v]&mask[l])==papers[k].color){representative[k]=v;weight[v]=1;break;}assert(representative[k]>=0);}
 vector<array<int,LOW+1>>prefix(P);for(int v=0;v<P;v++){prefix[v].fill(INF);prefix[v][0]=0;for(auto p:papers)if(p.len<=LOW){U64 diff=(A[v]^p.color)&mask[p.len];int start=diff?64-__builtin_clzll(diff):0;for(int j=start;j<p.len;j++)prefix[v][p.len]=min(prefix[v][p.len],prefix[v][j]+1);}}
 auto objective=[&](){int total=0;U64 color[192];for(int k=0;k<K;k++)color[k]=representative[k]<0?papers[k].color:A[representative[k]];for(int v=0;v<P;v++){int f[65];copy(prefix[v].begin(),prefix[v].end(),f);fill(f+LOW+1,f+65,INF);for(int k=0;k<K;k++){int l=papers[k].len;if(l<=LOW)continue;U64 diff=(A[v]^color[k])&mask[l];int start=diff?64-__builtin_clzll(diff):0;int best=INF;for(int j=start;j<l;j++)best=min(best,f[j]);f[l]=min(f[l],best+1);}total+=weight[v]*f[64];}return total;};
 int current=objective(),initial=current,best=current;auto bestA=A;auto bestcols=cols,bestflip=flip;mt19937_64 rng(A[0]^0x81bc7123679ULL);
 auto swapbits=[&](int x,int y){U64 m=(1ULL<<x)|(1ULL<<y);for(U64&v:A)if(((v>>x)^(v>>y))&1)v^=m;swap(cols[x],cols[y]);swap(flip[x],flip[y]);};
 for(int it=0;it<steps;it++){
  int x=LOW+rng()%(64-LOW),y=LOW+rng()%(64-LOW);if(rng()%2)y=x==63?62:x+1;if(x==y)continue;swapbits(x,y);int next=objective(),delta=next-current;double temp=20*pow(.025,double(it)/max(1,steps));
  if(delta<=0||double(rng()>>11)*0x1p-53<exp(-delta/temp)){current=next;if(current<best){best=current;bestA=A;bestcols=cols;bestflip=flip;}}else swapbits(x,y);
 }
 A=move(bestA);cols=move(bestcols);flip=move(bestflip);for(int k=0;k<K;k++)if(representative[k]>=0)papers[k].color=A[representative[k]]&mask[papers[k].len];
 cerr<<"Column directobjective="<<double(initial)/P<<"->"<<double(best)/P<<" steps="<<steps<<" weight="<<residualweight<<"\n";
}

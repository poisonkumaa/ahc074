#pragma once
static int round6_flip_passes=2,round6_flip_weight=1,round6_flip_repetitions=8;
inline void round6_optimize_flips(vector<U64>&A,vector<int>&flip,vector<PrefixPaper>&papers,int passes,int residualweight){
 const int P=A.size(),K=papers.size(),INF=1000;vector<int>representative(K,-1),weight(P,residualweight);vector<vector<int>>level(65);vector<array<uint8_t,192>>start(P);
 for(int k=0;k<K;k++){int l=papers[k].len;if(papers[k].color==PrefixDP::low(l-1)||(l==64&&papers[k].color==~0ULL))continue;for(int v=0;v<P;v++)if((A[v]&PrefixDP::low(l))==papers[k].color){representative[k]=v;weight[v]=1;break;}assert(representative[k]>=0);level[l].push_back(k);for(int v=0;v<P;v++){U64 diff=(A[v]^papers[k].color)&PrefixDP::low(l);start[v][k]=diff?64-__builtin_clzll(diff):0;}}
 auto objective=[&](){int total=0;for(int v=0;v<P;v++){int f[65],minimum=0;f[0]=0;for(int l=1;l<=64;l++){bool zero=!((A[v]>>(l-1))&1);f[l]=zero?minimum+1:INF;if(l==64)f[l]=min(f[l],minimum+1);for(int k:level[l]){int best=INF;for(int j=start[v][k];j<l;j++)best=min(best,f[j]);f[l]=min(f[l],best+1);}if(zero)minimum=f[l];else minimum=min(minimum,f[l]);}total+=weight[v]*f[64];}return total;};
 int current=objective(),initial=current,accepted=0;mt19937 rng(0x8b71a32);vector<int>columns(64);iota(columns.begin(),columns.end(),0);
 for(int pass=0;pass<passes;pass++){shuffle(columns.begin(),columns.end(),rng);int changes=0;for(int x:columns){for(U64&v:A)v^=1ULL<<x;int next=objective();if(next<current){current=next;flip[x]^=1;accepted++;changes++;}else for(U64&v:A)v^=1ULL<<x;}if(!changes)break;}
 for(int k=0;k<K;k++)if(representative[k]>=0)papers[k].color=A[representative[k]]&PrefixDP::low(papers[k].len);
 cerr<<"Flip objective="<<double(initial)/P<<"->"<<double(current)/P<<" accepted="<<accepted<<" weight="<<residualweight<<"\n";
}

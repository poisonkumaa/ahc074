#pragma once
inline void joint_adjacent_intrinsic(const vector<U64>&A,vector<PrefixPaper>&papers,int passes,int samples){
 const int N=64,P=A.size(),K=papers.size(),INF=255;
 vector<vector<int>>level(N+1);for(int k=0;k<K;k++)level[papers[k].len].push_back(k);
 vector<vector<uint8_t>>start(P,vector<uint8_t>(K));
 auto update=[&](int k){for(int v=0;v<P;v++){U64 d=(A[v]^papers[k].color)&PrefixDP::low(papers[k].len);start[v][k]=d?64-__builtin_clzll(d):0;}};for(int k=0;k<K;k++)update(k);
 auto evaluate=[&](){int total=0;for(int v=0;v<P;v++){int d[65];fill(d,d+65,INF);d[0]=0;for(int l=1;l<=N;l++)for(int k:level[l])for(int j=start[v][k];j<l;j++)d[l]=min(d[l],d[j]+1);if(d[N]>=INF)return 1000000000;total+=d[N];}return total;};
 int initial=evaluate(),accepted=0;auto began=clock();mt19937 rng(431217);vector<int>sample(P);iota(sample.begin(),sample.end(),0);shuffle(sample.begin(),sample.end(),rng);samples=min(samples,P);
 for(int pass=0;pass<passes;pass++)for(int l=63;l>=1;l--){
  if(level[l].size()!=1||level[l+1].size()!=1)continue;
  int lower=level[l][0],upper=level[l+1][0];
  vector<U64>lc{papers[lower].color},uc{papers[upper].color};for(int i=0;i<samples;i++){lc.push_back(A[sample[i]]&PrefixDP::low(l));uc.push_back(A[sample[i]]&PrefixDP::low(l+1));}
  sort(lc.begin(),lc.end());lc.erase(unique(lc.begin(),lc.end()),lc.end());sort(uc.begin(),uc.end());uc.erase(unique(uc.begin(),uc.end()),uc.end());
  const int L=lc.size(),U=uc.size();vector<array<uint8_t,256>>route_l(L),route_both(L),route_u(U);array<uint8_t,256>bypass{};
  for(int v=0;v<P;v++){
   int F[65],B[65];fill(F,F+65,INF);fill(B,B+65,INF);F[0]=0;B[N]=0;
   for(int len=1;len<=N;len++)for(int k:level[len])if(k!=lower&&k!=upper)for(int j=start[v][k];j<len;j++)F[len]=min(F[len],F[j]+1);
   for(int len=N;len>=1;len--)for(int k:level[len])if(k!=lower&&k!=upper)for(int j=start[v][k];j<len;j++)B[j]=min(B[j],B[len]+1);
   bypass[v]=min(INF,F[N]);int fl[65],fu[65];fill(fl,fl+65,INF);fill(fu,fu+65,INF);
   for(int j=l-1;j>=0;j--)fl[j]=min(F[j],fl[j+1]);for(int j=l;j>=0;j--)fu[j]=min(F[j],fu[j+1]);
   for(int c=0;c<L;c++){U64 d=(A[v]^lc[c])&PrefixDP::low(l);int s=d?64-__builtin_clzll(d):0;route_l[c][v]=min(INF,fl[s]+1+B[l]);route_both[c][v]=min(INF,fl[s]+2+B[l+1]);}
   for(int c=0;c<U;c++){U64 d=(A[v]^uc[c])&PrefixDP::low(l+1);int s=d?64-__builtin_clzll(d):0;route_u[c][v]=min(INF,fu[s]+1+B[l+1]);}
  }
  int old=evaluate(),best=old,bl=-1,bu=-1;array<uint8_t,256>base[2];
  for(int x=0;x<L;x++){
   for(int v=0;v<P;v++){int a=min(int(bypass[v]),int(route_l[x][v]));base[0][v]=base[1][v]=a;int bit=A[v]>>l&1;base[bit][v]=min(a,int(route_both[x][v]));}
   for(int y=0;y<U;y++){
    int value=0,bit=uc[y]>>l&1;for(int v=0;v<P;v++){int q=min(int(base[bit][v]),int(route_u[y][v]));if(q==INF){value=1000000000;break;}value+=q;if(value>=best)break;}
    if(value<best){best=value;bl=x;bu=y;}
   }
  }
  if(bl>=0){papers[lower].color=lc[bl];papers[upper].color=uc[bu];update(lower);update(upper);accepted++;int check=evaluate();assert(check==best);cerr<<"JointChange "<<l<<" "<<old<<" -> "<<best<<"\n";}
 }
 cerr<<"JointIntrinsic "<<double(initial)/P<<" -> "<<double(evaluate())/P<<" accepted="<<accepted<<" CPU="<<double(clock()-began)/CLOCKS_PER_SEC<<"\n";
}

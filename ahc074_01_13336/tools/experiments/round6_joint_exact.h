#pragma once
inline void joint_adjacent_exact(const vector<U64>&A,vector<PrefixPaper>&papers,int samples,int repetitions,int high=43,int low=1){
 const int N=64,P=A.size(),K=papers.size(),INF=255,M=P*repetitions;
 const auto began=clock();vector<int>length;vector<vector<int>>level(65);for(int k=0;k<K;k++){length.push_back(papers[k].len);level[length[k]].push_back(k);}
 struct Scenario{int target;array<uint8_t,192>pos;array<array<uint8_t,193>,65>cnt;array<vector<uint16_t>,65>F;array<array<uint8_t,193>,65>B;};
 vector<Scenario>scenarios(M);mt19937 rng(4123678);
 {
  PrefixDP dp(N,papers);dp.public_data(A);auto future=selection_teacher(A,papers);vector<int>order(K),targets(P);iota(order.begin(),order.end(),0);iota(targets.begin(),targets.end(),0);shuffle(targets.begin(),targets.end(),rng);
  for(int r=0;r<128+M;r++){
   if(r>128&&(r-128)%P==0)shuffle(targets.begin(),targets.end(),rng);int v=r<128?rng()%P:targets[(r-128)%P];
   if(r>=128){auto&s=scenarios[r-128];s.target=v;for(int i=0;i<K;i++)s.pos[order[i]]=i;for(int l=0;l<=N;l++){s.cnt[l][0]=0;for(int i=0;i<K;i++)s.cnt[l][i+1]=s.cnt[l][i]+(length[order[i]]>=l);}}
   for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);int distance;auto chosen=dp.solve(order,A[v],distance);vector<char>selected(K);for(int k:chosen)selected[k]=true;order=direct_final(order,length,selected,&future);U64 bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
  }
 }
 vector<array<int,192>>start(P);auto update=[&](int z){for(int v=0;v<P;v++){U64 d=(A[v]^papers[z].color)&PrefixDP::low(length[z]);start[v][z]=d?64-__builtin_clzll(d):0;}};for(int z=0;z<K;z++)update(z);
 auto add=[&](const Scenario&s,int j,int h,int z){int q=s.pos[z],l=length[z];if(q<h)return 0;int m=s.cnt[l][q]-s.cnt[l][h],u=s.cnt[j+1][q]-s.cnt[j+1][h];return m+(u>m);};
 for(auto&s:scenarios){
  int v=s.target;uint8_t F[65][193];memset(F,INF,sizeof(F));F[0][0]=0;
  for(int j=0;j<N;j++){
   for(int h=0;h<=K;h++)if(F[j][h]<INF)s.F[j].push_back((h<<8)|F[j][h]);
   for(int z=0;z<K;z++)if(start[v][z]<=j&&length[z]>j){int l=length[z],q=s.pos[z];for(int packed:s.F[j]){int h=packed>>8,f=packed&255,nh=max(q+1,h),value=f+add(s,j,h,z);assert(value<=K);if(value<F[l][nh])F[l][nh]=value;}}
  }
  s.B[N].fill(0);
 }
 auto backward=[&](Scenario&s,int j,int skip, array<uint8_t,193>&out){
  out.fill(INF);int v=s.target;
  for(int l=j+1;l<=N;l++){
   int at[193];fill(at,at+K+1,INF*2);int first=K+1;
   for(int z:level[l])if(z!=skip&&start[v][z]<=j){int q=s.pos[z];first=min(first,q);at[q]=s.cnt[l][q]+s.B[l][q+1];}
   if(first>K)continue;int same=INF*2,far=INF*2,last=-1;
   for(int h=K;h>=0;h--){int d=s.cnt[j+1][h]-s.cnt[l][h];if(d!=last){far=min(far,same);same=INF*2;last=d;}same=min(same,at[h]);int value=min(same,far+1)-s.cnt[l][h];if(first<h)value=min(value,int(s.B[l][h]));if(value<out[h])out[h]=value;}
  }
 };
 vector<int>sample(P);iota(sample.begin(),sample.end(),0);shuffle(sample.begin(),sample.end(),rng);samples=min(samples,P);long long total_gain=0;int accepted=0;
 for(int l=N-1;l>=low;l--){
  if(l+1<N)for(auto&s:scenarios)backward(s,l+1,-1,s.B[l+1]);
  if(l>high||level[l].size()!=1||level[l+1].size()!=1)continue;
  int z1=level[l][0],z2=level[l+1][0];vector<U64>lc{papers[z1].color&PrefixDP::low(l)},uc{papers[z2].color&PrefixDP::low(l+1)};for(int i=0;i<samples;i++){lc.push_back(A[sample[i]]&PrefixDP::low(l));uc.push_back(A[sample[i]]&PrefixDP::low(l+1));}
  sort(lc.begin(),lc.end());lc.erase(unique(lc.begin(),lc.end()),lc.end());sort(uc.begin(),uc.end());uc.erase(unique(uc.begin(),uc.end()),uc.end());int L=lc.size(),U=uc.size();
  vector<vector<uint8_t>>rl(L,vector<uint8_t>(M)),ru(U,vector<uint8_t>(M)),rb(L,vector<uint8_t>(M));vector<uint8_t>bypass(M);long long oldcost=0;
  for(int e=0;e<M;e++){
   auto&s=scenarios[e];int v=s.target,q1=s.pos[z1],q2=s.pos[z2];array<uint8_t,193>Bskip;backward(s,l,z2,Bskip);
   int base=INF;
   for(int z=0;z<K;z++)if(length[z]>l+1&&start[v][z]<l){int q=s.pos[z],len=length[z];for(int j=start[v][z];j<l;j++)for(int packed:s.F[j]){int h=packed>>8,f=packed&255;base=min(base,f+add(s,j,h,z)+s.B[len][max(q+1,h)]);}}
   bypass[e]=base;int al[65],au[65],ab[65];fill(al,al+65,INF);fill(au,au+65,INF);fill(ab,ab+65,INF);
   for(int j=l-1;j>=0;j--){
    al[j]=al[j+1];au[j]=au[j+1];ab[j]=ab[j+1];
    for(int packed:s.F[j]){int h=packed>>8,f=packed&255,h1=max(h,q1+1),h2=max(h,q2+1),first=f+add(s,j,h,z1);al[j]=min(al[j],first+Bskip[h1]);au[j]=min(au[j],f+add(s,j,h,z2)+s.B[l+1][h2]);ab[j]=min(ab[j],first+add(s,l,h1,z2)+s.B[l+1][max(h1,q2+1)]);}
   }
   for(int c=0;c<L;c++){U64 d=(A[v]^lc[c])&PrefixDP::low(l);int st=d?64-__builtin_clzll(d):0;rl[c][e]=al[st];rb[c][e]=ab[st];}
   for(int c=0;c<U;c++){U64 d=(A[v]^uc[c])&PrefixDP::low(l+1);int st=d?64-__builtin_clzll(d):0;ru[c][e]=au[st];}
   U64 d1=(A[v]^papers[z1].color)&PrefixDP::low(l),d2=(A[v]^papers[z2].color)&PrefixDP::low(l+1);int st1=d1?64-__builtin_clzll(d1):0,st2=d2?64-__builtin_clzll(d2):0;int old=min(base,min(al[st1],au[st2]));if(!((A[v]^papers[z2].color)>>l&1))old=min(old,ab[st1]);assert(old<INF);oldcost+=old;
  }
  long long best=oldcost;int bl=-1,bu=-1;vector<uint8_t>base0(M),base1(M);
  for(int x=0;x<L;x++){
   for(int e=0;e<M;e++){int c=min(int(bypass[e]),int(rl[x][e]));base0[e]=base1[e]=c;if(A[scenarios[e].target]>>l&1)base1[e]=min(c,int(rb[x][e]));else base0[e]=min(c,int(rb[x][e]));}
   for(int y=0;y<U;y++){const auto&base=(uc[y]>>l&1)?base1:base0;long long score=0;for(int e=0;e<M;e++){int c=min(int(base[e]),int(ru[y][e]));if(c==INF){score=1000000000;break;}score+=c;if(score>=best)break;}if(score<best){best=score;bl=x;bu=y;}}
  }
  if(bl>=0){papers[z1].color=lc[bl];papers[z2].color=uc[bu];update(z1);update(z2);accepted++;total_gain+=oldcost-best;cerr<<"JointExactChange "<<l<<" "<<oldcost<<" -> "<<best<<"\n";
#ifdef JOINT_EXACT_VERIFY
   PrefixDP check(N,papers);check.public_data(A);long long total=0;for(auto&s:scenarios){vector<int>order(K);for(int k=0;k<K;k++)order[s.pos[k]]=k;int moves;check.solve(order,A[s.target],moves,false);total+=moves;}if(total!=best){cerr<<"JOINT CHECK FAILED "<<total<<" "<<best<<"\n";abort();}
#endif
  }
 }
 cerr<<"JointExact accepted="<<accepted<<" gain="<<double(total_gain)/M<<" CPU="<<double(clock()-began)/CLOCKS_PER_SEC<<"\n";
}

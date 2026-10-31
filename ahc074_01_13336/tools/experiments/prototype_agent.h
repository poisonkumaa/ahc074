#pragma once
struct PrototypeDesigner{
 int P,K,N=64,INF=10000,current=0;const vector<U64>&A;vector<PrefixPaper>&papers;
 vector<vector<int>>level,start;vector<array<int,65>>F,B,bypass,first,second,owner;
 PrototypeDesigner(const vector<U64>&aa,vector<PrefixPaper>&pp):P(aa.size()),K(pp.size()),A(aa),papers(pp),level(65),start(P,vector<int>(K)),F(P),B(P),bypass(P),first(P),second(P),owner(P){for(int k=0;k<K;k++)level[papers[k].len].push_back(k);rebuild();}
 void rebuild(){current=0;for(int v=0;v<P;v++){
   for(int k=0;k<K;k++){U64 d=(A[v]^papers[k].color)&PrefixDP::low(papers[k].len);start[v][k]=d?64-__builtin_clzll(d):0;}
   auto&f=F[v];auto&b=B[v];f.fill(INF);b.fill(INF);f[0]=0;b[N]=0;
   for(int l=1;l<=N;l++)for(int k:level[l])for(int j=start[v][k];j<l;j++)f[l]=min(f[l],f[j]+1);
   for(int l=N;l>0;l--)for(int k:level[l])for(int j=start[v][k];j<l;j++)b[j]=min(b[j],b[l]+1);
   current+=f[N];bypass[v].fill(INF);first[v].fill(INF);second[v].fill(INF);owner[v].fill(-1);
   for(int k=0;k<K;k++){
    int l=papers[k].len,st=start[v][k],best=INF;
    for(int j=st;j<l;j++){best=min(best,f[j]);if(j+1<l)bypass[v][j+1]=min(bypass[v][j+1],best+1+b[l]);}
    int cost=best+1+b[l];if(cost<first[v][l]){second[v][l]=first[v][l];first[v][l]=cost;owner[v][l]=k;}else second[v][l]=min(second[v][l],cost);
   }
  }
 }
 int evaluate(int k,U64 color){int l=papers[k].len,total=0;for(int v=0;v<P;v++){
   int best=min(bypass[v][l],owner[v][l]==k?second[v][l]:first[v][l]);
   U64 d=(A[v]^color)&PrefixDP::low(l);int st=d?64-__builtin_clzll(d):0,pre=INF;for(int j=st;j<l;j++)pre=min(pre,F[v][j]);
   best=min(best,pre+1+B[v][l]);total+=best;
  }return total;}
 void optimize(int iterations,int low,double temperature,double seconds,int focus){
  int initial=current,best=current,accepted=0,changed=0;auto bestpapers=papers;mt19937_64 rng(372315);vector<int>eligible;for(int k=0;k<K;k++)if(papers[k].len>=low)eligible.push_back(k);
  double started=double(clock()-structure2_training_origin)/CLOCKS_PER_SEC;
  for(int it=0;it<iterations;it++){
   double elapsed=double(clock()-structure2_training_origin)/CLOCKS_PER_SEC;if(elapsed>seconds)break;
   int k=eligible[rng()%eligible.size()],v=rng()%P;
   for(int t=0;t<focus;t++){int q=rng()%P;if(F[q][N]>F[v][N])v=q;}
   U64 color=A[v]&PrefixDP::low(papers[k].len);if(color==papers[k].color)continue;
   int next=evaluate(k,color);double progress=max(double(it)/iterations,(elapsed-started)/max(1e-6,seconds-started));double temp=temperature*pow(.002,progress);
   if(next<INF&&(next<=current||double(rng()>>11)*0x1p-53<exp((current-next)/temp))){
    papers[k].color=color;accepted++;rebuild();if(current!=next){cerr<<"MISMATCH "<<current<<" "<<next<<endl;abort();}
    if(current<best){best=current;bestpapers=papers;changed++;}
   }
  }
  papers=bestpapers;rebuild();cerr<<"Prototype "<<double(initial)/P<<" -> "<<double(best)/P<<" accepted="<<accepted<<" improvements="<<changed<<" CPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
 }
};

#pragma once
inline void joint_backbone_columns(std::vector<uint64_t>&targets,std::vector<PrefixPaper>&papers,
                          std::vector<int>&cols,std::vector<int>&flip,int steps,double temperature=2.0){
 const int n=cols.size(),P=targets.size(),K=papers.size();
 std::vector<uint64_t>full(K);std::vector<char>fixed(K);
 for(int k=0;k<K;k++)fixed[k]=papers[k].color==PrefixDP::low(papers[k].len-1)||(papers[k].len==64&&papers[k].color==~0ULL);
 for(int k=0;k<K;k++){
  int best=999,bv=0;uint64_t mask=PrefixDP::low(papers[k].len);
  for(int v=0;v<P;v++){int d=__builtin_popcountll((papers[k].color^targets[v])&mask);if(d<best)best=d,bv=v;}
  full[k]=(targets[bv]&~mask)|papers[k].color;
 }
 auto evaluate=[&](){
  int score=0;
  for(uint64_t target:targets){
   int f[65];std::fill(f,f+n+1,1000);f[0]=0;
   for(int k=0;k<K;k++){
    int l=papers[k].len;uint64_t diff=(target^full[k])&PrefixDP::low(l);
    int start=diff?64-__builtin_clzll(diff):0;
    int best=1000;for(int j=start;j<l;j++)best=std::min(best,f[j]);f[l]=std::min(f[l],best+1);
   }
   if(f[n]>=1000)return 1000000;score+=f[n];
  }
  return score;
 };
 std::mt19937_64 rnd(0x9271957ULL);int score=evaluate(),best=score,initial=score,accepted=0;
 auto bt=targets,bp=full;auto bc=cols,bf=flip;
 auto swapcols=[&](int x,int y){
  uint64_t mask=(1ULL<<x)|(1ULL<<y);
  for(auto&v:targets)if(((v>>x)^(v>>y))&1)v^=mask;
  for(int k=0;k<K;k++)if(!fixed[k]&&(((full[k]>>x)^(full[k]>>y))&1))full[k]^=mask;
  std::swap(cols[x],cols[y]);std::swap(flip[x],flip[y]);
 };
 for(int it=0;it<steps;it++){
  int x=rnd()%n,y=rnd()%n;if(rnd()%2)y=(x+1)%n;if(x==y)continue;
  bool invert=rnd()%4==0;
  auto inv=[&](){for(auto&v:targets)v^=1ULL<<x;for(int k=0;k<K;k++)if(!fixed[k])full[k]^=1ULL<<x;flip[x]^=1;};
  if(invert)inv();else swapcols(x,y);int value=evaluate();double temp=temperature*std::pow(.005,double(it)/steps);
  if(value<=score||double(rnd()>>11)*0x1p-53<std::exp((score-value)/temp)){
   score=value;accepted++;if(score<best){best=score;bt=targets;bp=full;bc=cols;bf=flip;}
  }else if(invert)inv();else swapcols(x,y);
 }
 targets=bt;full=bp;cols=bc;flip=bf;
 for(int k=0;k<K;k++)papers[k].color=full[k]&PrefixDP::low(papers[k].len);
 std::cerr<<"column_joint "<<double(initial)/P<<" -> "<<double(best)/P<<" accepted="<<accepted<<" CPU="<<double(clock())/CLOCKS_PER_SEC<<"\n";
}

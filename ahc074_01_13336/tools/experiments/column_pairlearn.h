#pragma once
inline void pair_learn_columns(std::vector<uint64_t>&targets,std::vector<PrefixPaper>&papers,
                               std::vector<int>&cols,std::vector<int>&flip,int iterations,int weightmode=0){
 const int n=cols.size(),P=targets.size();std::mt19937_64 rng(0x18824552ULL);
 auto intrinsic=[&](std::vector<std::vector<int>>* paths=nullptr){
  int score=0;if(paths)paths->assign(P,{});
  for(int v=0;v<P;v++){
   int f[65],prev[65],chosen[65];std::fill(f,f+n+1,1000);f[0]=0;
   for(int k=0;k<int(papers.size());k++){
    int l=papers[k].len;uint64_t d=(targets[v]^papers[k].color)&PrefixDP::low(l);int start=d?64-__builtin_clzll(d):0;
    for(int j=start;j<l;j++)if(f[j]+1<f[l])f[l]=f[j]+1,prev[l]=j,chosen[l]=k;
   }
   score+=f[n];if(paths)for(int l=n;l;l=prev[l])(*paths)[v].push_back(chosen[l]);
  }return score;
 };
 int best=intrinsic();auto bt=targets;auto bp=papers;auto bc=cols,bf=flip;
 std::cerr<<"joint_start="<<double(best)/P<<"\n";
 for(int iter=0;iter<iterations;iter++){
  std::vector<std::vector<int>>path;intrinsic(&path);std::vector<uint64_t>full(papers.size());
  for(int k=0;k<int(papers.size());k++){
   int bd=999,bv=0;auto mask=PrefixDP::low(papers[k].len);
   for(int v=0;v<P;v++){int d=__builtin_popcountll((papers[k].color^targets[v])&mask);if(d<bd)bd=d,bv=v;}
   full[k]=(targets[bv]&~mask)|papers[k].color;
  }
  int edge[64][64]{};
  for(int v=0;v<P;v++)for(int k:path[v]){
   uint64_t d=targets[v]^full[k];int weight=weightmode?std::max(1,64-papers[k].len):1;
   for(int x=0;x<n;x++)for(int y=0;y<x;y++)if(((d>>x)^(d>>y))&1)edge[x][y]+=weight,edge[y][x]+=weight;
  }
  std::vector<int>permutation(n);std::iota(permutation.begin(),permutation.end(),0);int score=0;for(int j=1;j<n;j++)score+=edge[j-1][j];
  int lowest=score;auto bestperm=permutation;const int STEPS=100000;
  for(int it=0;it<STEPS;it++){
   int x=rng()%n,y=rng()%n;if(x>y)std::swap(x,y);if(x==y)continue;
   int delta=0;if(x)delta+=edge[permutation[x-1]][permutation[y]]-edge[permutation[x-1]][permutation[x]];
   if(y+1<n)delta+=edge[permutation[x]][permutation[y+1]]-edge[permutation[y]][permutation[y+1]];
   double temp=30*std::pow(.005,double(it)/STEPS)*(weightmode?16:1);
   if(delta<=0||double(rng()>>11)*0x1p-53<std::exp(-delta/temp)){
    std::reverse(permutation.begin()+x,permutation.begin()+y+1);score+=delta;if(score<lowest)lowest=score,bestperm=permutation;
   }
  }
  auto transform=[&](uint64_t v){uint64_t result=0;for(int j=0;j<n;j++)result|=((v>>bestperm[j])&1)<<j;return result;};
  for(auto&v:targets)v=transform(v);for(auto&v:full)v=transform(v);
  auto oldcols=cols,oldflip=flip;for(int j=0;j<n;j++)cols[j]=oldcols[bestperm[j]],flip[j]=oldflip[bestperm[j]];
  int count[65][2]{},first[65];std::fill(first,first+65,-1);
  for(int k=0;k<int(papers.size());k++){int l=papers[k].len;papers[k].color=full[k]&PrefixDP::low(l);count[l][papers[k].color>>(l-1)&1]++;first[l]=k;}
  for(int l=1;l<=n;l++)if(count[l][0]+count[l][1]>=2&&(!count[l][0]||!count[l][1]))papers[first[l]].color^=1ULL<<(l-1);
  train_prefix_dictionary(targets,papers,n,64,2,8,0,6,0);
  int val=intrinsic();std::cerr<<"joint_iter="<<iter<<" intrinsic="<<double(val)/P<<" CPU="<<double(clock())/CLOCKS_PER_SEC<<"\n";
  if(val<best)best=val,bt=targets,bp=papers,bc=cols,bf=flip;
 }
 targets=bt;papers=bp;cols=bc;flip=bf;
}

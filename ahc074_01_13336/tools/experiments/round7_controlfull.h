#pragma once
namespace Round7ControlFull {
double beta=1;
struct Bits {
 std::array<uint64_t,6> x{};
 Bits&operator|=(const Bits&b){for(int i=0;i<6;i++)x[i]|=b.x[i];return *this;}
 friend Bits operator|(Bits a,const Bits&b){return a|=b;}
 friend Bits operator-(Bits a,const Bits&b){for(int i=0;i<6;i++)a.x[i]&=~b.x[i];return a;}
 double score()const {int exact=0;for(int i=2;i<6;i++)exact+=__builtin_popcountll(x[i]);return 4.0*__builtin_popcountll(x[0])+beta*(exact-4*__builtin_popcountll(x[1]));}
};
std::vector<Bits>reference;
void init(const std::vector<U64>&A,const std::vector<PrefixPaper>&papers){
 const int P=A.size(),K=papers.size(),N=64;
 reference.assign(K,{});int total=0;
 for(int v=0;v<P;v++){
  int distance[65],previous[65],which[65];std::fill(distance,distance+65,1000000);distance[0]=0;
  for(int j=0;j<N;j++)if(distance[j]<1000000)for(int k=0;k<K;k++){
   int l=papers[k].len;if(l<=j||((A[v]^papers[k].color)&(PrefixDP::low(l)^PrefixDP::low(j))))continue;
   if(distance[j]+1<distance[l]){distance[l]=distance[j]+1;previous[l]=j;which[l]=k;}
  }
  assert(distance[N]<1000000);total+=distance[N];
  for(int j=N;j;){int k=which[j];reference[k].x[2+v/64]|=1ULL<<(v%64);j=previous[j];}
 }
 std::vector<int>targets(P);std::iota(targets.begin(),targets.end(),0);std::mt19937 rng(912367);std::shuffle(targets.begin(),targets.end(),rng);
 for(int r=0;r<64;r++)for(int k=0;k<K;k++)if(reference[k].x[2+targets[r]/64]>>(targets[r]%64)&1)reference[k].x[1]|=1ULL<<r;
 std::cerr<<"ControlIntrinsic="<<double(total)/P<<"\n";
}
void observe(int product,int r){
 U64 bit=1ULL<<(r%64);for(auto&v:reference)v.x[1]=(v.x[1]&~bit)|((v.x[2+product/64]>>(product%64)&1)?bit:0ULL);
}
std::vector<int>final_order(const std::vector<int>&order,const std::vector<int>&length,const std::vector<char>&selected,const std::vector<U64>&future){
 const int K=order.size(),N=*std::max_element(length.begin(),length.end());int owner[65],root=-1;std::fill(owner,owner+65,-1);
 for(int k=0;k<K;k++)if(selected[k])owner[length[k]]=k;
 for(int l=N;l>=0;l--){if(owner[l]>=0)root=owner[l];owner[l]=root;}
 std::vector<char>mandatory(K);int suffix=N+1;for(int i=K-1;i>=0;i--){int k=order[i];mandatory[k]=length[k]>=suffix;if(selected[k])suffix=std::min(suffix,length[k]);}
 std::vector<std::vector<int>>children(K);std::vector<int>insert;
 for(int k:order){if(selected[k])continue;if(mandatory[k])insert.push_back(k);else children[owner[length[k]]].push_back(k);}
 std::stable_sort(insert.begin(),insert.end(),[&](int a,int b){return length[a]<length[b];});
 Bits access[192],below[192];int inversions[192]{};
 for(int k=0;k<K;k++){access[k]=reference[k];access[k].x[0]=future[k];}
 for(auto&v:children)for(int i=int(v.size())-1;i>=0;i--){int x=v[i];for(int j=i+1;j<int(v.size());j++){int y=v[j];if(length[y]<length[x])inversions[x]++;if(length[y]<=length[x])below[x]|=access[y];}}
 for(int k:insert){
  auto&v=children[owner[length[k]]];int M=v.size(),kl=length[k];Bits remain[193];int lower_after[193]{};
  for(int i=M-1;i>=0;i--){int x=v[i];remain[i]=remain[i+1];if(length[x]<=kl)remain[i]|=access[x];lower_after[i]=lower_after[i+1]+(length[x]<kl);}
  double gain=0,best=1e100;int larger=0,bestinv=INT_MAX,at=0;
#ifdef CONTROLFULL_CHECK
  auto quality=[&](const std::vector<int>&a){double q=0;for(int i=0;i<int(a.size());i++){Bits b;for(int j=i+1;j<int(a.size());j++)if(length[a[j]]<=length[a[i]])b|=access[a[j]];q+=b.score();}return q;};
  auto first=v;first.insert(first.begin(),k);double reference_quality=quality(first),reference_delta=remain[0].score();
#endif
  for(int i=0;i<=M;i++){
   double q=gain+remain[i].score();int inv=larger+lower_after[i];
#ifdef CONTROLFULL_CHECK
   auto test=v;test.insert(test.begin()+i,k);assert(std::abs((quality(test)-reference_quality)-(q-reference_delta))<1e-7);
#endif
   if(q<best-1e-10||(std::abs(q-best)<=1e-10&&inv<bestinv)){best=q;bestinv=inv;at=i;}
   if(i<M){int x=v[i];if(length[x]>kl)larger++;if(length[x]>=kl)gain+=(access[k]-below[x]).score();}
  }
  below[k]=remain[at];inversions[k]=lower_after[at];for(int i=0;i<at;i++){int x=v[i];if(length[x]>=kl)below[x]|=access[k];if(length[x]>kl)inversions[x]++;}
  v.insert(v.begin()+at,k);
 }
 std::vector<int>out;out.reserve(K);for(int l=1;l<=N;l++){int k=owner[l];if(k>=0&&selected[k]&&length[k]==l){out.push_back(k);for(int x:children[k])out.push_back(x);}}return out;
}
}

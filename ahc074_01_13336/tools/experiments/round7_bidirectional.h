#pragma once
struct BidirectionalPaper {int len;bool right;U64 color;};
inline U64 reverse_bits(U64 x){x=((x&0x5555555555555555ULL)<<1)|((x>>1)&0x5555555555555555ULL);x=((x&0x3333333333333333ULL)<<2)|((x>>2)&0x3333333333333333ULL);x=((x&0x0f0f0f0f0f0f0f0fULL)<<4)|((x>>4)&0x0f0f0f0f0f0f0f0fULL);x=__builtin_bswap64(x);return x;}
struct BidirectionalDistance {
 static constexpr int INF=255;
 array<uint8_t,65>left,right;array<array<uint8_t,65>,65>back;
 vector<uint8_t>prefix[65],suffix[65];
 void build(const vector<BidirectionalPaper>&papers,U64 target,int exclude=-1,bool reverse=false){
  int maxprefix[65]{},minsuffix[65];fill(minsuffix,minsuffix+65,64);
  for(int j=0;j<=64;j++){prefix[j].clear();suffix[j].clear();back[j].fill(INF);back[j][j]=0;}
  for(int k=0;k<int(papers.size());k++)if(k!=exclude){
   auto p=papers[k];bool direction=p.right^reverse;U64 color=reverse?reverse_bits(p.color):p.color;
   if(!direction){
    for(int l=0;l<p.len;l++){U64 d=(target^color)&PrefixDP::low(p.len)&~PrefixDP::low(l);int reach=d?__builtin_ctzll(d):p.len;maxprefix[l]=max(maxprefix[l],reach);if(!d)prefix[l].push_back(p.len);}
   }else{
    int start=64-p.len;for(int r=start+1;r<=64;r++){U64 d=(target^color)&PrefixDP::low(r)&~PrefixDP::low(start);int reach=d?64-__builtin_clzll(d):start;minsuffix[r]=min(minsuffix[r],reach);if(!d)suffix[r].push_back(start);}
   }
  }
  left.fill(INF);right.fill(INF);left[0]=0;right[64]=0;
  for(int l=0;l<64;l++)for(int r:prefix[l])left[r]=min(int(left[r]),int(left[l])+1);
  for(int r=64;r>0;r--)for(int l:suffix[r])right[l]=min(int(right[l]),int(right[r])+1);
  for(int width=1;width<=64;width++)for(int l=0;l+width<=64;l++){
   int r=l+width;if(maxprefix[l]>=r||minsuffix[r]<=l){back[l][r]=1;continue;}
   int best=INF;for(int end:prefix[l])if(end<r)best=min(best,int(back[end][r])+1);for(int start:suffix[r])if(start>l)best=min(best,int(back[l][start])+1);back[l][r]=best;
  }
 }
};
inline double bidirectional_average(const vector<U64>&A,const vector<BidirectionalPaper>&papers){BidirectionalDistance d;int total=0;for(U64 a:A){d.build(papers,a);if(d.back[0][64]==255)return 1e9;total+=d.back[0][64];}return double(total)/A.size();}
inline void train_bidirectional(const vector<U64>&A,vector<BidirectionalPaper>&papers,int passes,int candidates){
 const int P=A.size(),K=papers.size(),INF=255;auto began=clock();double initial=bidirectional_average(A,papers);mt19937 rng(312371);vector<int>ids(K),sample(P);iota(ids.begin(),ids.end(),0);iota(sample.begin(),sample.end(),0);shuffle(sample.begin(),sample.end(),rng);stable_sort(ids.begin(),ids.end(),[&](int x,int y){return papers[x].len>papers[y].len;});candidates=min(candidates,P);
 vector<U64>reverseA;for(U64 a:A)reverseA.push_back(reverse_bits(a));int accepted=0;
 struct Route{array<uint8_t,65>left,right;int tail,bypass;};vector<Route>routes(P);BidirectionalDistance dp;
 for(int pass=0;pass<passes;pass++)for(int z:ids){
  auto paper=papers[z];int len=paper.len;const auto&data=paper.right?reverseA:A;
  for(int v=0;v<P;v++){
   dp.build(papers,data[v],z,paper.right);auto&r=routes[v];r.left=dp.left;r.right=dp.right;r.bypass=dp.back[0][64];r.tail=INF;
   for(int end=len+1;end<=64;end++)r.tail=min(r.tail,int(dp.right[end])+dp.back[len][end]+1);
  }
  auto cost=[&](U64 color,int bound){int sum=0;for(int v=0;v<P;v++){const auto&r=routes[v];int best=r.bypass,prefix=INF;U64 diff=data[v]^color;for(int i=0;i<len;i++)if(diff>>i&1)prefix=INF;else{prefix=min(prefix,int(r.left[i]));best=min(best,prefix+r.right[i+1]+1);}best=min(best,prefix+r.tail);if(best>=INF)return 1000000000;sum+=best;if(sum>=bound)return sum;}return sum;};
  U64 old=paper.right?reverse_bits(paper.color):paper.color,color=old;int before=cost(old,1000000000),best=before;
  for(int i=0;i<candidates;i++){U64 c=data[sample[i]];int value=cost(c,best);if(value<best)best=value,color=c;}
  if(best<before){papers[z].color=paper.right?reverse_bits(color):color;accepted++;
#ifdef BIDIRECTIONAL_CHECK
   int actual=lround(bidirectional_average(A,papers)*P);if(actual!=best){cerr<<"BIDIRECTIONAL ROUTE ERROR "<<z<<" "<<actual<<" "<<best<<"\n";abort();}
#endif
  }
 }
 cerr<<"BiTrain "<<initial<<" -> "<<bidirectional_average(A,papers)<<" accepted="<<accepted<<" CPU="<<double(clock()-began)/CLOCKS_PER_SEC<<"\n";
}

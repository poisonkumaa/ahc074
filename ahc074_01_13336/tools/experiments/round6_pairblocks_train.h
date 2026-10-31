#pragma once
inline void train_pairblocks(PairBlocksDP&dp,const vector<U64>&A,int passes){
 const int P=A.size(),K=dp.K,S=dp.S,INF=1000000;
 auto rebuild=[&](int z){
  int g=dp.group[z],c=dp.category[z];
  for(int v=0;v<P;v++)for(int s=0;s<S;s++){
   auto&out=dp.candidate[v][s];out.erase(remove_if(out.begin(),out.end(),[&](auto e){return e.paper==z;}),out.end());
   auto&t=dp.states[s];if(g<t.group||(g==t.group&&(t.subset>>c&1)))continue;
   if((A[v]^dp.papers[z].color)&dp.mask[z]&~t.covered)continue;
   int ns=dp.state_index[g][g==t.group?(t.subset|(1<<c)):(1<<c)];out.push_back({uint8_t(z),uint8_t(ns)});
  }
 };
 auto average=[&](){long long total=0;for(int v=0;v<P;v++){int f[97];fill(f,f+S,INF);f[0]=0;for(int s=0;s<S;s++)for(auto e:dp.candidate[v][s])f[e.next]=min(f[e.next],f[s]+1);if(f[S-1]>=INF)return 1e9;total+=f[S-1];}return double(total)/P;};
 double initial=average();int accepted=0;clock_t began=clock();
 vector<int>ids(K);iota(ids.begin(),ids.end(),0);stable_sort(ids.begin(),ids.end(),[&](int x,int y){return dp.papers[x].len>dp.papers[y].len;});
 struct Route{array<int,65>suffix;array<int,64>single;int bypass,new_suffix,other;};
 vector<Route>routes(P);
 for(int pass=0;pass<passes;pass++)for(int z:ids){
  int g=dp.group[z],cat=dp.category[z],endpoint=dp.base[g]+cat,newstate=dp.state_index[g][1<<cat];
  long long oldcost=0;
  for(int v=0;v<P;v++){
   int F[97],B[97];fill(F,F+S,INF);fill(B,B+S,INF);F[0]=0;B[S-1]=0;
   for(int s=0;s<S;s++)for(auto e:dp.candidate[v][s])if(e.paper!=z)F[e.next]=min(F[e.next],F[s]+1);
   for(int s=S-1;s>=0;s--)for(auto e:dp.candidate[v][s])if(e.paper!=z)B[s]=min(B[s],B[e.next]+1);
   auto&r=routes[v];r.bypass=B[0];r.new_suffix=B[newstate]+1;r.suffix.fill(INF);r.single.fill(INF);r.other=INF;
   for(int s=0;s<S;s++){
    const auto&t=dp.states[s];if(t.group>=g)break;
    int prefix=0;if(s){prefix=dp.base[t.group];if(t.subset==1)prefix++;else if(t.subset==3)prefix+=2;else r.single[prefix+1]=min(r.single[prefix+1],F[s]);}
    r.suffix[prefix]=min(r.suffix[prefix],F[s]);
   }
   for(int j=63;j>=0;j--)r.suffix[j]=min(r.suffix[j],r.suffix[j+1]);
   if(dp.size[g]==2)r.other=F[dp.state_index[g][1<<(cat^1)]]+1+B[dp.state_index[g][3]];
  }
  auto cost=[&](U64 color,bool early,long long bound){
   long long total=0;
   for(int v=0;v<P;v++){
    auto&r=routes[v];int best=r.bypass;
    if(!((A[v]^color)>>endpoint&1)){
     U64 d=(A[v]^color)&PrefixDP::low(dp.base[g]);int h=d?63-__builtin_clzll(d):-1;
     int f=r.suffix[h+1];if(h>0&&!(d>>(h-1)&1))f=min(f,r.single[h]);
     best=min(best,min(r.other,f+r.new_suffix));
    }
    total+=best;if(early&&total>=bound)return total;
   }
   return total;
  };
  oldcost=cost(dp.papers[z].color,false,LLONG_MAX);long long best=oldcost;U64 color=dp.papers[z].color;
  for(U64 a:A){long long score=cost(a,true,best);if(score<best){best=score;color=a;}}
  if(best<oldcost&&best<INF){dp.papers[z].color=color;rebuild(z);accepted++;}
 }
 cerr<<"PairTrain "<<initial<<" -> "<<average()<<" accepted="<<accepted<<" CPU="<<double(clock()-began)/CLOCKS_PER_SEC<<"\n";
}

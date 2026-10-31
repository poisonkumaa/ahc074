from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'round5_sampling_adaptive.cpp').read_text()
s=s.replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100').replace('#define AHC_REFINEMENT_BUDGET 1.65','#define AHC_REFINEMENT_BUDGET 100')
s=s[:s.index('int main()')]
s+=r'''
struct MaskFit {
 int K;vector<U64>mask,color;vector<vector<int>>canonical;
 MaskFit(const vector<U64>&A,const vector<PrefixPaper>&p):K(p.size()),mask(K),color(K),canonical(A.size()){
  for(int k=0;k<K;k++){mask[k]=PrefixDP::low(p[k].len);color[k]=p[k].color;}
  PrefixDP dp(64,p);dp.public_data(A);vector<int>order(K),length;for(auto x:p)length.push_back(x.len);iota(order.begin(),order.end(),0);
  auto future=selection_teacher(A,p);mt19937 rng(199735);vector<int>targets(A.size());iota(targets.begin(),targets.end(),0);shuffle(targets.begin(),targets.end(),rng);
  for(int r=0;r<128+int(A.size());r++){int t=r<128?rng()%A.size():targets[r-128],moves;for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);auto chosen=dp.solve(order,A[t],moves);vector<char>sel(K);for(int k:chosen)sel[k]=1;order=direct_final(order,length,sel,&future);if(r>=128)canonical[t]=order;U64 bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(sel[k]?bit:0ULL);}
 }
 U64 display(const vector<int>&order)const{U64 out=0,covered=0;for(int k:order){out|=color[k]&mask[k]&~covered;covered|=mask[k];}return out;}
 void remove_cells(const vector<U64>&A,int passes){
  int removed=0;for(int pass=0;pass<passes;pass++)for(int k=K-1;k>=0;k--)for(int j=0;j<64;j++)if(mask[k]>>j&1){U64 bit=1ULL<<j;bool valid=true;
   for(int v=0;v<int(A.size())&&valid;v++){bool reached=false;int value=0;for(int x:canonical[v]){if(x==k){reached=true;continue;}if(mask[x]&bit){value=bool(color[x]&bit);break;}}if(reached&&value!=int((A[v]>>j)&1))valid=false;}
   if(valid){mask[k]^=bit;removed++;}
  }for(int v=0;v<int(A.size());v++)assert(display(canonical[v])==A[v]);cerr<<"MaskRemoved="<<removed<<"\n";
 }
 vector<pair<int,int>>plan(vector<int>&order,int product,U64 target,int mode){
  int rank[192];for(int i=0;i<K;i++)rank[canonical[product][i]]=i;vector<char>deferred(K);vector<pair<int,int>>ops;U64 covered=0;int fixed=0;
  auto apply=[&](int from,int to){int k=order[from];assert(from>=fixed);U64 upper=0;for(int i=0;i<from;i++)upper|=mask[order[i]];assert(mask[k]&~upper);order.erase(order.begin()+from);order.insert(order.begin()+to,k);ops.push_back({k,to});};
  for(int chosen:canonical[product]){
   while(fixed<K){int x=order[fixed];if((color[x]^target)&mask[x]&~covered)break;covered|=mask[x];deferred[x]=false;fixed++;}
   if(display(order)==target)return ops;
   if(!(mask[chosen]&~covered))continue;
   while(true){
    int at=find(order.begin(),order.end(),chosen)-order.begin();U64 upper=0;for(int i=0;i<at;i++)upper|=mask[order[i]];if(mask[chosen]&~upper){if(at!=fixed)apply(at,fixed);deferred[chosen]=false;covered|=mask[chosen];fixed++;break;}
    U64 need=mask[chosen]&~covered;int bit=-1,best=100000;
    for(int j=0;j<64;j++)if(need>>j&1){int cnt=0;for(int i=fixed;i<at;i++)cnt+=(mask[order[i]]>>j)&1;if(cnt<best)best=cnt,bit=j;}
    assert(bit>=0);int from=fixed;while(!(mask[order[from]]>>bit&1))from++;int k=order[from];assert(from<at&&!deferred[k]);
    order.erase(order.begin()+from);int to=K-1;for(int i=fixed;i<K-1;i++)if(deferred[order[i]]&&rank[order[i]]>rank[k]){to=i;break;}order.insert(order.begin()+from,k);apply(from,to);deferred[k]=true;
    if(ops.size()>384)abort();
   }
  }
  assert(display(order)==target);return ops;
 }
};
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);int n,K,P,R;if(!(cin>>n>>K>>P>>R))return 0;vector<U64>original(P);for(int v=0;v<P;v++){string x;cin>>x;for(int j=0;j<n;j++)if(x[j]=='1')original[v]|=1ULL<<j;}
 structure2_training_origin=clock();auto m=train_multi_model(original,12345);exact_state_refit(m.A,m.papers,1,43,32,8,0);MaskFit fit(m.A,m.papers);fit.remove_cells(m.A,argc>1?atoi(argv[1]):1);
 for(int k=0;k<K;k++){string s(n,'.');for(int j=0;j<n;j++)if(fit.mask[k]>>j&1)s[m.cols[j]]=char('0'+(((fit.color[k]>>j)&1)^m.flip[j]));cout<<s<<'\n';}cout.flush();vector<int>order(K);iota(order.begin(),order.end(),0);
 for(int r=0;r<R;r++){int product;if(!(cin>>product)||product<0)return 0;auto ops=fit.plan(order,product,m.A[product],0);assert(fit.display(order)==m.A[product]);cout<<ops.size()<<'\n';for(auto[k,d]:ops)cout<<k<<' '<<d<<'\n';cout.flush();}
 cerr<<"FinishCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
}
'''
(d/'round6_maskfit.cpp').write_text(s)

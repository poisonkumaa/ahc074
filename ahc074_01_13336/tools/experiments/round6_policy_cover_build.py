from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'round6_maskfit.cpp').read_text()
s=s[:s.index('int main(int argc,char**argv)')]+r'''
#include "round6_policy_cover.h"
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);int n,K,P,R;if(!(cin>>n>>K>>P>>R))return 0;vector<U64>original(P);for(int v=0;v<P;v++){string x;cin>>x;for(int j=0;j<n;j++)if(x[j]=='1')original[v]|=1ULL<<j;}
 int passes=argc>1?atoi(argv[1]):1;
 structure2_training_origin=clock();auto m=train_multi_model(original,12345);exact_state_refit(m.A,m.papers,1,43,32,8,0);MaskFit fit(m.A,m.papers);fit.remove_cells(m.A,passes);
 for(int k=0;k<K;k++){string s(n,'.');for(int j=0;j<n;j++)if(fit.mask[k]>>j&1)s[m.cols[j]]=char('0'+(((fit.color[k]>>j)&1)^m.flip[j]));cout<<s<<'\n';}cout.flush();vector<int>order(K);iota(order.begin(),order.end(),0);
 int fallbacks=0;
 for(int r=0;r<R;r++){
  int product;if(!(cin>>product)||product<0)return 0;auto before=order;vector<pair<int,int>>operations;bool found=false;int best_visible=-1;
  for(double exponent:{.75,1.,1.25}){auto test=before;vector<pair<int,int>>ops;if(!visible_cover_plan(test,fit.mask,fit.color,m.A[product],exponent,ops))continue;assert(fit.display(test)==m.A[product]);U64 covered=0;int visible=0;for(int k:test){visible+=bool(fit.mask[k]&~covered);covered|=fit.mask[k];}
   if(!found||ops.size()<operations.size()||(ops.size()==operations.size()&&visible>best_visible)){found=true;best_visible=visible;operations=move(ops);order=move(test);}
  }
  if(!found){fallbacks++;order=before;operations=fit.plan(order,product,m.A[product],0);}assert(fit.display(order)==m.A[product]);assert(operations.size()<=2*K);
  cout<<operations.size()<<'\n';for(auto[k,d]:operations)cout<<k<<' '<<d<<'\n';cout.flush();
 }
 cerr<<"Fallbacks="<<fallbacks<<" FinishCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
}
'''
(p/'round6_policy_cover.cpp').write_text(s)
b=(p/'round6_policy_maskfit_bench.py').read_text().replace('round6_policy_maskfit','round6_policy_cover').replace("lower=int(re.search(r'DAGLower=(\\d+)',err)[1]),",'')
(p/'round6_policy_cover_bench.py').write_text(b)

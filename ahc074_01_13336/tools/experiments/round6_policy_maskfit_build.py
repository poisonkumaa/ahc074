from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'round6_maskfit.cpp').read_text()
interval=(p/'round6_intervals.cpp').read_text()
planner=interval[interval.index('struct IntervalPlan {'):interval.index('int main(int argc,char**argv)')]
planner=planner.replace('IntervalPlan','GeneralLISPlanner')
old='GeneralLISPlanner(const vector<PrefixPaper>&p,const vector<int>&begin):K(p.size()),mask(K),color(K){for(int k=0;k<K;k++){mask[k]=PrefixDP::low(p[k].len)^PrefixDP::low(begin[k]);color[k]=p[k].color&mask[k];}}'
new='GeneralLISPlanner(const vector<U64>&m,const vector<U64>&c):K(m.size()),mask(m),color(c){for(int k=0;k<K;k++)color[k]&=mask[k];}'
assert old in planner;planner=planner.replace(old,new)
s=s[:s.index('int main(int argc,char**argv)')]+planner+'\n#include "round6_policy_dag_target.h"\n'+r'''
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);int n,K,P,R;if(!(cin>>n>>K>>P>>R))return 0;vector<U64>original(P);for(int v=0;v<P;v++){string x;cin>>x;for(int j=0;j<n;j++)if(x[j]=='1')original[v]|=1ULL<<j;}
 int passes=argc>1?atoi(argv[1]):1,variants=argc>2?atoi(argv[2]):4;
 structure2_training_origin=clock();auto m=train_multi_model(original,12345);exact_state_refit(m.A,m.papers,1,43,32,8,0);MaskFit fit(m.A,m.papers);fit.remove_cells(m.A,passes);
 GeneralLISPlanner planner(fit.mask,fit.color);vector<int>length;for(auto p:m.papers)length.push_back(p.len);vector<char>selected(K);
 for(int k=0;k<K;k++){string s(n,'.');for(int j=0;j<n;j++)if(fit.mask[k]>>j&1)s[m.cols[j]]=char('0'+(((fit.color[k]>>j)&1)^m.flip[j]));cout<<s<<'\n';}cout.flush();vector<int>order(K);iota(order.begin(),order.end(),0);
 int fallbacks=0;long long lower_sum=0;
 for(int r=0;r<R;r++){
  int product;if(!(cin>>product)||product<0)return 0;auto before=order;vector<pair<int,int>>operations;bool found=false;int smallest=K;
  for(int variant=0;variant<variants;variant++){
   int minimum;auto target=color_dag_target(before,length,selected,fit.mask,fit.color,m.A[product],variant,minimum,&fit.canonical[product]);smallest=min(smallest,minimum);assert(planner.display(target)==m.A[product]);
   for(int mode=0;mode<4;mode++){auto test=before;vector<pair<int,int>>ops;int bound=found?int(operations.size())-1:2*K;
    if(planner.plan(test,target,m.A[product],bound,ops,mode)){operations=move(ops);order=move(test);found=true;}
   }
  }
  lower_sum+=smallest;
  if(!found){fallbacks++;order=before;operations=fit.plan(order,product,m.A[product],0);}
  assert(planner.display(order)==m.A[product]);assert(operations.size()<=2*K);
  cout<<operations.size()<<'\n';for(auto[k,d]:operations)cout<<k<<' '<<d<<'\n';cout.flush();
 }
 cerr<<"DAGLower="<<lower_sum<<" Fallbacks="<<fallbacks<<" FinishCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
}
'''
(p/'round6_policy_maskfit.cpp').write_text(s)

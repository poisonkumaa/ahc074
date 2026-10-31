from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'round6_intervals.cpp').read_text()
s=s.replace('int main(int argc,char**argv){','#include "round6_policy_free_goal.h"\nint main(int argc,char**argv){')
a=s.index(' int repetitions=argc>1?')
b=s.index(' PrefixDP dp(n,papers);',a)
s=s[:a]+r'''
 double quantile=argc>1?atof(argv[1]):.02;int tries=argc>2?atoi(argv[2]):8,repetitions=4;
 vector<vector<int>>required(active);
 {
  PrefixDP teacher(n,papers);teacher.public_data(A);vector<int>order(active);iota(order.begin(),order.end(),0);mt19937 rng(718927);vector<int>targets(P);iota(targets.begin(),targets.end(),0);auto ff=future;
  for(int r=0;r<128+P*repetitions;r++){
   if(r>=128&&(r-128)%P==0)shuffle(targets.begin(),targets.end(),rng);int t=r<128?rng()%P:targets[(r-128)%P],moves;
   for(int k=0;k<active;k++)teacher.reward[k]=16+2*__builtin_popcountll(ff[k]);
   auto chosen=teacher.solve(order,A[t],moves);vector<char>selected(active);int last=0;for(int k:chosen){selected[k]=1;if(r>=128)required[k].push_back(last);last=length[k];}
   order=direct_final(order,length,selected,&ff);U64 bit=1ULL<<(r%64);for(int k=0;k<active;k++)ff[k]=(ff[k]&~bit)|(selected[k]?bit:0ULL);
  }
 }
 for(int k=0;k<active;k++){auto&values=required[k];if(values.empty()){begin[k]=0;continue;}sort(values.begin(),values.end());begin[k]=values[min(int(values.size())-1,int(quantile*values.size()))];}
 int changed=0,holes=0;for(int k=0;k<active;k++){changed+=begin[k]>0;holes+=begin[k];}cerr<<"Quantile="<<quantile<<" CutPapers="<<changed<<" Holes="<<holes<<"\n";
 for(U64 target:A){int value[65],parent[65],who[65];fill(value,value+65,100000000);value[0]=0;
  for(int j=0;j<64;j++)if(value[j]<100000000)for(int k=0;k<active;k++){int l=length[k];if(l<=j||((papers[k].color^target)&(PrefixDP::low(l)^PrefixDP::low(j))))continue;int cost=value[j]+1024*max(0,begin[k]-j)+1;if(cost<value[l]){value[l]=cost;parent[l]=j;who[l]=k;}}
  for(int l=64;l;){int k=who[l],j=parent[l];begin[k]=min(begin[k],j);l=j;}
 }
''' + s[b:]
old='for(int mode=0;mode<tries;mode++){auto test=original_order;vector<pair<int,int>>ops;if(planner.plan(test,target,A[product],int(operations.size())-1,ops,mode)){operations=move(ops);order=move(test);}}'
new='auto free_target=free_goal_target(original_order,length,selected);assert(planner.display(free_target)==A[product]);for(int mode=0;mode<tries;mode++){auto test=original_order;vector<pair<int,int>>ops;if(planner.plan(test,mode<4?target:free_target,A[product],int(operations.size())-1,ops,mode)){operations=move(ops);order=move(test);}}'
assert old in s;s=s.replace(old,new)
(p/'round6_policy_quantile.cpp').write_text(s)

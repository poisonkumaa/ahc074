from pathlib import Path
p=Path(__file__).parent
s=(p.parent.parent/'main.cpp').read_text()
s=s.replace('inline void exact_state_refit(', 'static double round7_fair_alpha=0;\ninline void exact_state_refit(',1)
s=s.replace('vector<Scenario>scenarios(M);', 'vector<Scenario>scenarios(M);vector<long long>fair_sum(P),fair_count(P);')
old='''   for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);
   int moves;auto chosen=dp.solve(order,A[t],moves);'''
new='''   for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);
   if(round7_fair_alpha!=0&&r>=128&&(r-128)%128==0){dp.prepare(order);for(int v=0;v<P;v++){int cost;dp.solve_prepared(A[v],cost,false);fair_sum[v]+=cost;fair_count[v]++;}}
   int moves;auto chosen=dp.solve(order,A[t],moves);'''
assert old in s;s=s.replace(old,new,1)
s=s.replace(' scenarios.resize(M);', ''' scenarios.resize(M);
 vector<int>fair_weight(P,1);if(round7_fair_alpha!=0){double mean=0;for(int v=0;v<P;v++)mean+=double(fair_sum[v])/max(1LL,fair_count[v])/P;for(int v=0;v<P;v++)fair_weight[v]=clamp(int(lround(32*pow((double(fair_sum[v])/max(1LL,fair_count[v]))/mean,round7_fair_alpha))),8,128);cerr<<"FairWeight alpha="<<round7_fair_alpha<<" mean="<<mean<<" min="<<*min_element(fair_weight.begin(),fair_weight.end())<<" max="<<*max_element(fair_weight.begin(),fair_weight.end())<<"\\n";}
''',1)
s=s.replace('oldcost+=min(base,through[e*Q+q]);', 'oldcost+=fair_weight[v]*min(base,through[e*Q+q]);')
s=s.replace('strength*max(0,base-value));trie[path[v][d]].weight', 'strength*max(0,base-value));gain*=fair_weight[v];trie[path[v][d]].weight')
s=s.replace('newcost+=value;valid&=value<INF;', 'newcost+=fair_weight[v]*value;valid&=value<INF;')
s=s.replace('int main(){','''#include "round7_fair_metrics.h"
int main(int argc,char**argv){
 round7_fair_alpha=argc>1?atof(argv[1]):0;int repetitions=argc>2?atoi(argv[2]):8,second=argc>3?atoi(argv[3]):0,report=argc>4?atoi(argv[4]):0;''')
s=s.replace('exact_state_refit(A,papers,1,43,32,-1,0);','''exact_state_refit(A,papers,1,43,32,repetitions,0);
 if(second){round7_fair_alpha=0;exact_state_refit(A,papers,1,43,32,second,0);}
 if(report)round7_report_product_cost(A,papers);''')
assert 'gain*=fair_weight[v]' in s
(p/'round7_fair.cpp').write_text(s)

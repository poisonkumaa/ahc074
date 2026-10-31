from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'round6_policy_quantile.cpp').read_text()
s=s.replace('#include "round6_policy_free_goal.h"','#include "round6_policy_free_goal.h"\n#include "round6_policy_dag_target.h"')
s=s.replace('int savings=0,improved=0,fallbacks=0;','int savings=0,improved=0,fallbacks=0;long long dag_lower=0;')
old='auto free_target=free_goal_target(original_order,length,selected);assert(planner.display(free_target)==A[product]);for(int mode=0;mode<tries;mode++){auto test=original_order;vector<pair<int,int>>ops;if(planner.plan(test,mode<4?target:free_target,A[product],int(operations.size())-1,ops,mode)){operations=move(ops);order=move(test);}}'
new='''int lower;auto dag_target=color_dag_target(original_order,length,selected,planner.mask,planner.color,A[product],0,lower);dag_lower+=lower;assert(planner.display(dag_target)==A[product]);
  for(int mode=0;mode<tries;mode++){if(mode==4){dag_target=color_dag_target(original_order,length,selected,planner.mask,planner.color,A[product],3,lower);assert(planner.display(dag_target)==A[product]);}auto test=original_order;vector<pair<int,int>>ops;if(planner.plan(test,dag_target,A[product],int(operations.size())-1,ops,mode)){operations=move(ops);order=move(test);}}'''
assert old in s;s=s.replace(old,new).replace('cerr<<"IntervalSavings="','cerr<<"DAGLower="<<dag_lower<<"\\n";cerr<<"IntervalSavings="')
(p/'round6_policy_dag.cpp').write_text(s)
bench=(p/'round6_policy_quantile_bench.py').read_text().replace('round6_policy_quantile','round6_policy_dag')
(p/'round6_policy_dag_bench.py').write_text(bench)

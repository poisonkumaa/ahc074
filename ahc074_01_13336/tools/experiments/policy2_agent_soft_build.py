from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'global_agent_allocate_tune.cpp').read_text()
s=s.replace('#include "global_agent_allocate_tune.h"',(p/'global_agent_allocate_tune.h').read_text().replace('#pragma once','').replace('std::array<int,17> dp,split','std::array<int,7> dp,split'))
s=s.replace('train_prefix_dictionary(A,papers,n,argc>6?atoi(argv[6]):16,argc>1?atoi(argv[1]):1,argc>2?atoi(argv[2]):8,argc>3?atoi(argv[3]):0,argc>4?atoi(argv[4]):6,argc>5?atoi(argv[5]):0);','train_prefix_dictionary(A,papers,n,64,3,8,0,6,0);')
b=s.index('struct PrefixDP');e=s.index('#include <algorithm>',b);dp=s[b:e]
dp=dp.replace('int n,k;', 'int n,k,move_weight=256;')
dp=dp.replace('dp(n+1),parent(n+1),which(n+1);', 'dp(n+1),parent(n+1),which(n+1),opcount(n+1);')
dp=dp.replace('add*256-1', 'add*move_weight-1')
dp=dp.replace('dp[l][nh]=value;parent', 'dp[l][nh]=value;opcount[l][nh]=opcount[j][h]+add;parent')
dp=dp.replace('best=(dp[n][h]+255)/256;', 'best=opcount[n][h];')
s=s[:b]+dp+s[e:]
s=s.replace('PrefixDP dp(n,papers);', 'PrefixDP dp(n,papers);dp.move_weight=argc>1?atoi(argv[1]):4;long total_records=0;')
s=s.replace('int moves;auto chosen=dp.solve(order,A[product],moves);', 'int moves;auto chosen=dp.solve(order,A[product],moves);total_records+=chosen.size();')
pos=s.rfind('\n}');s=s[:pos]+'\n cerr<<"Records="<<double(total_records)/R<<" FinishCPU="<<double(clock())/CLOCKS_PER_SEC<<endl;'+s[pos:]
(p/'policy2_agent_soft.cpp').write_text(s)
grid=(p/'policy2_agent_grid.py').read_text().replace("settings=[['1','.2','1'],['0','1'],['.15','0'],['.3','.2'],['.5','.5'],['1','0'],['2','0'],['3','.2']]", "settings=[[str(w)] for w in [256,16,8,6,4,3,2,1]]")
(p/'policy2_agent_soft_grid.py').write_text(grid)

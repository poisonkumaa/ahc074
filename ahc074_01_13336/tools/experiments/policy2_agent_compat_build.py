from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'global_agent_allocate_tune.cpp').read_text()
s=s.replace('#include "global_agent_allocate_tune.h"',(p/'global_agent_allocate_tune.h').read_text().replace('#pragma once','').replace('std::array<int,17> dp,split','std::array<int,7> dp,split'))
s=s.replace('train_prefix_dictionary(A,papers,n,argc>6?atoi(argv[6]):16,argc>1?atoi(argv[1]):1,argc>2?atoi(argv[2]):8,argc>3?atoi(argv[3]):0,argc>4?atoi(argv[4]):6,argc>5?atoi(argv[5]):0);','train_prefix_dictionary(A,papers,n,64,3,8,0,6,0);')
b=s.index('struct PrefixDP');e=s.index('#include <algorithm>',b)
dp=s[b:e]
dp=dp.replace('int n,k;vector<PrefixPaper>a;', 'int n,k,offset=0;vector<PrefixPaper>a;vector<array<int,65>> compatible;')
loc=dp.index(' static U64 low')
fn=r'''
 void init_public(const vector<U64>&products,int weight){
  offset=weight;compatible.resize(k);
  for(int z=0;z<k;z++){int hist[65]{};for(U64 v:products){U64 d=(v^a[z].color)&low(a[z].len);int start=d?64-__builtin_clzll(d):0;hist[start]++;}int acc=0;for(int j=0;j<=64;j++){acc+=hist[j];compatible[z][j]=acc;}}
 }
'''
dp=dp[:loc]+fn+dp[loc:]
dp=dp.replace('1000000','1000000000').replace('add*256-1','add*65536-offset-compatible[z][j]').replace('(dp[n][h]+255)/256','(dp[n][h]+65535)/65536')
s=s[:b]+dp+s[e:]
s=s.replace('PrefixDP dp(n,papers);', 'PrefixDP dp(n,papers);dp.init_public(A,argc>1?atoi(argv[1]):0);long total_records=0;')
s=s.replace('int moves;auto chosen=dp.solve(order,A[product],moves);', 'int moves;auto chosen=dp.solve(order,A[product],moves);total_records+=chosen.size();')
pos=s.rfind('\n}')
s=s[:pos]+'\n cerr<<"Records="<<double(total_records)/R<<" FinishCPU="<<double(clock())/CLOCKS_PER_SEC<<endl;'+s[pos:]
(p/'policy2_agent_compat.cpp').write_text(s)

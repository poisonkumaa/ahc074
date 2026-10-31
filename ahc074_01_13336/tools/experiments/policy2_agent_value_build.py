from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'global_agent_allocate_tune.cpp').read_text()
s=s.replace('#include "global_agent_allocate_tune.h"',(p/'global_agent_allocate_tune.h').read_text().replace('#pragma once','').replace('std::array<int,17> dp,split','std::array<int,7> dp,split'))
s=s.replace('train_prefix_dictionary(A,papers,n,argc>6?atoi(argv[6]):16,argc>1?atoi(argv[1]):1,argc>2?atoi(argv[2]):8,argc>3?atoi(argv[3]):0,argc>4?atoi(argv[4]):6,argc>5?atoi(argv[5]):0);','train_prefix_dictionary(A,papers,n,64,3,8,0,6,0);')
b=s.index('struct PrefixDP');e=s.index('#include <algorithm>',b)
s=s[:b]+(p/'policy2_agent_value_dp.txt').read_text()+'\n'+s[e:]
b=s.index(' for(int k=0;k<K;k++){\n  string pattern');e=s.index(' for(int r=0;r<R;r++){',b)
block=s[b:e];d=block.index(' PrefixDP dp(n,papers);');setup=block[d:];output=block[:d]
s=s[:b]+setup+(p/'policy2_agent_value_train.txt').read_text()+output+s[e:]
pos=s.rfind('\n}');s=s[:pos]+'\n cerr<<"FinishCPU="<<double(clock())/CLOCKS_PER_SEC<<endl;'+s[pos:]
(p/'policy2_agent_value.cpp').write_text(s)

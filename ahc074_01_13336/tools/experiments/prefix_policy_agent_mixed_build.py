from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'prefix_policy_agent_fastdp.cpp').read_text()
start=s.index('// Coordinate optimization of dictionaries')
end=s.index('double ideal_records',start)
header=(p/'prefix_allocate_train.h').read_text().replace('#pragma once','').replace('std::array<int,17> dp,split','std::array<int,9> dp,split').replace('std::array<int,65> dp,split','std::array<int,9> dp,split')
s=s[:start]+header+'\n'+s[end:]
start=s.index('auto ordercols=');end=s.index('vector<int>training',start)
order=s[start:end]
order=order.replace('array<int,64>cols;', 'mt19937_64 crng(attempt?rng():12345);array<int,64>cols;')
order=order.replace('shuffle(cols.begin(),cols.end(),rng)', 'shuffle(cols.begin(),cols.end(),crng)')
order=order.replace('STEPS=90000', 'STEPS=300000')
order=order.replace('rng()%64', 'crng()%64').replace('rng()>>11', 'crng()>>11')
order=order.replace('if(x>y)swap(x,y);if(x==y)continue;', 'if(x>y)swap(x,y);(void)(crng()%3);if(x==y)continue;')
s=s[:start]+order+s[end:]
s=s.replace('auto build=[&](array<int,64>cols,int tsp)', 'auto build=[&](array<int,64>cols,int tsp,int variant)')
s=s.replace('train_prefix_dictionary(c.target,c.paper,64,16,1,8,0);', 'train_prefix_dictionary(c.target,c.paper,64,16,1,8,0,variant%3==2?8:6,variant%3==2?2:0);')
s=s.replace('auto c=build(cols,tsp);', 'auto c=build(cols,tsp,run*2+dir);')
s=s.replace('simulate(c,80)', 'simulate(c,128)')
(p/'prefix_policy_agent_mixed.cpp').write_text(s)

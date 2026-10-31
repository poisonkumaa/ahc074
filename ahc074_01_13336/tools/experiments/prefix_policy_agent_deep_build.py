from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'prefix_policy_agent_cap64.cpp').read_text()
b=s.index('// Coordinate optimization of dictionaries');e=s.index('double ideal_records',b)
header=(p/'global_agent_allocate_tune.h').read_text().replace('#pragma once','').replace('std::array<int,17> dp,split','std::array<int,9> dp,split')
s=s[:b]+header+'\n'+s[e:]
s=s.replace('#define SHRINK 1.4', '#define SHRINK 0.7')
s=s.replace('train_prefix_dictionary(c.target,c.paper,64,64,1,8,0,variant%3==2?8:6,variant%3==2?2:0);', 'train_prefix_dictionary(c.target,c.paper,64,64,3,8,0,variant==0?6:8,variant==0?0:variant==1?2:4);')
s=s.replace('vector<int>training;for(int q=0;q<2;q++)', 'vector<int>training;for(int w=0;w<16;w++){int t=rng()%P;if(w&&t==training.back())t=(t+1)%P;training.push_back(t);}for(int q=0;q<2;q++)')
s=s.replace('simulate(c,128)', 'simulate(c,272)')
s=s.replace('run<3;', 'run<2;')
s=s.replace('if(double(clock()-started)/CLOCKS_PER_SEC>.92)break;', 'if(candidates.size()>=3||double(clock()-started)/CLOCKS_PER_SEC>.92)break;')
s=s.replace('int finalists=min(3,int(candidates.size()));','int finalists=0;')
(p/'prefix_policy_agent_deep.cpp').write_text(s)

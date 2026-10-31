from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'prefix_policy_agent_shrink.cpp').read_text()
start=s.index('// Coordinate optimization of dictionaries')
end=s.index('double ideal_records',start)
s=s[:start]+(p/'prefix_allocate_train.h').read_text().replace('#pragma once','')+'\n'+s[end:]
s=s.replace('train_prefix_dictionary(c.target,c.paper,64,16,1,8);','train_prefix_dictionary(c.target,c.paper,64,16,1,8,100);')
(p/'prefix_policy_agent_allocate.cpp').write_text(s)
(p/'prefix_policy_agent_allocate0.cpp').write_text(s.replace('train_prefix_dictionary(c.target,c.paper,64,16,1,8,100);','train_prefix_dictionary(c.target,c.paper,64,16,1,8,0);'))

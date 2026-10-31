from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'prefix_policy_agent_mixed.cpp').read_text()
s=s.replace('train_prefix_dictionary(c.target,c.paper,64,16,1,8,0,','train_prefix_dictionary(c.target,c.paper,64,64,1,8,0,')
(p/'prefix_policy_agent_cap64.cpp').write_text(s)

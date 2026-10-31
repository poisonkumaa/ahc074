from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'prefix_policy_agent_short.cpp').read_text()
s=s.replace('struct Candidate{',(p/'global_agent_weighted.h').read_text().replace('#pragma once','')+'\nstruct Candidate{')
s=s.replace('trie_patterns(c.target,len,12,3)', 'trie_patterns(c.target,len,16,3)')
s=s.replace('c.order.resize(c.paper.size());', 'train_prefix_dictionary(c.target,c.paper,64,16,1,8);c.order.resize(c.paper.size());')
s=s.replace('run<6;', 'run<3;')
s=s.replace('>.65','>.92').replace('>.78','>1.05')
(p/'prefix_policy_agent_weighted.cpp').write_text(s)

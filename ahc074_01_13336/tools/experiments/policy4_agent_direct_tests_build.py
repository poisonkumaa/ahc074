from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy4_agent_first_stack100.py').read_text().replace('policy4_agent_first','policy4_agent_direct')
(p/'policy4_agent_direct_stack100.py').write_text(s)
s=(p/'policy4_agent_first_reachable.cpp').read_text().replace('policy4_agent_first_compare.cpp','policy4_agent_direct_compare.cpp')
s=s.replace('auto a=order,b=order;', 'auto a=order,b=direct_final(order,length,selected,&future);')
a=s.index('auto[u,v]=GlobalPrefix::first_move(');b=s.index('}',a)
s=s[:a]+s[b:]
(p/'policy4_agent_direct_reachable.cpp').write_text(s)

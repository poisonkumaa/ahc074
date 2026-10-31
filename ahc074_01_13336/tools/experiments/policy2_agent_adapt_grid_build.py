from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy2_agent_union_grid.py').read_text().replace("policy2_agent_union'),str(size)","policy2_agent_union_adapt'),str(size)").replace('for size in [64,128,256]:','for size in [1,2,4,8]:').replace('policy2_agent_union_grid.json','policy2_agent_union_adapt_grid.json')
(p/'policy2_agent_union_adapt_grid.py').write_text(s)

from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy2_agent_union.cpp').read_text()
pos=s.index('  cout<<operations.size()')
s=s[:pos]+'''  {int slot=r%samples,word=slot/64;uint64_t bit=1ULL<<(slot%64);for(int k=0;k<int(papers.size());k++)future[k][word]=(future[k][word]&~bit)|(selected[k]?bit:0ULL);}
'''+s[pos:]
(p/'policy2_agent_union_size.cpp').write_text(s)
s=(p/'policy2_agent_union_grid.py').read_text().replace('policy2_agent_union','policy2_agent_union_size').replace('for size in [64,128,256]:','for size in [8,16,32,64,128,256]:')
(p/'policy2_agent_union_size_grid.py').write_text(s)

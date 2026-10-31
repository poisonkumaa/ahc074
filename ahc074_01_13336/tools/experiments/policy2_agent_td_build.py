from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy2_agent_valuepapers.cpp').read_text()
b=s.index(' dp.public_data(A);');e=s.index(' for(int k=0;k<K;k++){\n  string pattern',b)
s=s[:b]+(p/'policy2_agent_td_train.txt').read_text()+s[e:]
(p/'policy2_agent_td.cpp').write_text(s)

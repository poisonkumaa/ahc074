from pathlib import Path
p=Path(__file__).resolve().parent
small=(p/'policy_agent_v3.cpp').read_text().split('struct Group')[0]
graph=(p/'policy_agent_auggraph.h').read_text().replace('#pragma once','')
main=(p/'policy_agent_augmain.txt').read_text()
for bits,limit in [(9,2),(10,1)]:
    s='#pragma GCC optimize("Ofast,unroll-loops")\n'+small+graph+(p/f'policy_agent_model{bits}.h').read_text()+main.replace('MODEL_LIMIT',str(limit))
    (p/f'policy_agent_aug{bits}.cpp').write_text(s)

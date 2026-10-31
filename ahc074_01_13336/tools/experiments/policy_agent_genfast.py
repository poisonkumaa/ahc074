from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy_agent_v3.cpp').read_text().split('struct Group')[0]
m=(p/'policy_agent_fastmain.txt').read_text()
for steps,temp in [(30000,'.02'),(100000,'.02'),(100000,'.008'),(250000,'.008')]:
    name=f'policy_agent_fast{steps}_{temp[1:]}'
    (p/(name+'.cpp')).write_text(s+m.replace('FAST_STEPS',str(steps)).replace('FAST_TEMP',temp))

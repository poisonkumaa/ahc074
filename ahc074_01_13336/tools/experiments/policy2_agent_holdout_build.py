from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy_agent_parallel.py').read_text().replace("root/f'tools/in/{seed:04}.txt'","root/f'tools/experiments/holdout/in/{seed:04}.txt'").replace('range(100)','range(50)').replace("f'{name}_100results.json'","f'{name}_holdout50results.json'").replace("/100,'cpu max'","/50,'cpu max'")
(p/'policy2_agent_holdout.py').write_text(s)

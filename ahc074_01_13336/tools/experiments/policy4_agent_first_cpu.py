import subprocess,re,json,hashlib
from pathlib import Path
root=Path(__file__).resolve().parents[2]
out=[]
for seed in [0,1,2,50,99]:
 basehash=None
 for name in ['policy4_agent_base_full','policy4_agent_first_full']:
  r=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments'/name)],input=(root/f'tools/in/{seed:04}.txt').read_text(),capture_output=True,text=True)
  score=int(re.search('Score = (\\d+)',r.stderr)[1]);prep=float(re.search('PolicyCPU=([\\d.]+)',r.stderr)[1]);finish=float(re.search('FinishCPU=([\\d.]+)',r.stderr)[1]);h=hashlib.sha256(r.stdout.encode()).hexdigest()
  if basehash is None:basehash=h
  # Operation order may differ; terminal stacks were checked independently.
  row=dict(seed=seed,name=name,score=score,cpu=finish,online=finish-prep);out.append(row);print(row,flush=True)
(root/'tools/experiments/policy4_agent_first_cpu.json').write_text(json.dumps(out,indent=2))

import subprocess,re,json,hashlib,concurrent.futures
from pathlib import Path
root=Path(__file__).resolve().parents[2]
def run(seed):
 out=[];basehash=None
 for name in ['policy2_agent_perf_base_full','policy2_agent_perf_training_full']:
  r=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments'/name)],input=(root/f'tools/in/{seed:04}.txt').read_text(),capture_output=True,text=True)
  score=int(re.search('Score = (\\d+)',r.stderr)[1]);prep=float(re.search('PolicyCPU=([\\d.]+)',r.stderr)[1]);finish=float(re.search('FinishCPU=([\\d.]+)',r.stderr)[1]);h=hashlib.sha256(r.stdout.encode()).hexdigest()
  ls=r.stdout.splitlines();idx=192;moves=0
  for _ in range(1024):m=int(ls[idx]);moves+=m;idx+=1+m
  assert idx==len(ls) and score==moves,(seed,name,r.stderr)
  if basehash is None:basehash=h
  assert h==basehash,(seed,name,r.stderr)
  out.append(dict(score=score,cpu=finish,online=finish-prep))
 if seed%10==0:print(seed,out,flush=True)
 return dict(seed=seed,base=out[0],fast=out[1])
with concurrent.futures.ThreadPoolExecutor(3) as ex:out=list(ex.map(run,range(100)))
(root/'tools/experiments/policy2_agent_perf_equivalence100.json').write_text(json.dumps(out,indent=2))
print('100 exact outputs equal; mean score',sum(r['fast']['score'] for r in out)/100)
for name in ['base','fast']:print(name,'mean CPU',sum(r[name]['cpu'] for r in out)/100,'max CPU',max(r[name]['cpu'] for r in out),'online CPU',sum(r[name]['online'] for r in out)/100)

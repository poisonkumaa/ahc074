from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import json,re,subprocess
d=Path(__file__).resolve().parent;root=d.parents[1]
configs=[(3,4),(3,8),(3,16),(3,32),(5,8),(5,16),(9,8),(9,16)]
def run(job):
 c,seed=job
 p=subprocess.run([str(root/'tools/target/release/tester'),str(d/'noisy_submit'),*map(str,c)],input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),capture_output=True,check=True)
 err=p.stderr.decode();score=int(re.search(r'Score = (\d+)',err)[1]);tokens=p.stdout.split();at=192;moves=0
 for _ in range(1024):m=int(tokens[at]);assert 0<=m<=384;moves+=m;at+=1+2*m
 assert at==len(tokens) and score==moves
 return dict(config=c,seed=seed,score=score,cpu=float(re.search(r'FinishCPU=([\d.]+)',err)[1]),log=err)
rows=[]
with ThreadPoolExecutor(max_workers=3) as pool:
 for r in pool.map(run,[(c,s) for c in configs for s in range(3)]):
  rows.append(r)
  if r['seed']==2:
   rr=[x for x in rows if x['config']==r['config']];print(r['config'],sum(x['score'] for x in rr)/len(rr),max(x['cpu'] for x in rr),flush=True)
(d/'noisy_grid.json').write_text(json.dumps(rows,indent=2))

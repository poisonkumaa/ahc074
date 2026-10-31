import subprocess,json,concurrent.futures
from pathlib import Path
root=Path(__file__).resolve().parents[2]
def run(job):
 size,seed=job
 with (root/f'tools/in/{seed:04}.txt').open() as inp:
  r=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/policy2_agent_union_size'),str(size)],stdin=inp,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
 import re
 score=int(re.search(r'Score = (\d+)',r.stderr)[1]);cpu=float(re.search(r'FinishCPU=([\d.]+)',r.stderr)[1]);ls=r.stdout.splitlines();i=192;t=0
 for _ in range(1024):m=int(ls[i]);t+=m;i+=1+m
 assert t==score,(size,seed,r.stderr)
 return size,seed,score,cpu
out=[]
for size in [8,16,32,64,128,256]:
 with concurrent.futures.ThreadPoolExecutor(3) as pool:r=list(pool.map(run,[(size,i) for i in range(3)]))
 item=dict(size=size,scores=[x[2] for x in r],mean=sum(x[2] for x in r)/len(r),cpu=max(x[3] for x in r));print(item,flush=True);out.append(item)
(root/'tools/experiments/policy2_agent_union_size_grid.json').write_text(json.dumps(out,indent=2))

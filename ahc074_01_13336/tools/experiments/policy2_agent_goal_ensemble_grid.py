import subprocess,json,concurrent.futures,re
from pathlib import Path
root=Path(__file__).resolve().parents[2]
def run(job):
 args,seed=job
 with (root/f'tools/in/{seed:04}.txt').open() as inp:
  r=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/policy2_agent_goal_ensemble'),*map(str,args)],stdin=inp,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
 score=int(re.search(r'Score = (\d+)',r.stderr)[1]);cpu=float(re.search(r'FinishCPU=([\d.]+)',r.stderr)[1]);ls=r.stdout.splitlines();i=192;t=0
 for _ in range(1024):m=int(ls[i]);t+=m;i+=1+m
 assert t==score,(args,seed,r.stderr)
 return args,seed,score,cpu
out=[]
for args in [(1,8,2),(3,8,0),(3,8,1),(3,4,2),(3,8,2),(3,16,2),(3,32,2),(4,8,2),(3,8,3)]:
 with concurrent.futures.ThreadPoolExecutor(3) as pool:r=list(pool.map(run,[(args,i) for i in range(3)]))
 item=dict(args=args,scores=[x[2] for x in r],mean=sum(x[2] for x in r)/len(r),cpu=max(x[3] for x in r));print(item,flush=True);out.append(item)
(root/'tools/experiments/policy2_agent_goal_ensemble_grid.json').write_text(json.dumps(out,indent=2))

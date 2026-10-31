import subprocess,re,json,concurrent.futures
from pathlib import Path
root=Path(__file__).resolve().parents[2]
def run(job):
 args,seed=job
 r=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/round4_policy_rollout'),*map(str,args)],input=(root/f'tools/in/{seed:04}.txt').read_text(),capture_output=True,text=True)
 score=int(re.search(r'Score = (\d+)',r.stderr)[1]);cpu=float(re.search(r'FinishCPU=([\d.]+)',r.stderr)[1]);count=re.search(r'Lookahead=(\d+) Candidates=(\d+) Changed=(\d+) Extra=(\d+)',r.stderr)
 lines=r.stdout.splitlines();at=192;ops=0
 for _ in range(1024):m=int(lines[at]);ops+=m;at+=1+m
 assert ops==score and at==len(lines),(args,seed,r.stderr)
 return dict(seed=seed,score=score,cpu=cpu,lookahead=int(count[1]),candidates=int(count[2]),changed=int(count[3]),extra=int(count[4]))
out=[]
for args in [(0,32,2,.05),(1,32,2,.05),(1,255,1,.05),(2,16,2,.05),(2,64,2,.1)]:
 with concurrent.futures.ThreadPoolExecutor(3) as pool:cases=list(pool.map(run,[(args,s) for s in range(3)]))
 row=dict(args=args,cases=cases,mean=sum(x['score'] for x in cases)/len(cases),max_cpu=max(x['cpu'] for x in cases));print(row,flush=True);out.append(row)
(root/'tools/experiments/round4_policy_rollout_grid.json').write_text(json.dumps(out,indent=2))

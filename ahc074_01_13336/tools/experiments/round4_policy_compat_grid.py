import subprocess,re,json,concurrent.futures
from pathlib import Path
root=Path(__file__).resolve().parents[2]
def run(job):
 weight,seed=job
 r=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/round4_policy_compat'),str(weight)],input=(root/f'tools/in/{seed:04}.txt').read_text(),capture_output=True,text=True)
 assert r.returncode==0,(weight,seed,r.stderr)
 score=int(re.search(r'Score = (\d+)',r.stderr)[1]);cpu=float(re.search(r'FinishCPU=([\d.]+)',r.stderr)[1]);lines=r.stdout.splitlines();at=192;ops=0
 for _ in range(1024):m=int(lines[at]);ops+=m;at+=1+m
 assert ops==score and at==len(lines),(weight,seed,r.stderr)
 return dict(seed=seed,score=score,cpu=cpu)
out=[]
for weight in [0,1,4]:
 with concurrent.futures.ThreadPoolExecutor(3) as pool:cases=list(pool.map(run,[(weight,s) for s in range(3)]))
 row=dict(weight=weight,cases=cases,mean=sum(x['score'] for x in cases)/len(cases),max_cpu=max(x['cpu'] for x in cases));print(row,flush=True);out.append(row)
(root/'tools/experiments/round4_policy_compat_grid.json').write_text(json.dumps(out,indent=2))

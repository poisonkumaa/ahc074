import subprocess,re,json,concurrent.futures,sys,hashlib
from pathlib import Path
root=Path(__file__).resolve().parents[2]
def run(job):
 args,seed=job
 r=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/round5_policy_optional'),*map(str,args)],input=(root/f'tools/in/{seed:04}.txt').read_text(),capture_output=True,text=True)
 assert r.returncode==0,(args,seed,r.stderr)
 score=int(re.search(r'Score = (\d+)',r.stderr)[1]);cpu=float(re.search(r'FinishCPU=([\d.]+)',r.stderr)[1]);count=re.search(r'Refinements=(\d+)',r.stderr);fallbacks=int(re.search(r'PathFallbacks=(\d+)',r.stderr)[1])
 lines=r.stdout.splitlines();at=192;ops=0
 for _ in range(1024):
  m=int(lines[at]);assert 0<=m<=384;ops+=m;at+=1+m
 assert ops==score and at==len(lines),(args,seed,r.stderr)
 return dict(seed=seed,score=score,cpu=cpu,sha=hashlib.sha256(r.stdout.encode()).hexdigest(),changed=int(count[1]),fallbacks=fallbacks)
out=[]
amount=int(sys.argv[1]) if len(sys.argv)>1 else 3
configs=[(1,-1),(1,1),(2,1),(.5,1)] if len(sys.argv)<3 else [(float(sys.argv[2]),int(sys.argv[3]))]
for args in configs:
 with concurrent.futures.ThreadPoolExecutor(3) as pool:cases=list(pool.map(run,[(args,s) for s in range(amount)]))
 row=dict(args=args,cases=cases,mean=sum(x['score'] for x in cases)/len(cases),max_cpu=max(x['cpu'] for x in cases));print(row,flush=True);out.append(row)
 (root/'tools/experiments/round5_policy_optional_grid.json').write_text(json.dumps(out,indent=2))

import subprocess,re,json,concurrent.futures,sys,hashlib
from pathlib import Path
root=Path(__file__).resolve().parents[2]
def run(job):
 name,args,seed=job
 r=subprocess.run([str(root/'tools/target/release/tester'),str(root/f'tools/experiments/{name}'),*map(str,args)],input=(root/f'tools/in/{seed:04}.txt').read_text(),capture_output=True,text=True)
 assert r.returncode==0,(name,args,seed,r.stderr)
 score=int(re.search(r'Score = (\d+)',r.stderr)[1]);cpu=float(re.search(r'FinishCPU=([\d.]+)',r.stderr)[1]);count=re.search(r'PlusUsed=(\d+) PlusPossible=(\d+) PredictedGain=([\d.]+)',r.stderr)
 lines=r.stdout.splitlines();at=192;ops=0
 for _ in range(1024):
  m=int(lines[at]);assert 0<=m<=384;ops+=m;at+=1+m
 assert ops==score and at==len(lines),(args,seed,r.stderr)
 return dict(seed=seed,score=score,cpu=cpu,sha=hashlib.sha256(r.stdout.encode()).hexdigest(),**(dict(plus=int(count[1]),possible=int(count[2]),predicted=float(count[3])) if count else {}))
out=[]
amount=int(sys.argv[1]) if len(sys.argv)>1 else 3
for name,args in [('round5_policy_base',()),('round5_policy_extra',(0,1,0)),('round5_policy_extra',(255,1,0)),('round5_policy_extra',(32,2,.1))]:
 with concurrent.futures.ThreadPoolExecutor(3) as pool:cases=list(pool.map(run,[(name,args,s) for s in range(amount)]))
 row=dict(name=name,args=args,cases=cases,mean=sum(x['score'] for x in cases)/len(cases),max_cpu=max(x['cpu'] for x in cases));print(row,flush=True);out.append(row)
 (root/'tools/experiments/round5_policy_grid.json').write_text(json.dumps(out,indent=2))

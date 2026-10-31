from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import subprocess,sys,json,re
d=Path(__file__).resolve().parent;root=d.parents[1]
solver=d/sys.argv[1];count=int(sys.argv[2]);args=sys.argv[3:]
def run(i):
 p=subprocess.run([str(root/'tools/target/release/tester'),str(solver),*args],input=(root/f'tools/in/{i:04d}.txt').read_bytes(),capture_output=True,check=True,timeout=120)
 err=p.stderr.decode();score=int(re.search(r'Score = (\d+)',err)[1]);tokens=p.stdout.split();at=192;total=0
 for r in range(1024):m=int(tokens[at]);assert 0<=m<=384;total+=m;at+=1+2*m
 assert at==len(tokens) and total==score
 cpu=re.search(r'FinishCPU=([\d.]+)',err)
 return dict(seed=i,score=score,cpu=float(cpu[1]) if cpu else None,diagnostics=err)
with ThreadPoolExecutor(max_workers=3) as pool:rows=list(pool.map(run,range(count)))
summary=dict(count=count,mean=sum(r['score'] for r in rows)/count,max_cpu=max((r['cpu'] or 0) for r in rows),errors=0)
(d/(solver.name+'_'+str(count)+'_'+'_'.join(args)+'_results.json')).write_text(json.dumps(dict(summary=summary,cases=rows),indent=2))
print(json.dumps(summary),flush=True)

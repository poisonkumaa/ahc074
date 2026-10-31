from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import json,re,subprocess,sys,hashlib
d=Path(__file__).resolve().parent;root=d.parents[1]
solver=d/sys.argv[1];dirname=sys.argv[2] if len(sys.argv)>2 else 'final_holdout'
indices=list(map(int,(d/dirname/'seeds.txt').read_text().split()))
def run(i):
 p=subprocess.run([str(root/'tools/target/release/tester'),str(solver)],input=(d/dirname/f'in/{i:04d}.txt').read_bytes(),capture_output=True,check=True)
 err=p.stderr.decode();score=int(re.search(r'Score = (\d+)',err)[1]);tokens=p.stdout.split();at=192;moves=0;maximum=0
 for _ in range(1024):m=int(tokens[at]);assert 0<=m<=384;moves+=m;maximum=max(maximum,m);at+=1+2*m
 assert at==len(tokens) and score==moves
 return dict(seed=indices[i],score=score,errors=0,max_moves=maximum,cpu=float(re.search(r'FinishCPU=([\d.]+)',err)[1]))
with ThreadPoolExecutor(max_workers=3) as pool:rows=list(pool.map(run,range(len(indices))))
summary=dict(mean=sum(x['score'] for x in rows)/len(rows),maximum_cpu=max(x['cpu'] for x in rows),errors=0,source_sha256=hashlib.sha256(solver.with_suffix('.cpp').read_bytes()).hexdigest())
(d/f'{solver.name}_{dirname}_results.json').write_text(json.dumps(dict(summary=summary,cases=rows),indent=2))
print(json.dumps(summary),flush=True)

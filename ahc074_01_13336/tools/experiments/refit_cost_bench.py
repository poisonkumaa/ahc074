from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import json,re,subprocess
root=Path(__file__).resolve().parents[2]
configs=[(0,2),(1,2),(2,2),(4,2),(1,1),(2,1)]
def run(job):
    cfg,seed=job;alpha,weight=cfg
    p=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/refit_cost_submit'),'1','8','0','6','0','64','1',str(weight),'0',str(alpha)],input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    err=p.stderr.decode();m=re.search(r'Score = (\d+)',err);assert p.returncode==0 and m,(job,err)
    score=int(m[1]);at=192;total=0;tokens=p.stdout.split()
    for _ in range(1024):count=int(tokens[at]);total+=count;at+=1+2*count
    assert total==score and at==len(tokens)
    return dict(alpha=alpha,weight=weight,seed=seed,score=score,log=err)
with ThreadPoolExecutor(max_workers=3) as pool: rows=list(pool.map(run,[(c,s) for c in configs for s in range(3)]))
for a,w in configs:
    scores=[r['score'] for r in rows if r['alpha']==a and r['weight']==w]
    print(a,w,sum(scores)/len(scores),scores,flush=True)
(root/'tools/experiments/refit_cost_bench.json').write_text(json.dumps(rows,indent=2))

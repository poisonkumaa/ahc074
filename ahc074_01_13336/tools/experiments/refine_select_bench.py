from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import json,re,subprocess
root=Path(__file__).resolve().parents[2]
configs=[(0,3),(.3,3),(0,4),(.3,4)]
def run(job):
    cfg,seed=job;shrink,count=cfg
    p=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/refine_select_submit'),str(shrink),str(count)],input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    err=p.stderr.decode();m=re.search(r'Score = (\d+)',err);assert p.returncode==0 and m,(job,err)
    score=int(m[1]);at=192;total=0;tokens=p.stdout.split()
    for _ in range(1024):count_=int(tokens[at]);total+=count_;at+=1+2*count_
    assert total==score and at==len(tokens)
    return dict(shrink=shrink,count=count,seed=seed,score=score,cpu=float(re.search(r'FinishCPU=([\d.]+)',err)[1]),log=err)
with ThreadPoolExecutor(max_workers=3) as pool: rows=list(pool.map(run,[(c,s) for c in configs for s in range(10)]))
for a,w in configs:
    rr=[r for r in rows if r['shrink']==a and r['count']==w]
    print(a,w,sum(r['score'] for r in rr)/len(rr),'CPU',max(r['cpu'] for r in rr),[r['score'] for r in rr],flush=True)
(root/'tools/experiments/refine_select_bench.json').write_text(json.dumps(rows,indent=2))

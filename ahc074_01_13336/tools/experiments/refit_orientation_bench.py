from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import json,re,subprocess
root=Path(__file__).resolve().parents[2]
configs=[0,.01,.25,.5,1,2,4,100]
def run(job):
    bias,seed=job
    p=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/refit_orientation_submit'),'1','8','0','6','0','64','1','8','0',str(bias)],input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    err=p.stderr.decode();m=re.search(r'Score = (\d+)',err);assert p.returncode==0 and m,(job,err)
    score=int(m[1]);at=192;total=0;tokens=p.stdout.split()
    for _ in range(1024):count=int(tokens[at]);total+=count;at+=1+2*count
    assert total==score and at==len(tokens)
    return dict(bias=bias,seed=seed,score=score,log=err)
with ThreadPoolExecutor(max_workers=3) as pool: rows=list(pool.map(run,[(c,s) for c in configs for s in range(3)]))
for c in configs:
    scores=[r['score'] for r in rows if r['bias']==c];print(c,sum(scores)/len(scores),scores,flush=True)
(root/'tools/experiments/refit_orientation_bench.json').write_text(json.dumps(rows,indent=2))

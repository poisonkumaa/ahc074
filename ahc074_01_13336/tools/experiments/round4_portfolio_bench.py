from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import json,re,subprocess,sys
root=Path(__file__).resolve().parents[2]
solver=root/'tools/experiments'/sys.argv[1]
count=int(sys.argv[2]) if len(sys.argv)>2 else 10
inputs=root/(sys.argv[3] if len(sys.argv)>3 else 'tools/in')
def run(i):
    r=subprocess.run([str(root/'tools/target/release/tester'),str(solver)],input=(inputs/f'{i:04d}.txt').read_bytes(),stdout=subprocess.PIPE,stderr=subprocess.PIPE,check=True,timeout=180)
    err=r.stderr.decode();score=int(re.findall(r'Score = (\d+)',err)[-1]);cpu=float(re.findall(r'FinishCPU=([\d.]+)',err)[-1])
    models=re.findall(r'candidate=(\d+) estimate=([\d.]+) elapsed=([\d.]+)',err)
    selected=re.findall(r'selected=(\d+) candidates=(\d+) estimate=([\d.]+) PrepCPU=([\d.]+)',err)
    tokens=r.stdout.split();at=192;moves=maximum=0
    for _ in range(1024):
        n=int(tokens[at]);assert 0<=n<=384;at+=1+2*n;moves+=n;maximum=max(maximum,n)
    assert at==len(tokens) and score==moves,(i,err)
    return dict(seed=i,score=score,errors=0,cpu=cpu,models=models,selected=int(selected[-1][0]),candidates=int(selected[-1][1]),estimate=float(selected[-1][2]),prep_cpu=float(selected[-1][3]),max_moves=maximum)
with ThreadPoolExecutor(max_workers=3) as pool:rows=list(pool.map(run,range(count)))
summary=dict(count=count,mean=sum(x['score'] for x in rows)/count,minimum=min(x['score'] for x in rows),maximum=max(x['score'] for x in rows),errors=0,max_cpu=max(x['cpu'] for x in rows),mean_cpu=sum(x['cpu'] for x in rows)/count,max_prep_cpu=max(x['prep_cpu'] for x in rows),candidate_counts={str(w):sum(x['candidates']==w for x in rows) for w in sorted(set(x['candidates'] for x in rows))},selected={str(w):sum(x['selected']==w for x in rows) for w in sorted(set(x['selected'] for x in rows))})
out=root/'tools/experiments'/(solver.name+'_'+inputs.parent.name+'_'+str(count)+'_results.json')
out.write_text(json.dumps(dict(summary=summary,cases=rows),indent=2));print(json.dumps(summary),flush=True)

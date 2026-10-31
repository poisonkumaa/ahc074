from concurrent.futures import ThreadPoolExecutor,as_completed
from pathlib import Path
import json,re,subprocess,sys
root=Path(__file__).resolve().parents[2]
solver=root/'tools/experiments'/sys.argv[1]
inputs=root/sys.argv[2]
count=int(sys.argv[3]);workers=int(sys.argv[4]);args=sys.argv[5:]
def run(seed):
    r=subprocess.run([str(root/'tools/target/release/tester'),str(solver),*args],input=(inputs/f'{seed:04d}.txt').read_bytes(),stdout=subprocess.PIPE,stderr=subprocess.PIPE,check=True,timeout=180)
    err=r.stderr.decode();score=int(re.findall(r'Score = (\d+)',err)[-1])
    tokens=r.stdout.split();at=192;moves=maximum=0
    for _ in range(1024):
        n=int(tokens[at]);assert 0<=n<=384;at+=1+2*n;moves+=n;maximum=max(maximum,n)
    assert at==len(tokens) and score==moves,(seed,err)
    def number(pattern,default=None):
        vals=re.findall(pattern,err);return float(vals[-1]) if vals else default
    return dict(seed=seed,score=score,errors=0,cpu=number(r'FinishCPU=([\d.]+)'),prep=number(r'PrepCPU=([\d.]+)'),repetitions=number(r'AdaptiveR=(\d+)'),snapshot=number(r'snapshot=([\d.]+)'),forward=number(r'forward=([\d.]+)'),initial_complete=bool(re.search(r'refit .* accepted=256 rejected=0',err)),refit_present='exact_refit accepted=' in err,max_moves=maximum,log=err)
rows=[]
with ThreadPoolExecutor(max_workers=workers) as pool:
    futures=[pool.submit(run,i) for i in range(count)]
    for future in as_completed(futures):
        rows.append(future.result())
        if len(rows)%10==0:print(f'completed={len(rows)} mean={sum(x["score"] for x in rows)/len(rows):.2f}',flush=True)
rows.sort(key=lambda x:x['seed'])
summary=dict(count=count,mean=sum(x['score'] for x in rows)/count,errors=0,max_cpu=max(x['cpu'] or 0 for x in rows),max_prep=max(x['prep'] or 0 for x in rows),min_cpu=min(x['cpu'] or 0 for x in rows),average_cpu=sum(x['cpu'] or 0 for x in rows)/count,initial_complete=sum(x['initial_complete'] for x in rows),refit_present=sum(x['refit_present'] for x in rows),repetitions={str(r):sum(x['repetitions']==r for x in rows) for r in sorted(set(x['repetitions'] or 0 for x in rows))})
output=root/'tools/experiments'/(solver.name+'_'+inputs.parent.name+'_'+'_'.join(args)+'_w'+str(workers)+'_detail.json')
output.write_text(json.dumps(dict(summary=summary,cases=rows),indent=2))
print(json.dumps(summary),flush=True)

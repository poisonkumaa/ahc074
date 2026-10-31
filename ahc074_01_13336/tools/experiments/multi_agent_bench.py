from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import json,re,subprocess,sys
root=Path(__file__).resolve().parents[2]
solver=root/'tools/experiments'/sys.argv[1]
inputs=root/sys.argv[2]
count=int(sys.argv[3]);args=sys.argv[4:]
def run(i):
    r=subprocess.run([str(root/'tools/target/release/tester'),str(solver),*args],input=(inputs/f'{i:04d}.txt').read_bytes(),stdout=subprocess.PIPE,stderr=subprocess.PIPE,check=True,timeout=120)
    err=r.stderr.decode();score=int(re.findall(r'Score = (\d+)',err)[-1]);cpus=re.findall(r'FinishCPU=([\d.]+)',err);models=re.findall(r'Model (\d+) (\d+) CPU=([\d.]+)',err)
    tokens=r.stdout.split();at=192;moves=maximum=0
    for _ in range(1024):
        n=int(tokens[at]);assert 0<=n<=384;at+=1+2*n;moves+=n;maximum=max(maximum,n)
    assert at==len(tokens) and score==moves,(i,err)
    winner=re.findall(r'Model winner=(\d+)',err);rounds=re.findall(r'Round selected=(\d+) states=(\d+) models=(\d+) PrepCPU=([\d.]+)',err)
    return dict(seed=i,score=score,errors=0,cpu=float(cpus[-1]) if cpus else None,models=models,winner=int(rounds[-1][0]) if rounds else int(winner[-1]) if winner else None,states=int(rounds[-1][1]) if rounds else None,model_count=int(rounds[-1][2]) if rounds else None,prep_cpu=float(rounds[-1][3]) if rounds else None,max_moves=maximum)
with ThreadPoolExecutor(max_workers=3) as pool:rows=list(pool.map(run,range(count)))
summary=dict(count=count,mean=sum(x['score'] for x in rows)/count,minimum=min(x['score'] for x in rows),maximum=max(x['score'] for x in rows),errors=0,max_cpu=max(x['cpu'] or 0 for x in rows),max_prep_cpu=max(x['prep_cpu'] or 0 for x in rows),min_states=min(x['states'] or 0 for x in rows),model_counts={str(w):sum(x['model_count']==w for x in rows) for w in sorted(set(x['model_count'] or 0 for x in rows))},winners={str(w):sum(x['winner']==w for x in rows) for w in sorted(set(x['winner'] or 0 for x in rows))})
output=root/'tools/experiments'/(solver.name+'_'+inputs.parent.name+'_'+'_'.join(args)+'_results.json')
output.write_text(json.dumps(dict(summary=summary,cases=rows),indent=2));print(json.dumps(summary),flush=True)

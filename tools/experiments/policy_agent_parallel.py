import subprocess,time,re,json,sys,concurrent.futures
from pathlib import Path
root=Path(__file__).resolve().parents[2]
name=sys.argv[1]
workers=int(sys.argv[2]) if len(sys.argv)>2 else 4
def run(seed):
    tic=time.perf_counter()
    p=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments'/name)],input=(root/f'tools/in/{seed:04}.txt').read_text(),text=True,capture_output=True)
    elapsed=time.perf_counter()-tic
    m=re.search(r'Score = (\d+)',p.stderr)
    if not m: raise RuntimeError((seed,p.stderr))
    score=int(m.group(1));lines=p.stdout.splitlines();idx=192;ops=0;max_ops=0
    for _ in range(1024):
        n=int(lines[idx]);ops+=n;max_ops=max(max_ops,n);idx+=n+1
    assert idx==len(lines),(seed,idx,len(lines))
    assert score==ops,(seed,score,ops)
    cpu=re.search(r'FinishCPU=([\d.]+)',p.stderr)
    r=dict(seed=seed,score=score,time=elapsed,max_ops=max_ops,cpu=float(cpu.group(1)) if cpu else None)
    print(seed,score,round(elapsed,4),r['cpu'],flush=True)
    return r
with concurrent.futures.ThreadPoolExecutor(workers) as ex:
    results=list(ex.map(run,range(100)))
(root/'tools/experiments'/f'{name}_100results.json').write_text(json.dumps(results,indent=2))
print('mean',sum(r['score'] for r in results)/100,'cpu max',max(r['cpu'] or 0 for r in results),flush=True)

import subprocess, time, re, json, sys
from pathlib import Path
root=Path(__file__).resolve().parents[2]
binary=root/'tools/experiments'/sys.argv[1]
start=int(sys.argv[2]) if len(sys.argv)>2 else 0
end=int(sys.argv[3]) if len(sys.argv)>3 else 10
results=[]
for seed in range(start,end):
    tic=time.perf_counter()
    p=subprocess.run([str(root/'tools/target/release/tester'),str(binary)],input=(root/f'tools/in/{seed:04}.txt').read_text(),text=True,capture_output=True)
    elapsed=time.perf_counter()-tic
    m=re.search(r'Score = (\d+)',p.stderr)
    if not m: raise RuntimeError(p.stderr)
    score=int(m.group(1))
    # The official tester verifies legality; separately check zero errors.
    lines=p.stdout.splitlines(); idx=192; moves=0
    for _ in range(1024):
        n=int(lines[idx]); moves+=n; idx+=n+1
    assert idx==len(lines),(idx,len(lines))
    assert score==moves,(score,moves)
    results.append(dict(seed=seed,score=score,time=elapsed))
    print(seed,score,round(elapsed,4),flush=True)
path=root/'tools/experiments'/f'{sys.argv[1]}_results.json'
path.write_text(json.dumps(results,indent=2))
print('mean',sum(v['score'] for v in results)/len(results),'time',sum(v['time'] for v in results)/len(results),flush=True)

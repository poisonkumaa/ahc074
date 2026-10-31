from pathlib import Path
import subprocess,re,json,concurrent.futures,sys
root=Path(__file__).resolve().parents[2]
def run(job):
 seed,q=job
 p=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/round6_policy_quantile'),str(q),'8'],input=(root/f'tools/in/{seed:04}.txt').read_bytes(),capture_output=True)
 assert p.returncode==0,(seed,q,p.stderr.decode());err=p.stderr.decode();score=int(re.search(r'Score = (\d+)',err)[1]);lines=p.stdout.splitlines();at=192;total=0
 for r in range(1024):m=int(lines[at]);assert 0<=m<=384;total+=m;at+=m+1
 assert total==score and at==len(lines),err
 row=dict(seed=seed,quantile=q,score=score,savings=int(re.search(r'IntervalSavings=(\d+)',err)[1]),cpu=float(re.search(r'FinishCPU=([\d.]+)',err)[1]),diagnostics=err)
 print({k:v for k,v in row.items() if k!='diagnostics'},flush=True);return row
jobs=[(0,q) for q in [0,.005,.01,.02,.05,.1]]
with concurrent.futures.ThreadPoolExecutor(2) as pool:rows=list(pool.map(run,jobs))
(root/'tools/experiments/round6_policy_quantile_results.json').write_text(json.dumps(rows,indent=2))

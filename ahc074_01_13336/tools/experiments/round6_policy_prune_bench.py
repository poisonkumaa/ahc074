from pathlib import Path
import subprocess,re,json,concurrent.futures
root=Path(__file__).resolve().parents[2]
def run(job):
 seed,when=job
 p=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/round6_policy_prune'),'16','512',str(when)],input=(root/f'tools/in/{seed:04}.txt').read_bytes(),capture_output=True)
 assert p.returncode==0,p.stderr.decode();err=p.stderr.decode();score=int(re.search(r'Score = (\d+)',err)[1]);lines=p.stdout.splitlines();at=192;total=0
 for r in range(1024):m=int(lines[at]);assert 0<=m<=384;total+=m;at+=m+1
 assert total==score and at==len(lines),err
 row=dict(seed=seed,when=when,score=score,removed=int(re.search(r'Removed=(\d+)',err)[1]),cpu=float(re.search(r'FinishCPU=([\d.]+)',err)[1]),diagnostics=err)
 print({k:v for k,v in row.items() if k!='diagnostics'},flush=True);return row
with concurrent.futures.ThreadPoolExecutor(2) as pool:rows=list(pool.map(run,[(0,1),(1,0),(2,0)]))
(root/'tools/experiments/round6_policy_prune_results.json').write_text(json.dumps(rows,indent=2))

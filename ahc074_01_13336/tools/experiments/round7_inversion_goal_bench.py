from pathlib import Path
import concurrent.futures,json,re,subprocess,sys
root=Path(__file__).resolve().parents[2]
weights=list(map(int,sys.argv[1].split(','))) if len(sys.argv)>1 else [0,4,16]
seeds=list(map(int,sys.argv[2].split(','))) if len(sys.argv)>2 else [0]
def run(job):
 weight,seed=job
 process=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/round7_inversion_goal'),str(weight)],input=(root/f'tools/in/{seed:04}.txt').read_bytes(),capture_output=True)
 err=process.stderr.decode();assert process.returncode==0,(weight,seed,err)
 score=int(re.search(r'Score = (\d+)',err)[1]);lines=process.stdout.splitlines();at=192;total=0
 for _ in range(1024):
  moves=int(lines[at]);assert 0<=moves<=384;total+=moves;at+=moves+1
 assert total==score and at==len(lines),(weight,seed,err)
 row=dict(weight=weight,seed=seed,score=score,inversions=float(re.search(r'MeanInversions=([\d.]+)',err)[1]),extra=int(re.search(r'ExtraMoves=(\d+)',err)[1]),cpu=float(re.search(r'FinishCPU=([\d.]+)',err)[1]),diagnostics=err)
 print({k:v for k,v in row.items() if k!='diagnostics'},flush=True);return row
with concurrent.futures.ThreadPoolExecutor(2) as pool:rows=list(pool.map(run,[(w,s)for w in weights for s in seeds]))
for weight in weights:
 cases=[r for r in rows if r['weight']==weight];print('mean',weight,sum(c['score'] for c in cases)/len(cases),flush=True)
(root/'tools/experiments/round7_inversion_goal_results.json').write_text(json.dumps(rows,indent=2))

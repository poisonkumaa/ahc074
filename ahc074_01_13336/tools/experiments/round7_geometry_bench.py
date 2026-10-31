from pathlib import Path
import subprocess,re,json,concurrent.futures
root=Path(__file__).resolve().parents[2]
def run(job):
 args,seed=job
 p=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/round7_geometry'),*map(str,args)],input=(root/f'tools/in/{seed:04}.txt').read_bytes(),capture_output=True)
 assert p.returncode==0,(args,seed,p.stderr.decode());err=p.stderr.decode();score=int(re.search(r'Score = (\d+)',err)[1]);lines=p.stdout.splitlines();at=192;total=0
 for r in range(1024):m=int(lines[at]);assert 0<=m<=384;total+=m;at+=m+1
 assert total==score and at==len(lines),err
 row=dict(seed=seed,args=args,score=score,cpu=float(re.search(r'FinishCPU=([\d.]+)',err)[1]),diagnostics=err);print({k:v for k,v in row.items() if k!='diagnostics'},flush=True);return row
rows=[]
for args in [(0,0),(1,1000000),(1,2),(2,1000000),(3,1000000),(3,2)]:
 with concurrent.futures.ThreadPoolExecutor(2) as pool:cases=list(pool.map(run,[(args,s) for s in range(3)]))
 print('mean',args,sum(x['score'] for x in cases)/len(cases),flush=True);rows.extend(cases)
 (root/'tools/experiments/round7_geometry_results.json').write_text(json.dumps(rows,indent=2))

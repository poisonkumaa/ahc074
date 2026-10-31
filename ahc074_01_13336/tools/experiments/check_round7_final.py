from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import subprocess,json,re,hashlib
root=Path(__file__).resolve().parents[2];d=root/'tools/experiments'
digest=hashlib.sha256((root/'main.cpp').read_bytes()).hexdigest();binaryhash=hashlib.sha256((root/'a.out').read_bytes()).hexdigest()
def run(item):
 folder,seed,index=item
 p=subprocess.run([str(root/'tools/target/release/tester'),str(root/'a.out')],input=(d/folder/f'in/{index:04d}.txt').read_bytes(),capture_output=True,check=True,timeout=90)
 err=p.stderr.decode();score=int(re.search(r'Score = (\d+)',err)[1]);tokens=p.stdout.split();at=192;total=maximum=0
 for _ in range(1024):m=int(tokens[at]);assert 0<=m<=384;total+=m;maximum=max(maximum,m);at+=1+2*m
 assert at==len(tokens) and total==score
 return dict(seed=seed,score=score,errors=0,max_moves=maximum,cpu=float(re.search(r'FinishCPU=([\d.]+)',err)[1]),prep_cpu=float(re.search(r'PrepCPU=([\d.]+)',err)[1]),fallbacks=int(re.search(r'PathFallbacks=(\d+)',err)[1]))
report=dict(source_sha256=digest,binary_sha256=binaryhash,batches=[],cases=[])
for folder,begin in [('final_holdout',5000),('multi_fresh6000',6000)]:
 assert list(map(int,(d/folder/'seeds.txt').read_text().split()))==list(range(begin,begin+50))
 with ThreadPoolExecutor(max_workers=3) as pool:rows=list(pool.map(run,[(folder,begin+i,i) for i in range(50)]))
 summary=dict(folder=folder,first_seed=begin,count=50,mean=sum(r['score'] for r in rows)/50,max_cpu=max(r['cpu'] for r in rows),max_moves=max(r['max_moves'] for r in rows),fallbacks=sum(r['fallbacks'] for r in rows),errors=0)
 print(json.dumps(summary),flush=True);report['batches'].append(summary);report['cases'].extend(rows)
 (d/'round7_final_validation.json').write_text(json.dumps(report,indent=2))
assert hashlib.sha256((root/'main.cpp').read_bytes()).hexdigest()==digest and hashlib.sha256((root/'a.out').read_bytes()).hexdigest()==binaryhash

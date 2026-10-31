from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import hashlib,json,re,subprocess

here=Path(__file__).resolve().parent
root=here.parents[2]
here.mkdir(exist_ok=True)
seeds=list(range(5000,5050))
(here/'seeds.txt').write_text(''.join(f'{seed}\n' for seed in seeds))
subprocess.run([str(root/'tools/target/release/gen'),str(here/'seeds.txt'),'-d',str(here/'in')],check=True)

def run(index):
    result=subprocess.run([str(root/'tools/target/release/tester'),str(root/'a.out')],input=(here/f'in/{index:04d}.txt').read_bytes(),stdout=subprocess.PIPE,stderr=subprocess.PIPE,check=True,timeout=60)
    error=result.stderr.decode()
    score=int(re.search(r'Score = (\d+)',error)[1])
    cpu=float(re.search(r'FinishCPU=([\d.]+)',error)[1])
    tokens=result.stdout.split();at=192;moves=0;maximum=0
    for _ in range(1024):
        count=int(tokens[at]);assert 0<=count<=384
        moves+=count;maximum=max(maximum,count);at+=1+2*count
    assert at==len(tokens) and moves==score,(seeds[index],error)
    return dict(seed=seeds[index],score=score,errors=0,cpu=cpu,max_moves=maximum)

with ThreadPoolExecutor(max_workers=3) as pool:cases=list(pool.map(run,range(len(seeds))))
summary=dict(count=len(cases),mean=sum(c['score'] for c in cases)/len(cases),minimum=min(c['score'] for c in cases),maximum=max(c['score'] for c in cases),errors=sum(c['errors'] for c in cases),max_cpu=max(c['cpu'] for c in cases),max_moves=max(c['max_moves'] for c in cases),source_sha256=hashlib.sha256((root/'main.cpp').read_bytes()).hexdigest())
(here/'results.json').write_text(json.dumps(dict(summary=summary,cases=cases),indent=2))
print(json.dumps(summary,indent=2),flush=True)

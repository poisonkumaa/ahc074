from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import json,re,subprocess

root=Path(__file__).resolve().parents[2]
d=root/'tools/experiments'
rows=[]
for limit in ['0','0.1','0.3']:
    binary=d/f'prefix_cutoff_{limit}'
    subprocess.run(['g++','-std=c++17','-O2',f'-DSTRUCTURE2_TRAINING_CPU_LIMIT={limit}',str(root/'main.cpp'),'-o',str(binary)],check=True)
    def run(seed):
        p=subprocess.run([str(root/'tools/target/release/tester'),str(binary)],input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),capture_output=True,check=True)
        score=int(re.search(rb'Score = (\d+)',p.stderr)[1]);tokens=p.stdout.split();at=192;moves=0;maximum=0
        for _ in range(1024):
            count=int(tokens[at]);assert 0<=count<=384
            moves+=count;maximum=max(maximum,count);at+=1+2*count
        assert at==len(tokens) and moves==score,(limit,seed,p.stderr)
        return dict(limit=float(limit),seed=seed,score=score,max_moves=maximum,errors=0)
    with ThreadPoolExecutor(max_workers=3) as pool:rows.extend(pool.map(run,[0,50,99]))
(d/'prefix_cutoff_results.json').write_text(json.dumps(rows,indent=2))
print(f'Validated {len(rows)} early-cutoff cases: all legal, no incorrect orders',flush=True)

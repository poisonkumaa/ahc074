from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import hashlib,json,re,subprocess
p=Path(__file__).parent;root=p.parents[1]
def run(item):
    seed,repetitions=item;data=(root/f'tools/in/{seed:04d}.txt').read_bytes();rows=[];outputs=[]
    for name in ['round6_initial_reference','round6_initial_fast_diag']+(['round6_initial_fast_check'] if repetitions==8 else []):
        r=subprocess.run([str(p/name),str(repetitions)],input=data,stdout=subprocess.PIPE,stderr=subprocess.PIPE,check=True,timeout=120)
        outputs.append(r.stdout);log=r.stderr.decode();rows.append(dict(name=name,prep=float(re.findall(r'PrepCPU=([\d.]+)',log)[-1]),cpu=float(re.findall(r'FinishCPU=([\d.]+)',log)[-1]),sha256=hashlib.sha256(r.stdout).hexdigest(),log=log))
    assert all(x==outputs[0] for x in outputs),(seed,repetitions,rows)
    return dict(seed=seed,repetitions=repetitions,identical=True,runs=rows)
with ThreadPoolExecutor(max_workers=3) as pool:rows=list(pool.map(run,[(s,r) for r in [8,32] for s in range(5)]))
(p/'round6_initial_compare.json').write_text(json.dumps(rows,indent=2))
for r in [8,32]:
    selected=[x for x in rows if x['repetitions']==r]
    print(json.dumps(dict(repetitions=r,identical=len(selected),reference_prep=sum(x['runs'][0]['prep'] for x in selected)/len(selected),fast_prep=sum(x['runs'][1]['prep'] for x in selected)/len(selected),reference_cpu=sum(x['runs'][0]['cpu'] for x in selected)/len(selected),fast_cpu=sum(x['runs'][1]['cpu'] for x in selected)/len(selected))),flush=True)

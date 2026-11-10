"""Paired checks spread across all 500 cases; not a ranking estimate."""
from pathlib import Path
import hashlib
import json
import re
import subprocess

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
seeds = [0,25,75,125,150,175,200,225,250,275,300,350,400,450,499]
variants = ['local_native','judge_unified','judge_explicit_fma']
rows=[]
for seed in seeds:
    current=[]
    for name in variants:
        stem=f'sample_{name}_{seed:04d}'
        with (ROOT/f'tools/in/{seed:04d}.txt').open('rb') as inp,(HERE/(stem+'.out')).open('wb') as out,(HERE/(stem+'.log')).open('wb') as err:
            p=subprocess.run([str(ROOT/'tools/target/release/tester'),str(HERE/name)],stdin=inp,stdout=out,stderr=err,timeout=30)
        data=(HERE/(stem+'.out')).read_bytes()
        log=(HERE/(stem+'.log')).read_text()
        scores=re.findall(r'Score\s*=\s*(\d+)',log)
        row=dict(seed=seed,variant=name,rc=p.returncode,score=int(scores[-1]) if scores else None,output_sha256=hashlib.sha256(data).hexdigest(),design_sha256=hashlib.sha256(b'\n'.join(data.splitlines()[:192])).hexdigest(),chosen=[s for s in log.splitlines() if s.startswith('CHOSEN')])
        current.append(row);rows.append(row)
    print(json.dumps(current),flush=True)
    (HERE/'sample_results.json').write_text(json.dumps(rows,indent=2))

from pathlib import Path
import concurrent.futures
import hashlib
import json
import re
import subprocess
import time

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
base = (HERE/'snapshot.cpp').read_text()
unified = base.replace('#if defined(__GNUC__) && !defined(ONLINE_JUDGE) && !defined(ATCODER)\n#pragma GCC optimize("O3")', '#if defined(__GNUC__) && !defined(__clang__)\n#pragma GCC optimize("O3")')
(HERE/'unified.cpp').write_text(unified)
fma = (HERE/'snapshot_o1.cpp').read_text().replace('return alpha*(pair_price*p*(p+1.0)+single_price*(s*p+s*(s+1.0)/2)+mixed);', 'return alpha*(std::fma(pair_price*p,p+1.0,single_price*(s*p+s*(s+1.0)/2))+mixed);')
(HERE/'explicit_fma.cpp').write_text(fma)
variants = {'judge_unified':'unified.cpp', 'judge_explicit_fma':'explicit_fma.cpp'}

def build(item):
    name, src = item
    cmd = ['g++','-std=c++20','-O2','-march=native','-DONLINE_JUDGE','-DATCODER', str(HERE/src),'-o',str(HERE/name)]
    t = time.monotonic()
    p = subprocess.run(cmd,capture_output=True,text=True)
    (HERE/(name+'.build.log')).write_text(p.stderr)
    print(json.dumps(dict(build=name,rc=p.returncode,seconds=round(time.monotonic()-t,2))),flush=True)
    return name,p.returncode

with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
    built = list(pool.map(build,variants.items()))

rows=[]
for name,rc in [('local_native',0), *built]:
    if rc: continue
    for repeat in range(2):
        stem = f'{name}_fixcheck_{repeat}'
        with (ROOT/'tools/in/0000.txt').open('rb') as inp,(HERE/(stem+'.out')).open('wb') as out,(HERE/(stem+'.log')).open('wb') as err:
            t=time.monotonic()
            p=subprocess.run([str(ROOT/'tools/target/release/tester'),str(HERE/name)],stdin=inp,stdout=out,stderr=err,timeout=30)
        output=(HERE/(stem+'.out')).read_bytes()
        log=(HERE/(stem+'.log')).read_text()
        row=dict(variant=name,repeat=repeat,rc=p.returncode,wall=round(time.monotonic()-t,3),score=re.findall(r'Score\s*=\s*(\d+)',log),sha256=hashlib.sha256(output).hexdigest(),design_sha256=hashlib.sha256(b'\n'.join(output.splitlines()[:192])).hexdigest(),chosen=[line for line in log.splitlines() if line.startswith(('CHOSEN','DUAL101'))])
        rows.append(row)
        print(json.dumps(row),flush=True)
        (HERE/'fix_results.json').write_text(json.dumps(rows,indent=2))

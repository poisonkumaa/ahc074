"""Isolated compiler comparison; original solver/config/output remain untouched."""
from pathlib import Path
import concurrent.futures
import hashlib
import json
import re
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
source = (ROOT / 'main.cpp').read_text()
(HERE / 'snapshot.cpp').write_text(source)
o1 = source.replace('// #if defined(__GNUC__) && !defined(__clang__) && (defined(ONLINE_JUDGE) || defined(ATCODER))', '#if defined(__GNUC__) && !defined(__clang__) && (defined(ONLINE_JUDGE) || defined(ATCODER))').replace('//     __attribute__((optimize("O1")))', '    __attribute__((optimize("O1")))').replace('// #endif', '#endif')
(HERE / 'snapshot_o1.cpp').write_text(o1)
variants = {
    'local_native': ('snapshot.cpp', ['-march=native']),
    'local_generic': ('snapshot.cpp', []),
    'local_no_contract': ('snapshot.cpp', ['-march=native', '-ffp-contract=off']),
    'judge_native': ('snapshot.cpp', ['-march=native', '-DONLINE_JUDGE', '-DATCODER']),
    'judge_o1': ('snapshot_o1.cpp', ['-march=native', '-DONLINE_JUDGE', '-DATCODER']),
}

def build(item):
    name, (src, flags) = item
    cmd = ['g++', '-std=c++20', '-O2', *flags, str(HERE/src), '-o', str(HERE/name)]
    t = time.monotonic()
    p = subprocess.run(cmd, capture_output=True, text=True)
    (HERE / (name+'.build.log')).write_text(p.stderr)
    print(json.dumps({'build': name, 'rc': p.returncode, 'seconds': round(time.monotonic()-t, 2)}), flush=True)
    return name, p.returncode, cmd

with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
    builds = list(pool.map(build, variants.items()))

rows = []
for name, rc, cmd in builds:
    if rc:
        continue
    for seed in [0]:
        for repeat in range(2):
            stem = f'{name}_{seed:04d}_{repeat}'
            t = time.monotonic()
            with (ROOT / f'tools/in/{seed:04d}.txt').open('rb') as inp, (HERE / (stem+'.out')).open('wb') as out, (HERE / (stem+'.log')).open('wb') as err:
                p = subprocess.run([str(ROOT/'tools/target/release/tester'), str(HERE/name)], stdin=inp, stdout=out, stderr=err, timeout=30)
            log = (HERE/(stem+'.log')).read_text()
            output = (HERE/(stem+'.out')).read_bytes()
            row = dict(variant=name, seed=seed, repeat=repeat, rc=p.returncode, wall=round(time.monotonic()-t, 3), score=re.findall(r'Score\s*=\s*(\d+)',log), output_sha256=hashlib.sha256(output).hexdigest(), design_sha256=hashlib.sha256(b'\n'.join(output.splitlines()[:192])).hexdigest(), selected=[line for line in log.splitlines() if line.startswith(('CHOSEN','DUAL101','TOTAL','NOV8_BUDGET','RESCUE30','COVER_SIBLING','PREFIX_COLUMN_ORDER','SPLIT','MATCH '))])
            rows.append(row)
            print(json.dumps(row), flush=True)
            (HERE/'results.json').write_text(json.dumps({'compiler': subprocess.check_output(['g++','--version'],text=True).splitlines()[0], 'builds': builds, 'runs': rows}, indent=2))

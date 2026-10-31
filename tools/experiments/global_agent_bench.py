import concurrent.futures
import json
import pathlib
import re
import subprocess
import sys
import time

root = pathlib.Path(__file__).resolve().parents[2]
solver = root / 'tools/experiments' / (sys.argv[1] if len(sys.argv)>1 else 'global_agent_record_submit')
solver_args = sys.argv[3:]
tester = root / 'tools/target/release/tester'

def run(seed):
    started = time.monotonic()
    data = (root / f'tools/in/{seed:04d}.txt').read_bytes()
    p = subprocess.run([str(tester), str(solver), *solver_args], input=data,
                       stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=90)
    err = p.stderr.decode()
    score = int(re.findall(r'Score = (\d+)', err)[-1])
    assert p.returncode == 0 and score > 0, (seed, err)
    tokens = p.stdout.split()
    at = 192
    moves = maximum = 0
    for r in range(1024):
        m = int(tokens[at])
        assert 0 <= m <= 384, (seed, r, m)
        at += 1 + 2 * m
        moves += m
        maximum = max(maximum, m)
    assert at == len(tokens), (seed, at, len(tokens))
    assert score == moves, (seed, score, moves, err)
    return dict(seed=seed, score=score, moves=moves, errors=0,
                max_moves=maximum, elapsed=time.monotonic()-started)

with concurrent.futures.ThreadPoolExecutor(max_workers=6) as pool:
    rows = list(pool.map(run, range(int(sys.argv[2]) if len(sys.argv)>2 else 100)))
result = dict(mean=sum(r['score'] for r in rows)/len(rows),
              minimum=min(r['score'] for r in rows),
              maximum=max(r['score'] for r in rows), cases=rows)
suffix = ('_'+'_'.join(solver_args)) if solver_args else ''
out = root / 'tools/experiments' / (solver.name+suffix+'_results.json')
out.write_text(json.dumps(result, indent=2))
print(json.dumps({k: v for k, v in result.items() if k != 'cases'}), flush=True)

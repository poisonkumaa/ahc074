from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import json
import re
import subprocess
import sys

root = Path(__file__).resolve().parents[2]
iterations = int(sys.argv[1]) if len(sys.argv) > 1 else 60000
cases = int(sys.argv[2]) if len(sys.argv) > 2 else 20

def run(seed):
    result = subprocess.run(
        [str(root / 'tools/target/release/tester'), str(root / 'tools/experiments/direct_groups'), str(iterations)],
        input=(root / f'tools/in/{seed:04d}.txt').read_text(), text=True,
        capture_output=True, cwd=root, check=True)
    score = int(re.search(r'Score = (\d+)', result.stderr)[1])
    prep = float(re.search(r'prep=([0-9.]+)', result.stderr)[1])
    tokens = result.stdout.split()
    at = 192
    moves = 0
    for _ in range(1024):
        count = int(tokens[at]); assert 0 <= count <= 384
        moves += count; at += 1 + 2 * count
    assert at == len(tokens) and score == moves and score > 0
    baseline = int(re.search(r'Score = (\d+)', (root / f'tools/err/{seed:04d}.txt').read_text())[1])
    return dict(seed=seed, score=score, previous=baseline, prep_parallel=prep)

with ThreadPoolExecutor(max_workers=4) as pool:
    results = list(pool.map(run, range(cases)))
summary = dict(iterations=iterations, cases=cases,
               mean=sum(r['score'] for r in results)/cases,
               previous_mean=sum(r['previous'] for r in results)/cases,
               wins=sum(r['score'] < r['previous'] for r in results))
print(json.dumps(summary, indent=2))
(root / f'tools/experiments/direct_bench_{iterations}.json').write_text(json.dumps(dict(summary=summary, cases=results), indent=2))

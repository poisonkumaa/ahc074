import json
import hashlib
import re
from pathlib import Path

root = Path(__file__).resolve().parents[2]
rows = []
for seed in range(100):
    data = (root / f'tools/in/{seed:04d}.txt').read_text().split()
    n, k, p, r = map(int, data[:4])
    products = [int(s, 2) for s in data[4:4 + p]]
    orders = list(map(int, data[4 + p:]))
    previous = 0
    baseline = 0
    for a in orders:
        baseline += (previous ^ products[a]).bit_count()
        previous = products[a]
    output = (root / f'tools/out/{seed:04d}.txt').read_text().split()
    position, total, max_moves = k, 0, 0
    for _ in range(r):
        count = int(output[position])
        assert 0 <= count <= 2 * k
        total += count
        max_moves = max(max_moves, count)
        position += 1 + count * 2
    assert position == len(output)
    error_log = (root / f'tools/err/{seed:04d}.txt').read_text()
    score = int(re.search(r'Score\s*=\s*(\d+)', error_log)[1])
    assert score > 0 and score >= total and (score - total) % (2 * k) == 0
    cpu_match = re.search(r'FinishCPU=([\d.]+)', error_log)
    rows.append(dict(seed=seed, baseline=baseline, score=score,
                     errors=(score - total) // (2 * k), max_moves=max_moves,
                     cpu_seconds=float(cpu_match[1]) if cpu_match else None))
result = dict(cases=len(rows), baseline_mean=sum(x['baseline'] for x in rows)/len(rows),
              score_mean=sum(x['score'] for x in rows)/len(rows),
              min_score=min(x['score'] for x in rows), max_score=max(x['score'] for x in rows),
              errors=sum(x['errors'] for x in rows),
              improved_cases=sum(x['score'] < x['baseline'] for x in rows),
              max_operations_per_order=max(x['max_moves'] for x in rows),
              max_cpu_seconds=max((x['cpu_seconds'] for x in rows if x['cpu_seconds'] is not None), default=None),
              source_sha256=hashlib.sha256((root/'main.cpp').read_bytes()).hexdigest())
print(json.dumps(result, indent=2))
(root / 'tools/experiments/validation.json').write_text(json.dumps(dict(summary=result, cases=rows), indent=2))

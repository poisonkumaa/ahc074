"""Paired sequential comparisons, with independent replay of every operation."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import statistics

ROOT = Path(__file__).resolve().parents[2]
os.chdir(ROOT)
helper = ROOT.parent / 'archived2_11434/experiments/bench_run.py'
spec = importlib.util.spec_from_file_location('reference_bench', helper)
reference = importlib.util.module_from_spec(spec)
spec.loader.exec_module(reference)

parser = argparse.ArgumentParser()
parser.add_argument('--spec', type=Path, required=True)
parser.add_argument('--seeds', default='0,1,2,3,4,5,6,7,8,9')
parser.add_argument('--input-dir', type=Path, default=ROOT / 'tools/in')
parser.add_argument('--report', type=Path, required=True)
parser.add_argument('--official', action='store_true')
parser.add_argument('--rotate', action='store_true', help='Balance execution order across seeds')
args = parser.parse_args()
variants = json.loads(args.spec.read_text(encoding='utf-8-sig'))
original_env = dict(os.environ)
if os.name == 'nt':
    original_env['PATH'] = r'C:\Users\dokuk\mingw64\bin' + os.pathsep + original_env['PATH']
report = {'official': args.official, 'variants': {}, 'cases': []}
for v in variants:
    exe = Path(v['exe']).resolve()
    v['exe'] = str(exe)
    report['variants'][v['name']] = dict(v, sha256=hashlib.sha256(exe.read_bytes()).hexdigest())
args.report.parent.mkdir(parents=True, exist_ok=True)

for seed in map(int, args.seeds.split(',')):
    offset = seed % len(variants) if args.rotate else 0
    for v in variants[offset:] + variants[:offset]:
        os.environ.clear()
        os.environ.update(original_env)
        os.environ.update({k: str(value) for k, value in v.get('env', {}).items()})
        out = args.report.parent / (args.report.stem + '_out') / v['name']
        result = reference.run_case(Path(v['exe']), args.input_dir / f'{seed:04d}.txt',
                                    args.official, 20, out)
        result['variant'] = v['name']
        result['seed'] = seed
        report['cases'].append(result)
        print(f"{v['name']} {seed:04d}: T={result['moves']} E={result['errors']} "
              f"wall={result['wall_seconds']:.3f} cpu={result['solver_cpu_seconds']}", flush=True)
        args.report.write_text(json.dumps(report, indent=2), encoding='utf-8')

report['summary'] = {}
for v in variants:
    cases = [c for c in report['cases'] if c['variant'] == v['name']]
    report['summary'][v['name']] = {
        'count': len(cases),
        'mean': statistics.mean(c['score'] for c in cases),
        'min': min(c['score'] for c in cases),
        'max': max(c['score'] for c in cases),
        'errors': sum(c['errors'] for c in cases),
        'max_round_moves': max(c['max_round_moves'] for c in cases),
        'max_wall': max(c['wall_seconds'] for c in cases),
        'max_cpu': max(c['solver_cpu_seconds'] or 0 for c in cases),
    }
print(json.dumps(report['summary'], indent=2), flush=True)
args.report.write_text(json.dumps(report, indent=2), encoding='utf-8')

from pathlib import Path
import re
import subprocess

root = Path(__file__).resolve().parents[2]
source = (root / 'main.cpp').read_text()
assert 'search_cpu_budget=0.95' in source
source = source.replace('search_cpu_budget=0.95', 'search_cpu_budget=0.0')
path = root / 'tools/experiments/direct_cutoff.cpp'
binary = root / 'tools/experiments/direct_cutoff'
path.write_text(source)
subprocess.run(['g++', '-std=c++20', '-O2', str(path), '-o', str(binary)], check=True)
result = subprocess.run(
    [str(root / 'tools/target/release/tester'), str(binary)],
    input=(root / 'tools/in/0000.txt').read_text(), text=True,
    capture_output=True, cwd=root, check=True)
score = int(re.search(r'Score = (\d+)', result.stderr)[1])
tokens = result.stdout.split()
at = 192
moves = 0
for _ in range(1024):
    count = int(tokens[at])
    assert 0 <= count <= 384
    moves += count
    at += 1 + 2 * count
assert at == len(tokens) and score == moves and score > 0
print(f'Immediate search cutoff: score={score}, errors=0, valid interaction')

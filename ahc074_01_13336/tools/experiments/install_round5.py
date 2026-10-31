from pathlib import Path
import hashlib
import shutil

root = Path(__file__).resolve().parents[2]
d = root / 'tools/experiments'
old = 'b357bd95e318e736b9552672c09e718fd442f77bfb8facbb35a9a0e61569a6e9'
new = '12f6ae6570b5d322e73a2e169e7f05d7ce5058a775fd65892f7b5da6f4209142'
assert hashlib.sha256((root/'main.cpp').read_bytes()).hexdigest() == old
assert hashlib.sha256((d/'round5_sampling_adaptive.cpp').read_bytes()).hexdigest() == new
shutil.copyfile(d/'validation.json', d/'baseline_13530_validation.json')
shutil.copyfile(d/'round5_sampling_adaptive.cpp', root/'main.cpp')
source = (d/'check_round4_final.py').read_text()
(d/'check_round5_final.py').write_text(source.replace('round4_final_validation.json', 'round5_final_validation.json'))
print('Installed', new)

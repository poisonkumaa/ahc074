from pathlib import Path
import hashlib
import shutil
root=Path(__file__).resolve().parents[2]
d=root/'tools/experiments'
assert hashlib.sha256((root/'main.cpp').read_bytes()).hexdigest()=='6fad7b22d149c8652d3572d4e89ee0855732167d1b331758dee97cf75278cff3'
for original,backup in [('validation.json','round6_triefast_validation.json'),('round6_final_validation.json','round6_triefast_final_validation.json'),('round6_cutoff_results.json','round6_triefast_cutoff_results.json'),('round6_ubsan_results.json','round6_triefast_ubsan_results.json')]:
    shutil.copyfile(d/original,d/backup)
shutil.copyfile(d/'round6_calibrated.cpp',root/'main.cpp')
print('Installed',hashlib.sha256((root/'main.cpp').read_bytes()).hexdigest())

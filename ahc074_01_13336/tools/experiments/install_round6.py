from pathlib import Path
import hashlib
import shutil

root=Path(__file__).resolve().parents[2]
d=root/'tools/experiments'
old='12f6ae6570b5d322e73a2e169e7f05d7ce5058a775fd65892f7b5da6f4209142'
new='6fad7b22d149c8652d3572d4e89ee0855732167d1b331758dee97cf75278cff3'
assert hashlib.sha256((root/'main.cpp').read_bytes()).hexdigest()==old
assert hashlib.sha256((d/'round6_triefast_final.cpp').read_bytes()).hexdigest()==new
shutil.copyfile(d/'validation.json',d/'baseline_13350_validation.json')
shutil.copyfile(d/'round6_triefast_final.cpp',root/'main.cpp')
(d/'check_round6_final.py').write_text((d/'check_round5_final.py').read_text().replace('round5_final_validation.json','round6_final_validation.json'))
print('Installed',new)

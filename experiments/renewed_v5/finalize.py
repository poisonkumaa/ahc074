from pathlib import Path
import argparse
import hashlib
import json
import difflib

parser=argparse.ArgumentParser()
parser.add_argument('--apply',action='store_true')
args=parser.parse_args()
here=Path(__file__).resolve().parent
root=here.parents[1]
baseline=here/'main_baseline.cpp'
candidate=here/'popcnt_top.cpp'
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
assert digest(baseline)=='2a36bd6c8d4944aa61068d4ac85fb7773dd1f1ff59970bc84cf37512cd3278e7'
assert digest(candidate)=='19b8ebae739aaabc130ca7667d0e08aafb2a917e630f515c39050e7ae2778d88'
old=baseline.read_text(encoding='utf-8').splitlines(keepends=True)
new=candidate.read_text(encoding='utf-8').splitlines(keepends=True)
assert new[4:]==old
(here/'final_change.patch').write_text(''.join(difflib.unified_diff(old,new,fromfile='main_before.cpp',tofile='main.cpp')),encoding='utf-8')
report=json.loads((here/'hardware_validation.json').read_text())
assert all(c['errors']==0 for c in report['cases']) and len(report['cases'])==40
official=json.loads((here/'official_validation.json').read_text())
assert official['official'] and len(official['cases'])==6 and all(c['errors']==0 for c in official['cases'])
if args.apply:
    assert digest(root/'main.cpp')==digest(baseline),'User main changed since experiment start; do not overwrite.'
    (root/'main.cpp').write_bytes(candidate.read_bytes())
    assert digest(root/'main.cpp')==digest(candidate)
print('main SHA256:',digest(root/'main.cpp'))
print('candidate SHA256:',digest(candidate))

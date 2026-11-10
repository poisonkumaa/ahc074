from pathlib import Path
import hashlib
import json
import subprocess
import sys

here=Path(__file__).resolve().parent
root=here.parents[1]
names=['selection_sampling.cpp','route_unique.cpp','route_reorder.cpp','structure_candidate.cpp','core4_candidate.cpp','selection_diagnostic.cpp']
def hashes():
    return {name:hashlib.sha256((here/name).read_bytes()).hexdigest() for name in names}
before=hashes()
for generator in ['selection_make.py','route_unique_make.py','route_reorder_make.py','structure_make.py','core4_make.py','make_diagnostic.py']:
    subprocess.run([sys.executable,str(here/generator)],cwd=root,check=True)
after=hashes()
assert before==after,(before,after)
assert (root/'main.cpp').read_bytes()==(here/'main_before_v4.cpp').read_bytes()
identical={}
for report,variants in [('unique_probe',['unique']),('reorder_probe',['reorder1k','reorder10k'])]:
    for variant in variants:
        for seed in range(6):
            out=here/(report+'_out')
            assert (out/'baseline'/f'{seed:04d}.txt').read_bytes()==(out/variant/f'{seed:04d}.txt').read_bytes()
        identical[f'{report}/{variant}']=6
result=dict(source_hashes=after,main_sha256=hashlib.sha256((root/'main.cpp').read_bytes()).hexdigest(),identical_output_cases=identical)
(here/'reproduction.json').write_text(json.dumps(result,indent=2))
print('PASS: six generated sources unchanged; main unchanged; 18 paired outputs identical.')

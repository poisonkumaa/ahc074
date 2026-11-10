from pathlib import Path
import hashlib
import json
import subprocess

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]
rows=[]
for name in ['local_native','judge_o1','judge_explicit_fma']:
    stem='direct_'+name
    with (ROOT/'tools/in/0000.txt').open('rb') as inp,(HERE/(stem+'.out')).open('wb') as out,(HERE/(stem+'.log')).open('wb') as err:
        p=subprocess.run([str(HERE/name)],stdin=inp,stdout=out,stderr=err,timeout=30)
    data=(HERE/(stem+'.out')).read_bytes()
    row=dict(variant=name,rc=p.returncode,moves=(HERE/(stem+'.log')).read_text().splitlines()[0],output_sha256=hashlib.sha256(data).hexdigest())
    print(json.dumps(row),flush=True);rows.append(row)
(HERE/'direct_results.json').write_text(json.dumps(rows,indent=2))

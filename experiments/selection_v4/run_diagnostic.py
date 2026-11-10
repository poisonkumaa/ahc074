from pathlib import Path
import os
import subprocess
import json

root = Path(__file__).resolve().parents[2]
os.chdir(root)
out = root / 'experiments/selection_v4/diagnostic_logs'
out.mkdir(exist_ok=True)
env = dict(os.environ)
for phase in ['POOL','PACK','MATCH','LAYOUT','SELECT','HOLES']:
    env[f'NOV8_{phase}_END']='30'
env['NOV8_SELECT_HARD']='30'
env['PAIR_SNAPSHOT_END']='30'
env['DIAGNOSTIC_ROOT']=str(root/'experiments/selection_v4/fixtures')
for seed in range(6):
    p=subprocess.run([str(root/'experiments/selection_v4/selection_diagnostic_linux')],
        input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),
        stdout=subprocess.PIPE,stderr=subprocess.PIPE,env=env,timeout=40)
    (out/f'{seed:04d}.stderr').write_bytes(p.stderr)
    assert p.returncode==0, p.stderr[-1000:]
    lines=p.stderr.decode().splitlines()
    print(seed, [x for x in lines if x.startswith('DIAGNOSTIC_COMPLETE')],flush=True)
    assert any('candidates=8' in x for x in lines if x.startswith('DIAGNOSTIC_COMPLETE'))
(out/'env.json').write_text(json.dumps({k:v for k,v in env.items() if k.startswith(('NOV8_','PAIR_','DIAGNOSTIC_'))},indent=2))

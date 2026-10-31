from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import subprocess, re, json
root=Path(__file__).resolve().parents[2]
def run(solver,args,seed):
    p=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments'/solver),*map(str,args)],input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),capture_output=True,check=True)
    return p.stdout,p.stderr.decode()
def check(seed):
    a,alog=run('round7_fair',[0,8,0,0],seed)
    b,blog=run('round7_controlfull',[0,8],seed)
    assert a==b,(seed,'beta0 output mismatch')
    return dict(seed=seed,bytes=len(a),score=int(re.search(r'Score = (\d+)',blog)[1]),equal=True)
with ThreadPoolExecutor(2) as pool:
    rows=list(pool.map(check,range(3)))
for row in rows:print(row,flush=True)
check_output,check_log=run('round7_controlfull_check',[1,8],0)
regular_output,regular_log=run('round7_controlfull',[1,8],0)
assert check_output==regular_output
print('Every insertion delta check passed; checked output equals regular beta1.',flush=True)
(root/'tools/experiments/round7_controlfull_verify.json').write_text(json.dumps(dict(beta0=rows,delta_check=True,beta1_score=int(re.search(r'Score = (\d+)',check_log)[1])),indent=2))

import subprocess,re,json,concurrent.futures
from pathlib import Path
root=Path(__file__).resolve().parents[2]
def run(seed):
 r=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/policy4_agent_first_compare')],input=(root/f'tools/in/{seed:04}.txt').read_text(),capture_output=True,text=True)
 score=int(re.search(r'Score = (\d+)',r.stderr)[1]);teacher=int(re.search(r'TeacherDifferent=(\d+)',r.stderr)[1]);different=int(re.search(r'\nDifferent=(\d+)',r.stderr)[1]);cpu=float(re.search(r'FinishCPU=([\d.]+)',r.stderr)[1]);assert not teacher and not different,(seed,r.stderr)
 ls=r.stdout.splitlines();i=192;ops=0
 for _ in range(1024):m=int(ls[i]);ops+=m;i+=1+m
 assert ops==score and i==len(ls),(seed,r.stderr)
 if seed%10==0:print(seed,score,cpu,flush=True)
 return dict(seed=seed,score=score,cpu=cpu,teacher_difference=teacher,online_difference=different)
with concurrent.futures.ThreadPoolExecutor(3) as pool:out=list(pool.map(run,range(100)))
(root/'tools/experiments/policy4_agent_first_stack100.json').write_text(json.dumps(out,indent=2))
print('all terminal stacks equal','mean',sum(x['score'] for x in out)/100,flush=True)

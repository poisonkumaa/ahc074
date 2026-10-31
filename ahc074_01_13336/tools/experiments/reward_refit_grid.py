from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import json,subprocess,re,sys
d=Path(__file__).resolve().parent;root=d.parents[1]
s=(d/'goal_reward_fast.cpp').read_text().replace('int main(){','int main(int argc,char**argv){')
s=s.replace('refit_all_prefix_papers(A,papers,1,8);','refit_all_prefix_papers(A,papers,argc>1?atoi(argv[1]):1,argc>2?atoi(argv[2]):8);')
(d/'reward_refit_grid.cpp').write_text(s)
subprocess.run(['g++','-std=c++20','-O2',str(d/'reward_refit_grid.cpp'),'-o',str(d/'reward_refit_grid')],check=True)
configs=[(1,8),(2,8),(4,8),(8,8),(1,32),(2,32),(4,32),(8,32),(2,1),(4,1),(8,1)]
def run(job):
    config,seed=job
    p=subprocess.run([str(root/'tools/target/release/tester'),str(d/'reward_refit_grid'),*map(str,config)],input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),capture_output=True)
    err=p.stderr.decode();m=re.search(r'Score = (\d+)',err);assert p.returncode==0 and m,(job,err)
    score=int(m[1]);at=192;total=0;tokens=p.stdout.split()
    for _ in range(1024):count=int(tokens[at]);total+=count;at+=1+2*count
    assert total==score and at==len(tokens)
    return dict(config=config,seed=seed,score=score,cpu=float(re.search(r'FinishCPU=([\d.]+)',err)[1]),intrinsic=float(re.search(r'refit [\d.]+ -> ([\d.]+)',err)[1]))
with ThreadPoolExecutor(max_workers=3) as pool:
    rows=[]
    for r in pool.map(run,[(c,s) for c in configs for s in range(3)]):
        rows.append(r)
        if r['seed']==2:
            rr=[x for x in rows if x['config']==r['config']];print(r['config'],sum(x['score'] for x in rr)/len(rr),max(x['cpu'] for x in rr),sum(x['intrinsic'] for x in rr)/len(rr),flush=True)
(d/'reward_refit_grid.json').write_text(json.dumps(rows,indent=2))

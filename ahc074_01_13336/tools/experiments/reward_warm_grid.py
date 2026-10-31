from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import json,subprocess,re
d=Path(__file__).resolve().parent;root=d.parents[1]
s=(d/'refit4_reward.cpp').read_text().replace('int main(){','int main(int argc,char**argv){')
s=s.replace('int samples=64;', 'int warm=argc>1?max(0,atoi(argv[1])):128;int samples=64;')
s=s.replace('r<128+samples', 'r<warm+samples').replace('r<128?','r<warm?').replace('r-128','r-warm').replace('r>=128','r>=warm')
s=s.replace('for(int t=0;t<moves;t++){auto[from,to]', 'for(int t=0;t<moves&&!(argc>1&&atoi(argv[1])<0);t++){auto[from,to]')
(d/'reward_warm_grid.cpp').write_text(s)
subprocess.run(['g++','-std=c++20','-O2',str(d/'reward_warm_grid.cpp'),'-o',str(d/'reward_warm_grid')],check=True)
configs=[128,64,32,0,-1]
def run(job):
    config,seed=job
    p=subprocess.run([str(root/'tools/target/release/tester'),str(d/'reward_warm_grid'),str(config)],input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),capture_output=True)
    err=p.stderr.decode();m=re.search(r'Score = (\d+)',err);assert p.returncode==0 and m,(job,err)
    score=int(m[1]);at=192;total=0;tokens=p.stdout.split()
    for _ in range(1024):count=int(tokens[at]);total+=count;at+=1+2*count
    assert total==score and at==len(tokens)
    return dict(config=config,seed=seed,score=score,cpu=float(re.search(r'FinishCPU=([\d.]+)',err)[1]))
with ThreadPoolExecutor(max_workers=3) as pool:
    rows=[]
    for r in pool.map(run,[(c,s) for c in configs for s in range(10)]):
        rows.append(r)
        if r['seed']==9:
            rr=[x for x in rows if x['config']==r['config']];print(r['config'],sum(x['score'] for x in rr)/len(rr),max(x['cpu'] for x in rr),flush=True)
(d/'reward_warm_grid.json').write_text(json.dumps(rows,indent=2))

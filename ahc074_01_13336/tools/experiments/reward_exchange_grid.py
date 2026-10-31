from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import json,subprocess,re
d=Path(__file__).resolve().parent;root=d.parents[1]
s=(d/'refit4_reward.cpp').read_text().replace('int main(){','int main(int argc,char**argv){')
s=s.replace('int main(int argc,char**argv){','#include "structure2_agent_exchange.h"\nint main(int argc,char**argv){')
s=s.replace('refit_all_prefix_papers(A,papers,4,32);','''refit_all_prefix_papers(A,papers,4,32);
 exchange_prefix_dictionary(A,papers,argc>1?atoi(argv[1]):5,argc>2?atoi(argv[2]):32,1,0,8,structure2_training_origin,0.95);
 if(argc>3&&atoi(argv[3]))refit_all_prefix_papers(A,papers,1,32);''')
(d/'reward_exchange_grid.cpp').write_text(s)
subprocess.run(['g++','-std=c++20','-O2',str(d/'reward_exchange_grid.cpp'),'-o',str(d/'reward_exchange_grid')],check=True)
configs=[(5,32,0),(10,32,0),(20,32,0),(10,256,0),(20,256,0),(10,32,1),(10,256,1)]
def run(job):
    config,seed=job
    p=subprocess.run([str(root/'tools/target/release/tester'),str(d/'reward_exchange_grid'),*map(str,config)],input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),capture_output=True)
    err=p.stderr.decode();m=re.search(r'Score = (\d+)',err);assert p.returncode==0 and m,(job,err)
    score=int(m[1]);at=192;total=0;tokens=p.stdout.split()
    for _ in range(1024):count=int(tokens[at]);total+=count;at+=1+2*count
    assert total==score and at==len(tokens)
    return dict(config=config,seed=seed,score=score,cpu=float(re.search(r'FinishCPU=([\d.]+)',err)[1]),log=err)
with ThreadPoolExecutor(max_workers=3) as pool:
    rows=[]
    for r in pool.map(run,[(c,s) for c in configs for s in range(3)]):
        rows.append(r)
        if r['seed']==2:
            rr=[x for x in rows if x['config']==r['config']];print(r['config'],sum(x['score'] for x in rr)/len(rr),max(x['cpu'] for x in rr),flush=True)
(d/'reward_exchange_grid.json').write_text(json.dumps(rows,indent=2))

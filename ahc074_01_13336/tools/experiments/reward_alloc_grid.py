from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import json,subprocess,re,sys
d=Path(__file__).resolve().parent;root=d.parents[1]
s=(d/'refit4_reward.cpp').read_text().replace('int main(){','int main(int argc,char**argv){')
s=s.replace('static clock_t structure2_training_origin=0;','static clock_t structure2_training_origin=0;\nstatic int allocation_mode=6;')
s=s.replace('structure2_training_origin=clock();','allocation_mode=argc>1?atoi(argv[1]):6;structure2_training_origin=clock();')
s=s.replace('int amount=std::min(perlevel,int(extra[l].size()+spare.size()));', '''int cap=perlevel;
  if(allocation_mode>=0&&allocation_mode<=6&&l==n)cap=allocation_mode;
  if(allocation_mode==7&&l>=n-1)cap=2;
  if(allocation_mode==8&&l>=n-3)cap=2;
  if(allocation_mode==9&&l>=n-7)cap=3;
  if(allocation_mode==10)cap=l>56?4:6;
  if(allocation_mode==11)cap=l>56?6:4;
  if(allocation_mode==12)cap=l>56?5:7;
  if(allocation_mode==13)cap=l>56?7:5;
  int amount=std::min(cap,int(extra[l].size()+spare.size()));''')
(d/'reward_alloc_grid.cpp').write_text(s)
subprocess.run(['g++','-std=c++20','-O2',str(d/'reward_alloc_grid.cpp'),'-o',str(d/'reward_alloc_grid')],check=True)
configs=list(range(13))
def run(job):
    config,seed=job
    p=subprocess.run([str(root/'tools/target/release/tester'),str(d/'reward_alloc_grid'),str(config)],input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),capture_output=True)
    err=p.stderr.decode();m=re.search(r'Score = (\d+)',err);assert p.returncode==0 and m,(job,err)
    score=int(m[1]);at=192;total=0;tokens=p.stdout.split()
    for _ in range(1024):count=int(tokens[at]);total+=count;at+=1+2*count
    assert total==score and at==len(tokens)
    return dict(config=config,seed=seed,score=score,cpu=float(re.search(r'FinishCPU=([\d.]+)',err)[1]))
with ThreadPoolExecutor(max_workers=3) as pool:
    rows=[]
    for r in pool.map(run,[(c,s) for c in configs for s in range(3)]):
        rows.append(r)
        if r['seed']==2:
            rr=[x for x in rows if x['config']==r['config']];print(r['config'],sum(x['score'] for x in rr)/len(rr),max(x['cpu'] for x in rr),flush=True)
(d/'reward_alloc_grid.json').write_text(json.dumps(rows,indent=2))

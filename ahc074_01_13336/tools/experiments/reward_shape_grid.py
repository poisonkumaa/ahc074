from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import json,subprocess,re
d=Path(__file__).resolve().parent;root=d.parents[1]
s=(d/'goal_reward_fast.cpp').read_text().replace('int main(){','int main(int argc,char**argv){')
old='for(int k=0;k<int(papers.size());k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);'
new='''for(int k=0;k<int(papers.size());k++){
   int f=__builtin_popcountll(future[k]),l=papers[k].len,mode=argc>1?atoi(argv[1]):0;
   int b=argc>2?atoi(argv[2]):16;
   if(mode==0)dp.reward[k]=b+2*f;
   if(mode==1)dp.reward[k]=b+f*f/16;
   if(mode==2)dp.reward[k]=b+int(16*sqrt(f));
   if(mode==3)dp.reward[k]=b+f*l/16;
   if(mode==4)dp.reward[k]=b+f*(65-l)/16;
   if(mode==5)dp.reward[k]=b+2*f+2*l;
   if(mode==6)dp.reward[k]=b+2*f+2*(65-l);
   if(mode==7)dp.reward[k]=b+2*f+l*(65-l)/16;
  }'''
assert old in s;s=s.replace(old,new)
# Offset negative tie scores uniformly by adding 32768 per move, still primary.
(d/'reward_shape_grid.cpp').write_text(s)
subprocess.run(['g++','-std=c++20','-O2',str(d/'reward_shape_grid.cpp'),'-o',str(d/'reward_shape_grid')],check=True)
configs=[(0,0),(0,4),(0,16),(0,64),(0,256)]+[(i,16) for i in range(1,8)]
def run(job):
    config,seed=job
    p=subprocess.run([str(root/'tools/target/release/tester'),str(d/'reward_shape_grid'),*map(str,config)],input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),capture_output=True)
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
(d/'reward_shape_grid.json').write_text(json.dumps(rows,indent=2))

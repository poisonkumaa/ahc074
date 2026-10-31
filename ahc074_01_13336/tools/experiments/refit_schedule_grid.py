from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import json,subprocess,re
d=Path(__file__).resolve().parent;root=d.parents[1]
s=(d/'main_13906_final.cpp').read_text()
a=s.index('inline void refit_all_prefix_papers(');b=s.index('\nint main()',a)
p=s[a:b].replace('int passes,int importance)', 'int passes,int importance,int mode=0)')
old='for(int pass=0;pass<passes;pass++)for(int l=N;l>0;l--)'
new='''std::mt19937 schedule_rng(146829);
 std::vector<int>schedule;
 for(int pass=0;pass<passes;pass++){
  std::vector<int>v(64);std::iota(v.rbegin(),v.rend(),1);
  if(mode==1&&pass>0)std::reverse(v.begin(),v.end());
  if(mode==2)std::reverse(v.begin(),v.end());
  if(mode==3||(mode==4&&pass>0))std::shuffle(v.begin(),v.end(),schedule_rng);
  if(mode==5&&(pass&1))std::reverse(v.begin(),v.end());
  if(mode==6)std::stable_sort(v.begin(),v.end(),[](int a,int b){return (a%8)>(b%8);});
  schedule.insert(schedule.end(),v.begin(),v.end());
 }
 for(int l:schedule)'''
assert old in p;p=p.replace(old,new)
s=s[:a]+p+s[b:]
s=s.replace('int main(){','int main(int argc,char**argv){').replace('refit_all_prefix_papers(A,papers,4,32);','refit_all_prefix_papers(A,papers,argc>2?atoi(argv[2]):4,32,argc>1?atoi(argv[1]):0);')
(d/'refit_schedule_grid.cpp').write_text(s)
subprocess.run(['g++','-std=c++20','-O2',str(d/'refit_schedule_grid.cpp'),'-o',str(d/'refit_schedule_grid')],check=True)
configs=[(i,4) for i in range(7)]+[(1,8),(3,8),(4,8),(5,8)]
def run(job):
 c,seed=job
 p=subprocess.run([str(root/'tools/target/release/tester'),str(d/'refit_schedule_grid'),*map(str,c)],input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),capture_output=True,check=True)
 err=p.stderr.decode();score=int(re.search(r'Score = (\d+)',err)[1]);tokens=p.stdout.split();at=192;moves=0
 for _ in range(1024):m=int(tokens[at]);assert 0<=m<=384;moves+=m;at+=1+2*m
 assert at==len(tokens) and score==moves
 return dict(config=c,seed=seed,score=score,cpu=float(re.search(r'FinishCPU=([\d.]+)',err)[1]),log=err)
rows=[]
with ThreadPoolExecutor(max_workers=3) as pool:
 for r in pool.map(run,[(c,s) for c in configs for s in range(3)]):
  rows.append(r)
  if r['seed']==2:
   rr=[x for x in rows if x['config']==r['config']];print(r['config'],sum(x['score'] for x in rr)/len(rr),max(x['cpu'] for x in rr),flush=True)
(d/'refit_schedule_grid.json').write_text(json.dumps(rows,indent=2))

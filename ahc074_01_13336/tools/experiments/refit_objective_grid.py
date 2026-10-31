from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import json,subprocess,re,sys
d=Path(__file__).resolve().parent;root=d.parents[1]
s=(d/'main_13906_final.cpp').read_text()
a=s.index('inline void refit_all_prefix_papers(');b=s.index('\nint main()',a)
p=s[a:b].replace('int passes,int importance)', 'int passes,int importance,int mode=0)')
p=p.replace('trie[x].weight+=benefit-previous+1;previous=benefit;', '''int secondary=1,gap=F[v][l-d]+1+B[v][l]-F[v][N];
    if(mode==1)secondary=0;
    if(mode==2)secondary=(gap<=0);
    if(mode==3)secondary=gap<=0?8:1;
    if(mode==4)secondary=gap<7?64>>std::max(0,gap):0;
    if(mode==5)secondary=(F[v][l-d]<INF&&B[v][l]<INF);
    trie[x].weight+=benefit-previous+secondary;previous=benefit;''')
s=s[:a]+p+s[b:]
s=s.replace('int main(){','int main(int argc,char**argv){')
s=s.replace('refit_all_prefix_papers(A,papers,4,32);','refit_all_prefix_papers(A,papers,4,argc>1?atoi(argv[1]):32,argc>2?atoi(argv[2]):0);')
(d/'refit_objective_grid.cpp').write_text(s)
subprocess.run(['g++','-std=c++20','-O2',str(d/'refit_objective_grid.cpp'),'-o',str(d/'refit_objective_grid')],check=True)
configs=[(32,0),(128,0),(512,0),(2048,0),(32,1),(32,2),(32,3),(256,3),(2048,4),(512,4),(128,4),(32,5)]
def run(job):
 c,seed=job
 p=subprocess.run([str(root/'tools/target/release/tester'),str(d/'refit_objective_grid'),*map(str,c)],input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),capture_output=True,check=True)
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
(d/'refit_objective_grid.json').write_text(json.dumps(rows,indent=2))

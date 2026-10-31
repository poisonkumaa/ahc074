from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import json,re,subprocess
d=Path(__file__).resolve().parent;root=d.parents[1]
s=(d/'main_13906_final.cpp').read_text().replace('int main(){','int main(int argc,char**argv){')
old='cols=bestcols;for(int j=1;'
new='''cols=bestcols;
 int mode=argc>1?atoi(argv[1]):0;long long bestmetric=LLONG_MAX;
 for(int rev=0;rev<2;rev++)for(int r=0;r<64;r++){
  vector<int>candidate(64);for(int j=0;j<64;j++)candidate[j]=bestcols[(r+(rev?64-j:j))%64];
  long long metric=0;for(int j=1;j<64;j++){int v=correlation[candidate[j-1]][candidate[j]];v=min(v,P-v);bool hi=j>=43;
   int weight=mode==0?(hi?2:1):mode==1?(hi?1:2):mode==2?(hi?4:1):mode==3?(hi?1:4):mode==4?(hi?1:0):(hi?0:1);
   metric+=weight*v;
  }
  if(metric<bestmetric){bestmetric=metric;cols=candidate;}
 }
 for(int j=1;'''
assert old in s;s=s.replace(old,new)
(d/'rotation_metric_grid.cpp').write_text(s)
subprocess.run(['g++','-std=c++20','-O2',str(d/'rotation_metric_grid.cpp'),'-o',str(d/'rotation_metric_grid')],check=True)
configs=list(range(6))
def run(job):
 c,seed=job
 p=subprocess.run([str(root/'tools/target/release/tester'),str(d/'rotation_metric_grid'),str(c)],input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),capture_output=True,check=True)
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
(d/'rotation_metric_grid.json').write_text(json.dumps(rows,indent=2))

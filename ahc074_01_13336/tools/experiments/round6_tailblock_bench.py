import argparse,concurrent.futures,json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser();p.add_argument('--count',type=int,default=3);p.add_argument('--blocks',nargs='+',type=int,default=[1,2,4,8]);p.add_argument('--binary',default='round6_tailblock');p.add_argument('--workers',type=int,default=1);p.add_argument('--extra',nargs='*',default=[]);a=p.parse_args()
def run(case):
 b,seed=case
 result=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments'/a.binary),str(b),'8',*a.extra],input=(root/f'tools/in/{seed:04}.txt').read_bytes(),capture_output=True,cwd=root)
 msg=result.stderr.decode();score=re.search(r'Score = (\d+)',msg)
 assert result.returncode==0 and score,(case,msg)
 fields=result.stdout.split();at=192;total=0;mx=0
 for r in range(1024):
  m=int(fields[at]);assert 0<=m<=384;total+=m;mx=max(mx,m);at+=1+2*m
 assert at==len(fields) and total==int(score[1]),(case,total,score[1])
 row=dict(block=b,seed=seed,score=total,cpu=float(re.search(r'FinishCPU=([\d.]+)',msg)[1]),max_moves=mx,diagnostics=msg)
 print(b,seed,total,flush=True);return row
with concurrent.futures.ThreadPoolExecutor(max_workers=a.workers) as pool:rows=list(pool.map(run,[(b,s)for b in a.blocks for s in range(a.count)]))
for b in a.blocks:
 r=[x for x in rows if x['block']==b];print('MEAN',b,sum(x['score']for x in r)/len(r),'CPU',max(x['cpu']for x in r),flush=True)
(root/f'tools/experiments/{a.binary}_{a.count}_results.json').write_text(json.dumps(rows,indent=2))

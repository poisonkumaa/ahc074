import argparse,concurrent.futures,json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser();p.add_argument('--passes',nargs='+',type=int,default=[0,1,3]);p.add_argument('--pairs',type=int,default=1000);p.add_argument('--snapshots',type=int,default=256);p.add_argument('--count',type=int,default=3);p.add_argument('--reps',type=int,default=32);p.add_argument('--workers',type=int,default=1);p.add_argument('--binary',default='round7_source_swap');p.add_argument('--validation',type=int,default=0);a=p.parse_args()
def run(case):
 passes,seed=case
 result=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments'/a.binary),str(passes),str(a.pairs),str(a.snapshots),str(a.reps),str(a.validation)],input=(root/f'tools/in/{seed:04}.txt').read_bytes(),capture_output=True,cwd=root)
 msg=result.stderr.decode();score=re.search(r'Score = (\d+)',msg);assert result.returncode==0 and score,(case,msg)
 fields=result.stdout.split();at=192;total=0;mx=0
 for _ in range(1024):m=int(fields[at]);assert 0<=m<=384;total+=m;mx=max(mx,m);at+=1+2*m
 assert at==len(fields)and total==int(score[1]),(case,total,score[1])
 row=dict(passes=passes,seed=seed,score=total,cpu=float(re.search(r'FinishCPU=([\d.]+)',msg)[1]),max_moves=mx,diagnostics=msg);print(passes,seed,total,flush=True);return row
with concurrent.futures.ThreadPoolExecutor(max_workers=a.workers) as pool:rows=list(pool.map(run,[(p,s)for p in a.passes for s in range(a.count)]))
for passes in a.passes:
 r=[x for x in rows if x['passes']==passes];print('MEAN',passes,sum(x['score']for x in r)/len(r),'CPU',max(x['cpu']for x in r),flush=True)
suffix=f'_v{a.validation}'if a.validation else''
(root/f'tools/experiments/round7_source_swap_c{a.count}_p{a.pairs}_s{a.snapshots}_r{a.reps}{suffix}_results.json').write_text(json.dumps(rows,indent=2))

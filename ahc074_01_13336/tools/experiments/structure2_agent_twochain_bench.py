import subprocess,pathlib,re,concurrent.futures,json
root=pathlib.Path(__file__).resolve().parents[2]
def run(case):
 amount,seed=case
 p=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/structure2_agent_twochain'),str(amount)],input=(root/f'tools/in/{seed:04}.txt').read_bytes(),capture_output=True,cwd=root)
 msg=p.stderr.decode();score=re.search(r'Score = (\d+)',msg);cpu=re.search(r'FinishCPU=([\d.]+)',msg)
 assert p.returncode==0 and score and cpu,(case,msg)
 data=p.stdout.split();at=192;total=0;mx=0
 for _ in range(1024):
  m=int(data[at]);assert 0<=m<=384;total+=m;mx=max(mx,m);at+=1+2*m
 assert at==len(data) and total==int(score[1]),(case,total,score[1])
 return dict(amount=amount,seed=seed,score=int(score[1]),cpu=float(cpu[1]),max_moves=mx,diagnostics=msg)
with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
 rows=list(pool.map(run,[(amount,seed) for amount in [4,6] for seed in range(3)]))
for amount in [4,6]:
 r=[x for x in rows if x['amount']==amount]
 print(amount,'mean',sum(x['score'] for x in r)/len(r),'maxcpu',max(x['cpu'] for x in r),'scores',[x['score'] for x in r],flush=True)
(root/'tools/experiments/structure2_agent_twochain_results.json').write_text(json.dumps(rows,indent=2))

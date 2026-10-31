import subprocess,pathlib,re,concurrent.futures,json,sys
root=pathlib.Path(__file__).resolve().parents[2]
count=int(sys.argv[1]) if len(sys.argv)>1 else 3
alphas=[int(s) for s in sys.argv[2:]] or [0,1,2,3,4]
def run(case):
 alpha,seed=case
 p=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/structure2_agent_pair'),str(alpha)],input=(root/f'tools/in/{seed:04}.txt').read_bytes(),capture_output=True,cwd=root)
 msg=p.stderr.decode();score=re.search(r'Score = (\d+)',msg);cpu=re.search(r'FinishCPU=([\d.]+)',msg)
 assert p.returncode==0 and score and cpu,(case,msg)
 data=p.stdout.split();at=192;total=0;mx=0
 for _ in range(1024):
  m=int(data[at]);assert 0<=m<=384;total+=m;mx=max(mx,m);at+=1+2*m
 assert at==len(data) and total==int(score[1]),(case,total,score[1])
 return dict(alpha=alpha,seed=seed,score=int(score[1]),cpu=float(cpu[1]),max_moves=mx)
cases=[(alpha,seed) for alpha in alphas for seed in range(count)]
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
 rows=list(pool.map(run,cases))
for alpha in alphas:
 r=[x for x in rows if x['alpha']==alpha]
 print(alpha,'mean',sum(x['score'] for x in r)/len(r),'maxcpu',max(x['cpu'] for x in r),'scores',[x['score'] for x in r],flush=True)
(root/f'tools/experiments/structure2_agent_pair_{count}.json').write_text(json.dumps(rows,indent=2))

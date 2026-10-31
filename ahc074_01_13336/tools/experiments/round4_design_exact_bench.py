import subprocess,pathlib,re,concurrent.futures,json,sys
root=pathlib.Path(__file__).resolve().parents[2]
count=int(sys.argv[1]) if len(sys.argv)>1 else 10
orientations=list(map(int,sys.argv[2:])) or [32,12345]
def run(case):
 orientation,seed=case
 p=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/round4_design_exact'),'1','43',str(orientation),'4'],input=(root/f'tools/in/{seed:04}.txt').read_bytes(),capture_output=True,cwd=root)
 msg=p.stderr.decode();score=re.search(r'Score = (\d+)',msg);cpu=re.search(r'FinishCPU=([\d.]+)',msg)
 assert p.returncode==0 and score and cpu,(case,msg)
 data=p.stdout.split();at=192;total=0;mx=0
 for _ in range(1024):m=int(data[at]);assert 0<=m<=384;total+=m;mx=max(mx,m);at+=1+2*m
 assert at==len(data) and total==int(score[1]),(case,total,score[1])
 return dict(orientation=orientation,seed=seed,score=int(score[1]),cpu=float(cpu[1]),max_moves=mx,diagnostics=msg)
with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
 rows=list(pool.map(run,[(orientation,seed) for orientation in orientations for seed in range(count)]))
for orientation in orientations:
 r=[x for x in rows if x['orientation']==orientation]
 print(orientation,'mean',sum(x['score'] for x in r)/len(r),'maxcpu',max(x['cpu'] for x in r),'scores',[x['score'] for x in r],flush=True)
(root/f'tools/experiments/round4_design_exact_{count}.json').write_text(json.dumps(rows,indent=2))

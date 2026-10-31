import subprocess,pathlib,re,concurrent.futures,json,sys
root=pathlib.Path(__file__).resolve().parents[2]
arguments=sys.argv[1:]
orientation=32
if '--orientation' in arguments:
 index=arguments.index('--orientation');orientation=int(arguments[index+1]);del arguments[index:index+2]
count=int(arguments[0]) if arguments else 3
modes=list(map(int,arguments[1:])) or [0,1,2,3,4,5,10,11,13,14]
def run(case):
 mode,seed=case
 p=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/round4_design_profile'),str(mode),'0','0','0',str(orientation)],input=(root/f'tools/in/{seed:04}.txt').read_bytes(),capture_output=True,cwd=root)
 msg=p.stderr.decode();score=re.search(r'Score = (\d+)',msg);cpu=re.search(r'FinishCPU=([\d.]+)',msg)
 assert p.returncode==0 and score and cpu,(case,msg)
 data=p.stdout.split();at=192;total=0;mx=0
 for _ in range(1024):m=int(data[at]);assert 0<=m<=384;total+=m;mx=max(mx,m);at+=1+2*m
 assert at==len(data) and total==int(score[1]),(case,total,score[1])
 return dict(mode=mode,seed=seed,score=int(score[1]),cpu=float(cpu[1]),max_moves=mx,diagnostics=msg)
with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
 rows=list(pool.map(run,[(mode,seed) for mode in modes for seed in range(count)]))
for mode in modes:
 r=[x for x in rows if x['mode']==mode]
 print(mode,'mean',sum(x['score'] for x in r)/len(r),'maxcpu',max(x['cpu'] for x in r),'scores',[x['score'] for x in r],flush=True)
(root/f'tools/experiments/round4_design_profile_{count}_ori{orientation}.json').write_text(json.dumps(rows,indent=2))

import pathlib,subprocess,re,sys,json,time,concurrent.futures
root=pathlib.Path(__file__).resolve().parents[2]
passes=int(sys.argv[1]) if len(sys.argv)>1 else 1
weight=int(sys.argv[2]) if len(sys.argv)>2 else 8
standalone=len(sys.argv)>3 and sys.argv[3]=='main'
def run(case):
 kind,seed=case
 path=root/(f'tools/in/{seed:04}.txt' if kind=='public' else f'tools/experiments/holdout/in/{seed:04}.txt')
 started=time.monotonic()
 command=[str(root/'tools/target/release/tester'),str(root/('tools/experiments/structure2_agent_main' if standalone else 'tools/experiments/structure2_agent_backbone'))]
 if not standalone:command+=['1','8','0','6','0','64',str(passes),str(weight)]
 p=subprocess.run(command,input=path.read_bytes(),capture_output=True,cwd=root)
 msg=p.stderr.decode();m=re.search(r'Score = (\d+)',msg);cpu=re.search(r'FinishCPU=([\d.]+)',msg)
 assert p.returncode==0 and m and cpu,(case,msg)
 tokens=p.stdout.split();at=192;total=0;mx=0
 for r in range(1024):
  count=int(tokens[at]);assert 0<=count<=384;at+=1;mx=max(mx,count);total+=count
  for j in range(count):assert 0<=int(tokens[at+2*j])<192 and 0<=int(tokens[at+2*j+1])<192
  at+=count*2
 score=int(m[1]);assert at==len(tokens) and score==total,(case,score,total)
 return dict(kind=kind,seed=seed if kind=='public' else seed+1000,score=score,moves=total,errors=0,max_moves=mx,cpu=float(cpu[1]),elapsed=time.monotonic()-started)
cases=[('public',s) for s in range(100)]+[('holdout',s) for s in range(50)]
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
 results=[]
 for r in pool.map(run,cases):
  results.append(r)
  if len(results)%25==0:print('done',len(results),flush=True)
summary={}
for kind in ['public','holdout']:
 rows=[r for r in results if r['kind']==kind]
 summary[kind]=dict(count=len(rows),mean=sum(r['score'] for r in rows)/len(rows),minimum=min(r['score'] for r in rows),maximum=max(r['score'] for r in rows),max_cpu=max(r['cpu'] for r in rows),max_moves=max(r['max_moves'] for r in rows),errors=0)
out=dict(passes=passes,weight=weight,summary=summary,cases=results)
(root/f'tools/experiments/structure2_agent_validate_{passes}_{weight}{"_main" if standalone else ""}.json').write_text(json.dumps(out,indent=2))
print(json.dumps(summary,indent=2),flush=True)

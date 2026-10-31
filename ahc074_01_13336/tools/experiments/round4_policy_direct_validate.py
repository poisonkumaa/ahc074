from pathlib import Path
import subprocess,re,json,concurrent.futures
root=Path(__file__).resolve().parents[2]
old=json.loads((root/'tools/experiments/policy2_agent_perf_equivalence100.json').read_text());old_score={x['seed']:x['fast']['score'] for x in old}
new=json.loads((root/'tools/experiments/round4_exact_iter_100_1_43_12345_4_0_2_results.json').read_text());new_score={x['seed']:x['score'] for x in new['cases']}
def run(job):
 name,args,expected,seed=job
 r=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments'/name),*args],input=(root/f'tools/in/{seed:04}.txt').read_text(),capture_output=True,text=True,timeout=90)
 assert r.returncode==0,(name,seed,r.stderr)
 score=int(re.search(r'Score = (\d+)',r.stderr)[1]);failures=int(re.search(r'DirectFailures=(\d+)',r.stderr)[1]);cpu=float(re.search(r'FinishCPU=([\d.]+)',r.stderr)[1]);path=float(re.search(r'PathCPU=([\d.]+)',r.stderr)[1]);lines=r.stdout.splitlines();at=192;ops=0;maximum=0
 for _ in range(1024):m=int(lines[at]);assert 0<=m<=384;ops+=m;maximum=max(maximum,m);at+=1+m
 assert ops==score==expected[seed] and at==len(lines) and failures==0,(name,seed,score,expected[seed],r.stderr)
 if seed%20==0:print(name,seed,score,cpu,path,flush=True)
 return dict(seed=seed,score=score,errors=0,direct_failures=failures,cpu=cpu,path_cpu=path,max_moves=maximum)
out={}
for name,args,expected in [('round4_policy_direct_moves',[],old_score),('round4_policy_direct_exact',['1','43','12345','4','0','2'],new_score)]:
 with concurrent.futures.ThreadPoolExecutor(3) as pool:cases=list(pool.map(run,[(name,args,expected,s) for s in range(100)]))
 summary=dict(mean=sum(x['score'] for x in cases)/100,max_cpu=max(x['cpu'] for x in cases),mean_path_cpu=sum(x['path_cpu'] for x in cases)/100,max_path_cpu=max(x['path_cpu'] for x in cases),max_moves=max(x['max_moves'] for x in cases),errors=0,direct_failures=0)
 print(name,summary,flush=True);out[name]=dict(summary=summary,cases=cases)
 (root/'tools/experiments/round4_policy_direct_validation.json').write_text(json.dumps(out,indent=2))

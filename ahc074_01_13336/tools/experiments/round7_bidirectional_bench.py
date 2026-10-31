import argparse,concurrent.futures,json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser();p.add_argument('--rights',nargs='+',type=int,default=[0,32,64,96]);p.add_argument('--passes',type=int,default=2);p.add_argument('--count',type=int,default=1);p.add_argument('--workers',type=int,default=1);p.add_argument('--binary',default='round7_bidirectional');a=p.parse_args()
def run(case):
 count,seed=case
 r=subprocess.run([str(root/'tools/experiments'/a.binary),str(count),str(a.passes),'256'],input=(root/f'tools/in/{seed:04}.txt').read_bytes(),capture_output=True,cwd=root)
 msg=r.stderr.decode();assert r.returncode==0,(case,msg)
 result=dict(right=count,seed=seed,intrinsic=float(r.stdout.split()[0]),diagnostics=msg)
 (root/f'tools/experiments/round7_bidirectional_r{count}_s{seed}_p{a.passes}.txt').write_bytes(r.stdout)
 print(count,seed,result['intrinsic'],flush=True);return result
with concurrent.futures.ThreadPoolExecutor(max_workers=a.workers) as pool:rows=list(pool.map(run,[(r,s)for r in a.rights for s in range(a.count)]))
(root/f'tools/experiments/round7_bidirectional_p{a.passes}_results.json').write_text(json.dumps(rows,indent=2))

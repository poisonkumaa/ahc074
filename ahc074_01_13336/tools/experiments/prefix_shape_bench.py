import concurrent.futures,json,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
cap=int(sys.argv[1]) if len(sys.argv)>1 else 16
configs=[(5,0),(6,0),(7,0),(8,0),(10,0),(12,0),(8,2),(8,4),(8,8)]
def run(job):
    config,seed=job;amount,shape=config
    p=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/prefix_allocate_submit'),'1','8','0',str(amount),str(shape),str(cap)],input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    err=p.stderr.decode();m=re.search(r'Score = (\d+)',err);score=int(m[1]) if m else None
    at=192;tokens=p.stdout.split();moves=0
    for _ in range(1024): count=int(tokens[at]);moves+=count;at+=1+2*count
    assert moves==score and at==len(tokens),(job,err)
    return {'amount':amount,'shape':shape,'seed':seed,'score':score,'stderr':err}
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool: rows=list(pool.map(run,[(c,s) for c in configs for s in range(5)]))
for amount,shape in configs:
    rr=[r['score'] for r in rows if r['amount']==amount and r['shape']==shape]
    print(amount,shape,sum(rr)/len(rr),rr,flush=True)
(root/f'tools/experiments/prefix_shape_bench_{cap}.json').write_text(json.dumps(rows,indent=2))

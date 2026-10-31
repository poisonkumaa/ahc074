import concurrent.futures,json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[2]
configs=[0,40,80,120,160]
def run(job):
    price,seed=job
    p=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/prefix_allocate_submit'),'1','8',str(price)],input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    err=p.stderr.decode();m=re.search(r'Score = (\d+)',err);score=int(m[1]) if m else None
    at=192;tokens=p.stdout.split();moves=0
    for _ in range(1024): count=int(tokens[at]);moves+=count;at+=1+2*count
    assert moves==score and at==len(tokens),(job,err)
    return {'price':price,'seed':seed,'score':score,'stderr':err}
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool: rows=list(pool.map(run,[(c,s) for c in configs for s in range(10)]))
for c in configs:
    rr=[r['score'] for r in rows if r['price']==c]
    print(c,sum(rr)/len(rr),rr,flush=True)
(root/'tools/experiments/prefix_price_seeds.json').write_text(json.dumps(rows,indent=2))

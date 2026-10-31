import concurrent.futures,json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[2]
configs=[8,12,16,24,32,64]
def run(job):
    cap,seed=job
    p=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/prefix_allocate_submit'),'1','8','0','6','0',str(cap)],input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    err=p.stderr.decode();m=re.search(r'Score = (\d+)',err);score=int(m[1]) if m else None
    at=192;tokens=p.stdout.split();moves=0
    for _ in range(1024): count=int(tokens[at]);moves+=count;at+=1+2*count
    assert moves==score and at==len(tokens),(job,err)
    return {'cap':cap,'seed':seed,'score':score,'stderr':err}
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool: rows=list(pool.map(run,[(c,s) for c in configs for s in range(3)]))
for c in configs:
    rr=[r['score'] for r in rows if r['cap']==c]
    print(c,sum(rr)/len(rr),rr,flush=True)
(root/'tools/experiments/prefix_depth_bench.json').write_text(json.dumps(rows,indent=2))

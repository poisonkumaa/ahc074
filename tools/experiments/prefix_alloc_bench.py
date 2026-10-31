exec(open(__file__.replace('prefix_alloc_bench.py','prefix_bench.py')).read().split('configs=')[0])
configs=[(3,2,12),(4,2,8),(4,2,12),(4,2,16),(3,3,12),(4,3,12)]
seeds=range(5)
def run(t):
    (mode,assignment,cap),seed=t
    with open(root/f'tools/in/{seed:04d}.txt') as inp:
        p=subprocess.run([str(exe),'1024',str(mode),'3','0',str(assignment),str(cap),'.5'],stdin=inp,stdout=subprocess.DEVNULL,stderr=subprocess.PIPE,text=True)
    m=re.search(r'TOTAL (\d+) .* CPU ([\d.]+)',p.stderr)
    return dict(mode=mode,assignment=assignment,cap=cap,seed=seed,score=int(m[1]) if m else None,cpu=float(m[2]) if m else None,error=p.stderr if not m else None)
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool: results=list(pool.map(run,[(c,s) for c in configs for s in seeds]))
for c in configs:
    rr=[r for r in results if (r['mode'],r['assignment'],r['cap'])==c]
    print(c,'mean',sum(r['score'] for r in rr)/len(rr),'cpu',max(r['cpu'] for r in rr),'scores',[r['score'] for r in rr],flush=True)
(root/'tools/experiments/prefix_alloc_bench.json').write_text(json.dumps(results,indent=2))

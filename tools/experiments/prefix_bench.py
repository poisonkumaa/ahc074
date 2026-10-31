import concurrent.futures,json,pathlib,re,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2]
exe=root/'tools/experiments/prefix_run'
configs=[(64,.3),(64,.5),(64,.7),(64,.85),(16,.5),(8,.5),(5,.5),(8,.75)]
seeds=range(3)
def run(t):
    (cap,decay),seed=t
    with open(root/f'tools/in/{seed:04d}.txt') as inp:
        p=subprocess.run([str(exe),'1024','2','3','0','1',str(cap),str(decay)],stdin=inp,stdout=subprocess.DEVNULL,stderr=subprocess.PIPE,text=True)
    m=re.search(r'TOTAL (\d+) .* CPU ([\d.]+)',p.stderr)
    return dict(cap=cap,decay=decay,seed=seed,score=int(m[1]) if m else None,cpu=float(m[2]) if m else None,error=p.stderr if not m else None)
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool: results=list(pool.map(run,[(c,s) for c in configs for s in seeds]))
for c in configs:
    rr=[r for r in results if (r['cap'],r['decay'])==c]
    print(c,'mean',sum(r['score'] for r in rr)/len(rr),'cpu',max(r['cpu'] for r in rr),'scores',[r['score'] for r in rr],flush=True)
(root/'tools/experiments/prefix_bench.json').write_text(json.dumps(results,indent=2))

exec(open(__file__.replace('prefix_trie_bench.py','prefix_bench.py')).read().split('configs=')[0])
configs=[(2,3,16),(3,3,64),(3,3,32),(3,3,16),(3,3,12),(3,3,8),(3,2,16)]
seeds=range(5)
def run(t):
    (mode,mult,cap),seed=t
    with open(root/f'tools/in/{seed:04d}.txt') as inp:
        p=subprocess.run([str(exe),'1024',str(mode),str(mult),'0','2',str(cap),'.5'],stdin=inp,stdout=subprocess.DEVNULL,stderr=subprocess.PIPE,text=True)
    m=re.search(r'TOTAL (\d+) .* CPU ([\d.]+)',p.stderr)
    return dict(mode=mode,mult=mult,cap=cap,seed=seed,score=int(m[1]) if m else None,cpu=float(m[2]) if m else None,error=p.stderr if not m else None)
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool: results=list(pool.map(run,[(c,s) for c in configs for s in seeds]))
for c in configs:
    rr=[r for r in results if (r['mode'],r['mult'],r['cap'])==c]
    print(c,'mean',sum(r['score'] for r in rr)/len(rr),'cpu',max(r['cpu'] for r in rr),'scores',[r['score'] for r in rr],flush=True)
(root/'tools/experiments/prefix_trie_bench.json').write_text(json.dumps(results,indent=2))

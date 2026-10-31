import concurrent.futures,json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[2]
configs=[(0,0,0)]+[(1,1,j) for j in [16,32,40,48,52,56,60,62]]+[(2,4,52),(2,4,56),(4,4,48),(8,2,48)]
def run(job):
    cfg,seed=job;ports,spacing,start=cfg
    args=['1024','3','3','0','2','16','.5','0','32','0','64',str(ports),str(spacing),str(start)]
    p=subprocess.run([str(root/'tools/experiments/prefix_ports_run')]+args,input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    err=p.stderr.decode();m=re.search(r'TOTAL (\d+) .* CPU ([\d.]+)',err)
    assert p.returncode==0 and m,(job,err)
    return {'ports':ports,'spacing':spacing,'start':start,'seed':seed,'score':int(m[1]),'cpu':float(m[2])}
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool: rows=list(pool.map(run,[(c,s) for c in configs for s in range(3)]))
for cfg in configs:
    rr=[r for r in rows if (r['ports'],r['spacing'],r['start'])==cfg]
    print(cfg,sum(r['score'] for r in rr)/len(rr),'cpu',max(r['cpu'] for r in rr),[r['score'] for r in rr],flush=True)
(root/'tools/experiments/prefix_ports_bench.json').write_text(json.dumps(rows,indent=2))

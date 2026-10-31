import concurrent.futures,json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[2]
configs=[(p,c) for p in [1,2] for c in [100,200,300,400,600]]
def run(config):
    passes,price=config
    p=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments/prefix_allocate_submit'),str(passes),'8',str(price)],input=(root/'tools/in/0000.txt').read_bytes(),stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    err=p.stderr.decode();m=re.search(r'Score = (\d+)',err)
    return {'passes':passes,'price':price,'score':int(m[1]) if m else None,'stderr':err}
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool: rows=list(pool.map(run,configs))
for row in rows:print(row,flush=True)
(root/'tools/experiments/prefix_price_bench.json').write_text(json.dumps(rows,indent=2))

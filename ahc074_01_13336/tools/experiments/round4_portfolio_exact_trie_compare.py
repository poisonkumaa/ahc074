from pathlib import Path
import subprocess,re,json
root=Path(__file__).resolve().parents[2]
rows=[]
for seed in range(10):
    data=(root/f'tools/in/{seed:04d}.txt').read_bytes();out={}
    names=['round4_portfolio_exact_trie_base','round4_exact_trie']
    if seed%2:names.reverse()
    for name in names:
        r=subprocess.run([str(root/'tools/experiments'/name),'1','43','12345','4','0','2'],input=data,stdout=subprocess.PIPE,stderr=subprocess.PIPE,check=True)
        err=r.stderr.decode();out[name]=(r.stdout,float(re.findall(r'FinishCPU=([\d.]+)',err)[-1]))
    x,y=out['round4_portfolio_exact_trie_base'],out['round4_exact_trie'];assert x[0]==y[0],seed
    rows.append(dict(seed=seed,before=x[1],after=y[1]))
summary={key:sum(r[key] for r in rows)/len(rows) for key in ('before','after')}
(root/'tools/experiments/round4_portfolio_exact_trie_compare_results.json').write_text(json.dumps(dict(summary=summary,cases=rows),indent=2));print(json.dumps(summary),flush=True)

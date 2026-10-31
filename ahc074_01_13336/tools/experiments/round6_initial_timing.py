from pathlib import Path
import json,re,subprocess
p=Path(__file__).parent;root=p.parents[1];rows=[]
for seed in range(5):
    data=(root/f'tools/in/{seed:04d}.txt').read_bytes();runs=[];outputs=[]
    for name in ['round6_initial_reference','round6_initial_fast_diag']:
        r=subprocess.run([str(p/name),'8'],input=data,stdout=subprocess.PIPE,stderr=subprocess.PIPE,check=True,timeout=120)
        log=r.stderr.decode();outputs.append(r.stdout);runs.append(dict(name=name,initial=float(re.findall(r'InitialCPU=([\d.]+)',log)[-1]),cpu=float(re.findall(r'FinishCPU=([\d.]+)',log)[-1])))
    assert outputs[0]==outputs[1]
    rows.append(dict(seed=seed,runs=runs));print(json.dumps(rows[-1]),flush=True)
summary=dict(reference_initial=sum(x['runs'][0]['initial'] for x in rows)/len(rows),fast_initial=sum(x['runs'][1]['initial'] for x in rows)/len(rows),reference_cpu=sum(x['runs'][0]['cpu'] for x in rows)/len(rows),fast_cpu=sum(x['runs'][1]['cpu'] for x in rows)/len(rows))
(p/'round6_initial_timing.json').write_text(json.dumps(dict(summary=summary,cases=rows),indent=2));print(json.dumps(summary),flush=True)

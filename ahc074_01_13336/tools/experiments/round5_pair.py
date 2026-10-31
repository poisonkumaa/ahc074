from pathlib import Path
import subprocess,json,re,sys
d=Path(__file__).resolve().parent;root=d.parents[1]
names=sys.argv[1:3];args=sys.argv[3:];rows=[]
for i,seed in enumerate([0,1,2,50,99]):
 result={}
 for name in names[::1 if i%2==0 else -1]:
  p=subprocess.run([str(root/'tools/target/release/tester'),str(d/name),*args],input=(root/f'tools/in/{seed:04d}.txt').read_bytes(),capture_output=True,check=True,timeout=120)
  err=p.stderr.decode();result[name]=(p.stdout,float(re.search(r'FinishCPU=([\d.]+)',err)[1]),float(re.search(r'exact_refit.*CPU=([\d.]+)',err)[1]))
 assert result[names[0]][0]==result[names[1]][0],seed
 rows.append(dict(seed=seed,cpu={name:result[name][1] for name in names},refit_cpu={name:result[name][2] for name in names}))
summary={name:dict(cpu=sum(r['cpu'][name] for r in rows)/len(rows),refit_cpu=sum(r['refit_cpu'][name] for r in rows)/len(rows)) for name in names}
(d/('round5_pair_'+names[1]+'.json')).write_text(json.dumps(dict(summary=summary,cases=rows),indent=2));print(json.dumps(summary),flush=True)

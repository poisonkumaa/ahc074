from pathlib import Path
import subprocess,re,json
d=Path(__file__).resolve().parent;r=d.parents[1];rows=[]
for reps in (8,32):
 for seed in range(5):
  data=(r/f'tools/in/{seed:04d}.txt').read_bytes();outputs=[];times=[]
  for name,args in [('round7_record_price',['0',str(reps)]),('round7_fast_maskdp',[str(reps)]),('round7_fast_maskpop',[str(reps)])]:
   p=subprocess.run([r/'tools/target/release/tester',d/name,*args],input=data,capture_output=True,check=True,timeout=120);outputs.append(p.stdout)
   log=p.stderr.decode();times.append(dict(cpu=float(re.search(r'FinishCPU=([\d.]+)',log)[1]),prep=float(re.search(r'PrepCPU=([\d.]+)',log)[1])))
  assert outputs[0]==outputs[1]==outputs[2]
  row=dict(reps=reps,seed=seed,baseline=times[0],mask=times[1],pop=times[2]);rows.append(row);print(row,flush=True)
(d/'round7_fast_compare.json').write_text(json.dumps(rows,indent=2))

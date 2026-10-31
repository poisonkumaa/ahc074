from pathlib import Path
import subprocess,re,json
d=Path(__file__).resolve().parent;r=d.parents[1];rows=[]
for reps in (8,32):
 for seed in range(3):
  data=(r/f'tools/in/{seed:04d}.txt').read_bytes();outputs=[];times=[]
  for cmd in ([d/'round7_record_price','0',str(reps)],[d/'round7_maskdp',str(reps)]):
   p=subprocess.run([r/'tools/target/release/tester',*cmd],input=data,capture_output=True,check=True,timeout=120);outputs.append(p.stdout);times.append(float(re.search(rb'FinishCPU=([\d.]+)',p.stderr)[1]))
  assert outputs[0]==outputs[1]
  row=dict(reps=reps,seed=seed,baseline=times[0],mask=times[1]);rows.append(row);print(row,flush=True)
(d/'round7_maskdp_compare.json').write_text(json.dumps(rows,indent=2))

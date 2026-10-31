from pathlib import Path
import subprocess,json,re,hashlib,sys
from check_multi_cutoff import validate_transcript
root=Path(__file__).resolve().parents[2];d=root/'tools/experiments'
ubsan='--ubsan' in sys.argv
budgets=[(20,30)] if ubsan else [(0,0),(.1,.15),(.5,.6),(.5,.9),(.5,1.2)]
seeds=[0] if ubsan else [0,50,99]
source=(root/'main.cpp').read_bytes();snapshot=d/'round4_cutoff_source.cpp';snapshot.write_bytes(source);rows=[]
for learn,refine in budgets:
 binary=d/('round4_cutoff_'+str(learn)+'_'+str(refine)+('_ubsan' if ubsan else ''))
 flags=['-O1','-fsanitize=undefined','-fno-sanitize-recover=all'] if ubsan else ['-O2']
 subprocess.run(['g++','-std=c++17',*flags,f'-DAHC_LEARNING_BUDGET={learn}',f'-DAHC_REFINEMENT_BUDGET={refine}',str(snapshot),'-o',str(binary)],check=True)
 for seed in seeds:
  data=(root/f'tools/in/{seed:04d}.txt').read_bytes();p=subprocess.run([str(root/'tools/target/release/tester'),str(binary)],input=data,capture_output=True,check=True,timeout=300)
  err=p.stderr.decode();score=int(re.search(r'Score = (\d+)',err)[1]);_,maximum=validate_transcript(data,p.stdout,score)
  row=dict(learning=learn,refinement=refine,seed=seed,score=score,errors=0,max_moves=maximum,cpu=float(re.search(r'FinishCPU=([\d.]+)',err)[1]),diagnostics=err);rows.append(row)
  print({k:v for k,v in row.items() if k!='diagnostics'},flush=True)
result=dict(source_sha256=hashlib.sha256(source).hexdigest(),ubsan=ubsan,cases=rows)
(d/('round4_ubsan_results.json' if ubsan else 'round4_cutoff_results.json')).write_text(json.dumps(result,indent=2))

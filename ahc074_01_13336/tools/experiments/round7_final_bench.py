from pathlib import Path
import concurrent.futures,hashlib,json,re,subprocess,sys
root=Path(__file__).resolve().parents[2]
count=int(sys.argv[1]) if len(sys.argv)>1 else 10
def run(seed):
 data=(root/f'tools/in/{seed:04}.txt').read_bytes();cases=[];previous=None
 for variant in ['reference','fast']:
  command=[str(root/'tools/target/release/tester'),str(root/f'tools/experiments/round7_final_{variant}')]
  process=subprocess.run(command,input=data,capture_output=True)
  err=process.stderr.decode();assert process.returncode==0,(seed,variant,err)
  score=int(re.search(r'Score = (\d+)',err)[1]);lines=process.stdout.splitlines();at=192;total=0
  for _ in range(1024):
   moves=int(lines[at]);assert 0<=moves<=384;total+=moves;at+=moves+1
  assert total==score and at==len(lines),(seed,variant,err)
  if previous is not None:assert process.stdout==previous,(seed,'stdout mismatch')
  previous=process.stdout
  cases.append(dict(variant=variant,score=score,prep=float(re.search(r'PrepCPU=([\d.]+)',err)[1]),cpu=float(re.search(r'FinishCPU=([\d.]+)',err)[1]),diagnostics=err))
 row=dict(seed=seed,sha256=hashlib.sha256(previous).hexdigest(),cases=cases)
 print(seed,score,*[round(c['cpu'],5) for c in cases],flush=True);return row
with concurrent.futures.ThreadPoolExecutor(2) as pool:rows=list(pool.map(run,range(count)))
for i,name in enumerate(['reference','fast']):
 cases=[r['cases'][i] for r in rows]
 print(name,'mean score',sum(c['score'] for c in cases)/count,'mean cpu',sum(c['cpu'] for c in cases)/count,'mean online',sum(c['cpu']-c['prep'] for c in cases)/count,flush=True)
(root/'tools/experiments/round7_final_results.json').write_text(json.dumps(rows,indent=2))

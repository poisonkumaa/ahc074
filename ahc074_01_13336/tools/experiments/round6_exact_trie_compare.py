import argparse,hashlib,json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser();p.add_argument('--count',type=int,default=5);p.add_argument('--reps',nargs='+',type=int,default=[8,32]);p.add_argument('--check',action='store_true');a=p.parse_args()
rows=[]
for repetitions in a.reps:
 for seed in range(a.count):
  results=[]
  for kind in ['reference','fast']:
   binary='round6_exact_trie_reference'if kind=='reference'else'round6_exact_triefast'+('_check'if a.check else'')
   run=subprocess.run([str(root/'tools/target/release/tester'),str(root/'tools/experiments'/binary),str(repetitions)],input=(root/f'tools/in/{seed:04}.txt').read_bytes(),capture_output=True,cwd=root)
   err=run.stderr.decode();score=re.search(r'Score = (\d+)',err);assert run.returncode==0 and score,(repetitions,seed,kind,err)
   data=run.stdout.split();at=192;total=0
   for _ in range(1024):m=int(data[at]);assert 0<=m<=384;total+=m;at+=1+2*m
   assert at==len(data)and total==int(score[1])
   results.append(dict(kind=kind,score=total,sha256=hashlib.sha256(run.stdout).hexdigest(),refit_cpu=float(re.search(r'exact_refit[^\n]* CPU=([\d.]+)',err)[1]),cpu=float(re.search(r'FinishCPU=([\d.]+)',err)[1]),diagnostics=err,stdout=run.stdout))
  assert results[0]['stdout']==results[1]['stdout'],(repetitions,seed,results[0]['sha256'],results[1]['sha256'])
  for result in results:del result['stdout']
  rows.append(dict(repetitions=repetitions,seed=seed,results=results));print(repetitions,seed,'MATCH',results[0]['score'],results[0]['refit_cpu'],results[1]['refit_cpu'],flush=True)
for repetitions in a.reps:
 subset=[x for x in rows if x['repetitions']==repetitions]
 print('MEAN',repetitions,[(kind,sum(x['results'][i]['refit_cpu']for x in subset)/len(subset),sum(x['results'][i]['cpu']for x in subset)/len(subset))for i,kind in enumerate(['reference','fast'])],flush=True)
(root/f'tools/experiments/round6_exact_trie_compare{"_check"if a.check else""}.json').write_text(json.dumps(rows,indent=2))

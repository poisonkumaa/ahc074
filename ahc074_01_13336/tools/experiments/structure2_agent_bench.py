import pathlib,subprocess,time,re,sys
seeds=[int(x) for x in sys.argv[1].split(',')] if len(sys.argv)>1 else [0]
for importance in [1,2,4,8,16,32,128]:
 scores=[];times=[]
 for seed in seeds:
  data=pathlib.Path(f'tools/in/{seed:04}.txt').read_bytes();started=time.monotonic()
  p=subprocess.run(['tools/target/release/tester','tools/experiments/structure2_agent_exchange','3','8','0','6','0','64','30',str(importance),'3'],input=data,capture_output=True)
  elapsed=time.monotonic()-started;msg=p.stderr.decode();m=re.search(r'Score = (\d+)',msg)
  if m:scores.append(int(m[1]));times.append(elapsed)
  else:print(msg,flush=True)
 print(importance,scores,sum(scores)/len(scores) if scores else None,max(times) if times else None,flush=True)

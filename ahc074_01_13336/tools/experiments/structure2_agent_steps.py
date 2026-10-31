import pathlib,subprocess,time,re,sys
seeds=[int(x) for x in sys.argv[1].split(',')] if len(sys.argv)>1 else [0,1,2,3,4]
for steps in [0,1,3,5,10,20,30]:
 scores=[];times=[]
 for seed in seeds:
  data=pathlib.Path(f'tools/in/{seed:04}.txt').read_bytes();started=time.monotonic()
  p=subprocess.run(['tools/target/release/tester','tools/experiments/structure2_agent_exchange','3','8','0','6','0','64',str(steps),'16','3'],input=data,capture_output=True)
  elapsed=time.monotonic()-started;msg=p.stderr.decode();m=re.search(r'Score = (\d+)',msg)
  if m:scores.append(int(m[1]));times.append(elapsed)
  else:print(msg,flush=True)
 print(steps,scores,sum(scores)/len(scores) if scores else None,max(times) if times else None,flush=True)

import pathlib,subprocess,re,sys
seeds=[int(x) for x in sys.argv[1].split(',')] if len(sys.argv)>1 else [0]
for steps,reg in [(0,0),(100,0),(100,1),(100,2),(100,4),(250,1),(250,2)]:
 scores=[];times=[]
 for seed in seeds:
  data=pathlib.Path(f'tools/in/{seed:04}.txt').read_bytes()
  p=subprocess.run(['tools/experiments/templates_agent_weighted_pattern','1024','3','3','0','2','16','.5',str(steps),'256',str(reg)],input=data,capture_output=True)
  msg=p.stderr.decode();m=re.search(r'TOTAL (\d+).*CPU ([\d.]+)',msg)
  if m:scores.append(int(m[1]));times.append(float(m[2]))
  else:print(msg,flush=True)
 print(steps,reg,scores, sum(scores)/len(scores) if scores else None,max(times) if times else None,flush=True)

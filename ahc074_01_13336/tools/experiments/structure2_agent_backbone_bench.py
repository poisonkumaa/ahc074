import pathlib,subprocess,time,re,sys
seeds=[int(x) for x in sys.argv[1].split(',')] if len(sys.argv)>1 else [0]
configs=[(1,1,8),(1,2,8),(1,3,8),(1,4,8),(3,1,8),(3,2,8),(3,3,8),(1,3,16),(1,3,32),(1,5,8)]
for initial,passes,weight in configs:
 scores=[];times=[]
 for seed in seeds:
  data=pathlib.Path(f'tools/in/{seed:04}.txt').read_bytes()
  p=subprocess.run(['tools/target/release/tester','tools/experiments/structure2_agent_backbone',str(initial),'8','0','8','0','64',str(passes),str(weight)],input=data,capture_output=True)
  msg=p.stderr.decode();m=re.search(r'Score = (\d+)',msg);tm=re.search(r'FinishCPU=([\d.]+)',msg)
  if m:scores.append(int(m[1]));times.append(float(tm[1]) if tm else 0)
  else:print(msg,flush=True)
 print((initial,passes,weight),scores,sum(scores)/len(scores) if scores else None,max(times) if times else None,flush=True)

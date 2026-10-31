import pathlib,subprocess,re,sys
seeds=[int(x) for x in sys.argv[1].split(',')] if len(sys.argv)>1 else [0]
configs=[(0,0,0,8),(1,0,0,8),(3,0,0,8),(5,0,0,8),(5,0,4,8),(10,0,4,8),(5,1,0,8),(5,1,4,8),(5,1,4,32),(10,1,4,8)]
if len(sys.argv)>2:configs=[(0,0,0,8),(3,0,0,8),(5,0,4,8),(10,1,4,8)]
for exch,post,penalty,weight in configs:
 scores=[];times=[]
 for seed in seeds:
  p=subprocess.run(['tools/target/release/tester','tools/experiments/structure2_agent_post_exchange','1','8','0','6','0','64','1','8',str(exch),str(post),str(penalty),str(weight)],input=pathlib.Path(f'tools/in/{seed:04}.txt').read_bytes(),capture_output=True)
  msg=p.stderr.decode();m=re.search(r'Score = (\d+)',msg);tm=re.search(r'FinishCPU=([\d.]+)',msg)
  if m:scores.append(int(m[1]));times.append(float(tm[1]) if tm else 0)
  else:print(msg,flush=True)
 print((exch,post,penalty,weight),scores,sum(scores)/len(scores) if scores else None,max(times) if times else None,flush=True)

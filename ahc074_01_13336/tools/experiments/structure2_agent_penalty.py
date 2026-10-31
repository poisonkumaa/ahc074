import pathlib,subprocess,time,re,sys
seeds=[int(x) for x in sys.argv[1].split(',')] if len(sys.argv)>1 else [0]
for penalty,cap in [(0,6),(0,8),(2,192),(4,192),(8,192),(16,192),(32,192)]:
 scores=[]
 for seed in seeds:
  data=pathlib.Path(f'tools/in/{seed:04}.txt').read_bytes()
  p=subprocess.run(['tools/target/release/tester','tools/experiments/structure2_agent_exchange','3','8','0','6','0','64','30','16','3',str(penalty),str(cap)],input=data,capture_output=True)
  msg=p.stderr.decode();m=re.search(r'Score = (\d+)',msg)
  if m:scores.append(int(m[1]))
  else:print(msg,flush=True)
 print(penalty,cap,scores,sum(scores)/len(scores) if scores else None,flush=True)

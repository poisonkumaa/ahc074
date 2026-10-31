import pathlib,subprocess,re,sys
seeds=[int(x) for x in sys.argv[1].split(',')] if len(sys.argv)>1 else [0]
for extra in [6,8,12,16]:
 for passes in [1,2,3]:
  scores=[];times=[]
  for seed in seeds:
   p=subprocess.run(['tools/target/release/tester','tools/experiments/structure2_agent_backbone','1','8','0',str(extra),'0','64',str(passes),'8'],input=pathlib.Path(f'tools/in/{seed:04}.txt').read_bytes(),capture_output=True)
   msg=p.stderr.decode();m=re.search(r'Score = (\d+)',msg);tm=re.search(r'FinishCPU=([\d.]+)',msg)
   if m:scores.append(int(m[1]));times.append(float(tm[1]) if tm else 0)
   else:print(msg,flush=True)
  print((extra,passes),scores,sum(scores)/len(scores) if scores else None,max(times) if times else None,flush=True)

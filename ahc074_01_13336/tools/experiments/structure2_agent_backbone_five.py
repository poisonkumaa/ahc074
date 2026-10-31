import pathlib,subprocess,re,sys
seeds=[int(x) for x in sys.argv[1].split(',')] if len(sys.argv)>1 else [0,1,2,3,4]
for extra,passes,weight in [(6,1,8),(6,1,16),(6,1,32),(6,2,8),(6,2,32),(8,1,8),(8,3,32)]:
 scores=[];times=[]
 for seed in seeds:
  p=subprocess.run(['tools/target/release/tester','tools/experiments/structure2_agent_backbone','1','8','0',str(extra),'0','64',str(passes),str(weight)],input=pathlib.Path(f'tools/in/{seed:04}.txt').read_bytes(),capture_output=True)
  msg=p.stderr.decode();m=re.search(r'Score = (\d+)',msg);tm=re.search(r'FinishCPU=([\d.]+)',msg)
  if m:scores.append(int(m[1]));times.append(float(tm[1]) if tm else 0)
  else:print(msg,flush=True)
 print((extra,passes,weight),scores,sum(scores)/len(scores) if scores else None,max(times) if times else None,flush=True)

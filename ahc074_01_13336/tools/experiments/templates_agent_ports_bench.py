import pathlib,subprocess,re,sys
seeds=[int(x) for x in sys.argv[1].split(',')] if len(sys.argv)>1 else [0]
for count,scheme in [(0,0),(1,2),(2,2),(3,2),(4,2),(6,2),(8,2),(4,3),(4,5),(8,5)]:
 scores=[];times=[]
 for seed in seeds:
  data=pathlib.Path(f'tools/in/{seed:04}.txt').read_bytes()
  p=subprocess.run(['tools/experiments/templates_agent_ports_fast','1024','3','3','0','2','16','.5','0','256','0','64',str(count),str(scheme)],input=data,capture_output=True)
  msg=p.stderr.decode();m=re.search(r'TOTAL (\d+).*CPU ([\d.]+)',msg)
  if m:scores.append(int(m[1]));times.append(float(m[2]))
  else:print(msg,flush=True)
 print(count,scheme,scores,sum(scores)/len(scores) if scores else None,max(times) if times else None,flush=True)

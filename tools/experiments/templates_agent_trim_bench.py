import pathlib,subprocess,re,sys
seeds=[int(x) for x in sys.argv[1].split(',')] if len(sys.argv)>1 else [0]
for span in [64,48,40,32,24,16,8]:
 scores=[];times=[]
 for seed in seeds:
  data=pathlib.Path(f'tools/in/{seed:04}.txt').read_bytes()
  p=subprocess.run(['tools/experiments/templates_agent_trim','1024','3','3','0','2','16','.5','0','256','0',str(span)],input=data,capture_output=True)
  msg=p.stderr.decode();m=re.search(r'TOTAL (\d+).*CPU ([\d.]+)',msg)
  if m:scores.append(int(m[1]));times.append(float(m[2]))
  else:print(msg,flush=True)
 print(span,scores,sum(scores)/len(scores) if scores else None,max(times) if times else None,flush=True)

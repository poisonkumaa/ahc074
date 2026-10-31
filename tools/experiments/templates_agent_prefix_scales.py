import subprocess,pathlib,re,sys
seeds=[int(x) for x in sys.argv[1].split(',')] if len(sys.argv)>1 else [0]
for scale in [1,2,3,4,6,8,12,16,24,32,48,64,128,16384]:
    scores=[]
    for seed in seeds:
        data=pathlib.Path(f'tools/in/{seed:04}.txt').read_bytes()
        p=subprocess.run(['tools/experiments/templates_agent_prefix','1024','3','3','0','2','16','.5','0',str(scale)],input=data,capture_output=True)
        m=re.search(r'TOTAL (\d+)',p.stderr.decode())
        if m:scores.append(int(m[1]))
        else:print(p.stderr.decode(),flush=True)
    print(scale,scores,sum(scores)/len(scores) if scores else None,flush=True)

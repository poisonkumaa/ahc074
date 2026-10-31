import subprocess, pathlib, sys, re
seeds=[int(x) for x in sys.argv[1].split(',')] if len(sys.argv)>1 else [0]
modes=[int(x) for x in sys.argv[2].split(',')] if len(sys.argv)>2 else list(range(9))
scale=sys.argv[3] if len(sys.argv)>3 else '16384'
for mode in modes:
    values=[]
    for seed in seeds:
        data=pathlib.Path(f'tools/in/{seed:04}.txt').read_bytes()
        result=subprocess.run(['tools/experiments/templates_agent_prefix','1024','3','3','0','2','16','.5',str(mode),scale],input=data,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        msg=result.stderr.decode(); match=re.search(r'TOTAL (\d+)',msg)
        if match: values.append(int(match[1]));print(mode,seed,match[1],flush=True)
        else: print('ERROR',mode,seed,msg,flush=True)
    if values: print('MEAN',mode,sum(values)/len(values),flush=True)

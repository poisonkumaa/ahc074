from pathlib import Path
import subprocess,re,time,json,sys
p=Path(__file__).resolve().parent;root=p.parents[1]
binary=sys.argv[1] if len(sys.argv)>1 else 'policy2_agent_weighted'
settings=[[str(w)] for w in [256,16,8,6,4,3,2,1]]
results=[]
for args in settings:
    scores=[];cpu=[]
    for seed in range(3):
        run=subprocess.run([str(root/'tools/target/release/tester'),str(p/binary),*args],input=(root/f'tools/in/{seed:04}.txt').read_text(),capture_output=True,text=True)
        m=re.search(r'Score = (\d+)',run.stderr)
        if not m:raise RuntimeError(run.stderr)
        score=int(m.group(1));lines=run.stdout.splitlines();idx=192;moves=0
        for _ in range(1024):
            n=int(lines[idx]);moves+=n;idx+=n+1
        assert idx==len(lines) and moves==score,(args,seed,score,moves)
        scores.append(score);cpu.append(float(re.search(r'FinishCPU=([\d.]+)',run.stderr).group(1)))
    r=dict(args=args,scores=scores,mean=sum(scores)/len(scores),cpu=max(cpu));results.append(r);print(r,flush=True)
(p/f'{binary}_grid.json').write_text(json.dumps(results,indent=2))

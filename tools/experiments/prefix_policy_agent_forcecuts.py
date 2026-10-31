from pathlib import Path
import subprocess,re,time,json
p=Path(__file__).resolve().parent;root=p.parents[1]
results=[]
for limit in ['0.0','0.06','0.18','0.45']:
    binary=p/('prefix_policy_agent_cut'+limit.replace('.',''))
    subprocess.run(['g++','-std=c++17','-O2',f'-DTRAINING_CPU_LIMIT={limit}',str(p/'prefix_policy_agent_hardguard.cpp'),'-o',str(binary)],check=True)
    for seed in [0,50,99]:
        run=subprocess.run([str(root/'tools/target/release/tester'),str(binary)],input=(root/f'tools/in/{seed:04}.txt').read_text(),text=True,capture_output=True)
        match=re.search(r'Score = (\d+)',run.stderr)
        if not match:raise RuntimeError(run.stderr)
        score=int(match.group(1));lines=run.stdout.splitlines();idx=192;ops=0;maximum=0
        for _ in range(1024):
            n=int(lines[idx]);ops+=n;maximum=max(maximum,n);idx+=n+1
        assert idx==len(lines) and ops==score,(limit,seed,idx,len(lines),ops,score)
        levels=list(map(int,re.findall(r'levels=(\d+)',run.stderr)))
        cpu=float(re.search(r'FinishCPU=([\d.]+)',run.stderr).group(1))
        result=dict(limit=float(limit),seed=seed,score=score,max_ops=maximum,levels=levels,cpu=cpu)
        print(result,flush=True);results.append(result)
(p/'prefix_policy_agent_hardguard_cut_results.json').write_text(json.dumps(results,indent=2))

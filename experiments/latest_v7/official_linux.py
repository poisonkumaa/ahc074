"""Run the supplied reactive tester, then independently replay every operation."""
from pathlib import Path
import argparse,importlib.util,json,re,subprocess,time
p=Path(__file__).resolve().parent
root=p.parents[1]
spec=importlib.util.spec_from_file_location('replay',root.parent/'archived2_11434/experiments/bench_run.py')
ref=importlib.util.module_from_spec(spec);spec.loader.exec_module(ref)
a=argparse.ArgumentParser();a.add_argument('exe',type=Path);a.add_argument('--seeds',default='0,1,2');a.add_argument('--report',type=Path,required=True)
args=a.parse_args();rows=[]
for seed in map(int,args.seeds.split(',')):
    data=(root/'tools/in'/f'{seed:04d}.txt').read_bytes();start=time.perf_counter()
    run=subprocess.run([str(root/'tools/target/release/tester'),str(args.exe.resolve())],input=data,capture_output=True,timeout=20)
    err=run.stderr.decode();assert run.returncode==0,err
    result=ref.replay(data.decode(),run.stdout.decode())
    score=re.findall(r'^Score\s*=\s*(\d+)\s*$',err,re.M);assert score and int(score[-1])==result['score'],err
    result.update(seed=seed,wall_seconds=time.perf_counter()-start,stderr=err)
    rows.append(result);print(seed,result['score'],result['errors'],result['wall_seconds'],flush=True)
args.report.write_text(json.dumps(dict(exe=str(args.exe),cases=rows),indent=2))

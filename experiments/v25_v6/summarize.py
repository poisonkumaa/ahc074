from pathlib import Path
import json, re, statistics, argparse

p=argparse.ArgumentParser()
p.add_argument('report',nargs='+')
a=p.parse_args()
for name in a.report:
    report=json.loads(Path(name).read_text(encoding='utf-8'))
    by=report['variants']
    rows=report['cases']
    print(name)
    for key in by:
        cs=[c for c in rows if c['variant']==key]
        if not cs:continue
        cpu=[]
        for c in cs:
            found=re.search(r'BUDGET design_ready[^\n]* cpu=([\d.]+)',c['stderr'])
            if found:cpu.append(float(found[1]))
        print(key,'n',len(cs),'mean',statistics.mean(c['score'] for c in cs),
              'range',(min(c['score'] for c in cs),max(c['score'] for c in cs)),
              'errors',sum(c['errors'] for c in cs),
              'max_wall',round(max(c['wall_seconds'] for c in cs),6),
              'cpu_mean',round(statistics.mean(cpu),6) if cpu else None,
              'cpu_max',max(cpu) if cpu else None)
    if len(by)==2:
        keys=list(by);cases={}
        for c in rows:cases.setdefault(c['seed'],{})[c['variant']]=c
        ds=[v[keys[1]]['score']-v[keys[0]]['score'] for v in cases.values() if len(v)==2]
        if ds:print('candidate - baseline:',statistics.mean(ds),
                    'win/tie/loss',sum(d<0 for d in ds),sum(d==0 for d in ds),sum(d>0 for d in ds))

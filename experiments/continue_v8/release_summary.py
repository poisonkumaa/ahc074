"""Summarize the two untouched normal-budget validation batches together."""
from pathlib import Path
import hashlib,json,math,re,statistics

root=Path(__file__).resolve().parent
names=['final_filter40.json','final_filter_confirm40.json']
reports=[json.loads((root/name).read_text(encoding='utf-8')) for name in names]
assert all('summary' in report for report in reports), 'Wait for both complete batches'
assert reports[0]['variants']==reports[1]['variants']
rows=[row for report in reports for row in report['cases']]
def cpu(row):
    return float(re.search(r'^BUDGET design_ready[^\n]* cpu=([0-9.]+)',row['stderr'],re.M)[1])
summary={}
for variant in ['baseline','candidate']:
    selected=[row for row in rows if row['variant']==variant]
    assert len(selected)==80 and len({row['seed'] for row in selected})==80
    summary[variant]=dict(count=80,mean=statistics.mean(row['score'] for row in selected),
        total=sum(row['score'] for row in selected),errors=sum(row['errors'] for row in selected),
        max_round_moves=max(row['max_round_moves'] for row in selected),
        cpu_mean=statistics.mean(map(cpu,selected)),cpu_max=max(map(cpu,selected)),
        wall_max=max(row['wall_seconds'] for row in selected))
by={}
for row in rows: by.setdefault(row['seed'],{})[row['variant']]=row['score']
delta=[row['candidate']-row['baseline'] for row in by.values()]
summary['paired']=dict(mean_delta=statistics.mean(delta),total_delta=sum(delta),
    standard_error=statistics.stdev(delta)/math.sqrt(len(delta)),
    wins=sum(d<0 for d in delta),ties=sum(d==0 for d in delta),losses=sum(d>0 for d in delta),
    min_delta=min(delta),max_delta=max(delta))
summary['source_sha256']=hashlib.sha256((root/'alternative_filter.cpp').read_bytes()).hexdigest()
summary['reports']=names
(root/'release_summary80.json').write_text(json.dumps(summary,indent=2)+'\n',encoding='utf-8')
print(json.dumps(summary,indent=2))

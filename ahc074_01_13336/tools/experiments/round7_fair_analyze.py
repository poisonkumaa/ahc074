from pathlib import Path
import json,re,statistics,sys
p=Path(__file__).parent
for name in sys.argv[1:]:
 data=json.loads((p/name).read_text());rows=[]
 for case in data['cases']:
  match=re.search(r'FairMetrics (\{[^\n]+\})',case['log'])
  if not match:continue
  products=json.loads(match[1])['products'];groups=[]
  for low,high in [(0,42),(43,52),(53,56),(57,60),(61,64)]:
   selected=[v for v in products if low<=v['own_length']<=high]
   if selected:groups.append(dict(range=[low,high],count=len(selected),cost=statistics.mean(v['cost'] for v in selected),records=statistics.mean(v['records'] for v in selected),own_probability=statistics.mean(v['own_probability'] for v in selected),own_paper_usage=statistics.mean(v['own_paper_usage'] for v in selected)))
  x=[v['own_length'] for v in products];y=[v['cost'] for v in products];mx=statistics.mean(x);my=statistics.mean(y);corr=sum((a-mx)*(b-my) for a,b in zip(x,y))/(sum((a-mx)**2 for a in x)*sum((b-my)**2 for b in y))**.5
  row=dict(seed=case['seed'],mean=my,min=min(y),max=max(y),correlation=corr,groups=groups);rows.append(row);print(json.dumps(row),flush=True)
 (p/name.replace('.json','_analysis.json')).write_text(json.dumps(rows,indent=2))

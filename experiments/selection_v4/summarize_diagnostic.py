from pathlib import Path
import json
import re
import statistics

here=Path(__file__).resolve().parent
report=json.loads((here/'diagnostic.json').read_text())
rows=[]
for seed in range(6):
    log=(here/f'diagnostic_logs/{seed:04d}.stderr').read_text()
    selected=int(re.search(r'DIAGNOSTIC_COMPLETE selected=(\d+)',log)[1])
    choices={int(c['variant'][5:]):c['moves'] for c in report['cases'] if c['seed']==seed}
    best=min(choices,key=choices.get)
    rows.append(dict(seed=seed,selected=selected,selected_moves=choices[selected],best=best,best_moves=choices[best],gap=choices[selected]-choices[best]))
summary=dict(note='Retrospective best on actual orders is diagnostic only; never used by submitted code.',cases=rows,
    selected_mean=statistics.mean(r['selected_moves'] for r in rows),
    oracle_mean=statistics.mean(r['best_moves'] for r in rows),
    mean_gap=statistics.mean(r['gap'] for r in rows))
(here/'diagnostic_summary.json').write_text(json.dumps(summary,indent=2))
print(json.dumps(summary,indent=2))

from pathlib import Path
import json
p=Path(__file__).resolve().parent
base='/mnt/c/Users/dokuk/Documents/Competitive_programing/Atcoder/AHC/ahc074'
common={'BEAM_FIXTURE_DIR':base+'/experiments/continue_v8/fixtures','NOV8_ROUTE_END':30,'NOV8_RELAX_END':30,'NOV8_SHORT_END':30,'NOV5_RELAX_BUDGET':10}
def row(name,exe,fixed=False,**env):
    return dict(name=name,exe=f'{base}/experiments/continue_v8/{exe}_linux',env={**(common if fixed else {}),**env})
specs={
 'dump':[row('dump','baseline_dump',BEAM_FIXTURE_DIR=common['BEAM_FIXTURE_DIR'])],
 'fixed_initial':[row('baseline','baseline_fixture',True),
                  row('head','alternative_both_fixture',True,NO_PAIR68=1),
                  row('pair','alternative_both_fixture',True,NO_HEADVALUE64=1),
                  row('both','alternative_both_fixture',True),
                  row('pre_utility','design_pre_fixture',True)],
 'pre_normal':[row('baseline','baseline'),row('pre_holes','design_pre',DESIGN_PRE_UTILITY=0),row('pre_both','design_pre')],
}
for name,rows in specs.items():(p/(name+'_spec.json')).write_text(json.dumps(rows,indent=2),encoding='utf-8')

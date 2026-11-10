from pathlib import Path
import json
p=Path(__file__).resolve().parent
base='/mnt/c/Users/dokuk/Documents/Competitive_programing/Atcoder/AHC/ahc074'
common={'BEAM_FIXTURE_DIR':base+'/experiments/beam_v1/fixtures','NOV8_ROUTE_END':30,'NOV8_RELAX_END':30,'NOV8_SHORT_END':30,'NOV5_RELAX_BUDGET':10}
def row(name,exe,fixture=False,**env):
    return dict(name=name,exe=f'{base}/experiments/latest_v7/{exe}_linux',env={**(common if fixture else {}),**env})
specs={
    'noop':[row('baseline','baseline'),row('noop','route_noop')],
    'mdp_fixed':[row('baseline','baseline_fixture',True),row('mdp1','future_mdp_fixture',True,ENDPOINT_MDP=1),row('mdp2','future_mdp_fixture',True,ENDPOINT_MDP=2)],
    'cache_fixed':[row('baseline','struct_cache_fixture',True,STRUCT_VALUE_INVALIDATE=0,STRUCT_VALUE_VERIFY=1),row('cache','struct_cache_fixture',True,STRUCT_VALUE_INVALIDATE=1,STRUCT_VALUE_VERIFY=1)],
    'final_fixed':[row('baseline','baseline_fixture',True),row('final','final_fixture',True)],
    'post_fixed':[row('baseline','baseline_fixture',True),row('post','future_post_fixture',True)],
}
for name,rows in specs.items():(p/(name+'_spec.json')).write_text(json.dumps(rows,indent=2),encoding='utf-8')

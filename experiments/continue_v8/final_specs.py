from pathlib import Path
import json
p=Path(__file__).resolve().parent
base='/mnt/c/Users/dokuk/Documents/Competitive_programing/Atcoder/AHC/ahc074/experiments/continue_v8'
fixed={'BEAM_FIXTURE_DIR':base+'/fixtures','NOV8_ROUTE_END':30,'NOV8_RELAX_END':30,'NOV8_SHORT_END':30,'NOV5_RELAX_BUDGET':10}
def row(name,exe,freeze=False,**env):
    return dict(name=name,exe=f'{base}/{exe}_linux',env={**(fixed if freeze else {}),**env})
specs={
 'guard6': [row('both','alternative_both_fixture',True),row('lazy0','alternative_lazy_fixture',True,PAIRS_TIME_GUARD=0),row('guard2','alternative_guard_fixture',True),row('core0','route_pairvalue_fixture',True,PAIRS_TIME_GUARD=0)],
 'fixed_hold': [row('baseline','baseline_fixture',True),row('head','alternative_both_fixture',True,NO_PAIR68=1),row('both','alternative_both_fixture',True),row('guard2','alternative_guard_fixture',True)],
 'normal_hold': [row('baseline','baseline'),row('both','alternative_both'),row('guard2','alternative_guard')],
 'neighbors6': [row('guard2','alternative_guard_fixture',True),row('shift','alternative_shift_fixture',True),row('equal','alternative_equal_fixture',True)],
 'zero6': [row('guard2','alternative_guard_fixture',True),row('zero','alternative_zero_fixture',True)],
 'filter_verify': [row('original','alternative_guard_fixture',True,PAIRS_BUDGET=10),row('filter','alternative_filterzero_fixture',True,PAIR68_ZERO=0,PAIRS_BUDGET=10,PAIRS_VERIFY=1)],
 'filter_hold': [row('guard2','alternative_guard_fixture',True),row('filter','alternative_filterzero_fixture',True,PAIR68_ZERO=0),row('filterzero','alternative_filterzero_fixture',True)],
 'final_zero': [row('baseline','baseline'),row('candidate','alternative_filterzero')],
 'final_filter': [row('baseline','baseline'),row('candidate','alternative_filter')],
}
for name,rows in specs.items(): (p/(name+'_spec.json')).write_text(json.dumps(rows,indent=2),encoding='utf-8')

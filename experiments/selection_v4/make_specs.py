from pathlib import Path
import json
here=Path(__file__).resolve().parent
base='/mnt/c/Users/dokuk/Documents/Competitive_programing/Atcoder/AHC/ahc074'
common=dict(BEAM_FIXTURE_DIR=f'{base}/experiments/beam_v1/fixtures',NOV8_ROUTE_END=30,NOV8_RELAX_END=30,NOV8_SHORT_END=30,NOV5_RELAX_BUDGET=10)
def save(name,variants):
    (here/f'{name}_spec.json').write_text(json.dumps(variants,indent=2),encoding='utf-8')
save('unique',[dict(name=name,exe='experiments/selection_v4/route_unique_fixture_linux',env=common|dict(ROUTE_UNIQUE=mode)) for name,mode in [('baseline',0),('unique',1)]])
save('sampling',[dict(name=f'mode{mode}',exe='experiments/selection_v4/selection_sampling_linux',env=dict(SEL4_MODE=mode)) for mode in [0,1,3]])
save('structure',[dict(name=name,exe='experiments/selection_v4/structure_fixture_linux',env=common|dict(STRUCTURE_ASSIGN=enable,STRUCTURE_EMPIRICAL=empirical,STRUCTURE_CPU=2,STRUCTURE_END=30)) for name,enable,empirical in [('baseline',0,1),('empirical',1,1),('original_f',1,0)]])
save('reorder',[dict(name=name,exe='experiments/selection_v4/route_reorder_fixture_linux',env=common|dict(ROUTE_REORDER=enable,ROUTE_REORDER_NODES=nodes,ROUTE_REORDER_CPU=10)) for name,enable,nodes in [('baseline',0,1000),('reorder1k',1,1000),('reorder10k',1,10000)]])
research=dict(NOV8_PACK_END=30,NOV8_MATCH_END=30,NOV8_LAYOUT_END=30,NOV8_POOL_END=30,NOV8_SELECT_END=30,NOV8_SELECT_HARD=30,NOV8_HOLES_END=30,NOV8_INIT_END=30,NOV8_MDP_END=30,NOV8_ROUTE_END=30,NOV8_RELAX_END=30,NOV8_SHORT_END=30,NOV5_RELAX_BUDGET=10)
save('core4',[dict(name=name,exe='experiments/selection_v4/core4_candidate_linux',env=research|dict(CORE4_MODE=mode,CORE4_W=width)) for name,mode,width in [('baseline',0,30),('core4_w30',2,30),('core4_w32',2,32),('mixed_w32',1,32)]])

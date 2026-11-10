from pathlib import Path
import json
p=Path(__file__).resolve().parent
fixture='/mnt/c/Users/dokuk/Documents/Competitive_programing/Atcoder/AHC/ahc074/experiments/beam_v1/fixtures'
common={'BEAM_FIXTURE_DIR':fixture,'NOV8_ROUTE_END':30,'NOV8_SHORT_END':30,'NOV8_RELAX_END':30}
def variant(name,exe,**options):
    return dict(name=name,exe=f'experiments/beam_v1/{exe}_linux',env=common|options)
base=variant('baseline','fixture_baseline')
grids={
 'early':[base,variant('early3','fixture_early',EARLY_BEAM=1,EARLY_BEAM_CPU=10,EARLY_BEAM_WIDTH=3),
          variant('early6','fixture_early',EARLY_BEAM=1,EARLY_BEAM_CPU=10,EARLY_BEAM_WIDTH=6,EARLY_BEAM_NODES=36)],
 'post':[base,variant('post1','fixture_post',POST_BEAM=1,POST_BEAM_WIDTH=1,POST_BEAM_CPU=10,POST_BEAM_CALL_CPU=0),
         variant('post4','fixture_post',POST_BEAM=1,POST_BEAM_WIDTH=4,POST_BEAM_CPU=10,POST_BEAM_CALL_CPU=0),
         variant('post8','fixture_post',POST_BEAM=1,POST_BEAM_WIDTH=8,POST_BEAM_OPTIONS=8,POST_BEAM_CPU=10,POST_BEAM_CALL_CPU=0)],
 'joint':[base,variant('joint0','fixture_joint',EARLY_BEAM=1,EARLY_BEAM_CPU=10),
          variant('joint_minus','fixture_joint',EARLY_BEAM=1,EARLY_BEAM_CPU=10,EARLY_POST_MIN_GAIN=-1,EARLY_BEAM_NODES=64)],
 'trace':[base,variant('trace4','fixture_trace',TRACE_BEAM=1,TRACE_BEAM_WIDTH=4,TRACE_BEAM_BUDGET=10,TRACE_BEAM_CALL=1),
          variant('trace8','fixture_trace',TRACE_BEAM=1,TRACE_BEAM_WIDTH=8,TRACE_BEAM_BUDGET=10,TRACE_BEAM_CALL=1)],
 'diverse':[base,variant('diverse1','fixture_diverse',POST_BEAM=1,POST_BEAM_WIDTH=4,POST_BEAM_CPU=10,POST_BEAM_CALL_CPU=0,POST_BEAM_DIVERSITY=1),
            variant('diverse2','fixture_diverse',POST_BEAM=1,POST_BEAM_WIDTH=4,POST_BEAM_CPU=10,POST_BEAM_CALL_CPU=0,POST_BEAM_DIVERSITY=2)],
 'post_holdout':[base,variant('post4','fixture_post',POST_BEAM=1,POST_BEAM_WIDTH=4,POST_BEAM_CPU=10,POST_BEAM_CALL_CPU=0)],
 'fast':[variant('post4','fixture_post',POST_BEAM=1,POST_BEAM_WIDTH=4,POST_BEAM_CPU=10,POST_BEAM_CALL_CPU=0),
         variant('fast4','fixture_fast_post',POST_BEAM=1,POST_BEAM_WIDTH=4,POST_BEAM_CPU=10,POST_BEAM_CALL_CPU=0)],
 'combo':[base,variant('combo','fixture_combo',POST_BEAM=1,POST_BEAM_WIDTH=4,POST_BEAM_CPU=10,POST_BEAM_CALL_CPU=0,TRACE_BEAM=1,TRACE_BEAM_WIDTH=4,TRACE_BEAM_BUDGET=10,TRACE_BEAM_CALL=1)],
 'holdout':[base,variant('post4','fixture_fast_post',POST_BEAM=1,POST_BEAM_WIDTH=4,POST_BEAM_CPU=10,POST_BEAM_CALL_CPU=0),
            variant('trace4','fixture_trace',TRACE_BEAM=1,TRACE_BEAM_WIDTH=4,TRACE_BEAM_BUDGET=10,TRACE_BEAM_CALL=1),
            variant('combo','fixture_combo',POST_BEAM=1,POST_BEAM_WIDTH=4,POST_BEAM_CPU=10,POST_BEAM_CALL_CPU=0,TRACE_BEAM=1,TRACE_BEAM_WIDTH=4,TRACE_BEAM_BUDGET=10,TRACE_BEAM_CALL=1)],
 'v2':[base,variant('trace_v2_4','fixture_trace_v2',TRACE_BEAM=1,TRACE_BEAM_WIDTH=4,TRACE_BEAM_BUDGET=10,TRACE_BEAM_CALL=1),
       variant('trace_v2_8','fixture_trace_v2',TRACE_BEAM=1,TRACE_BEAM_WIDTH=8,TRACE_BEAM_BUDGET=10,TRACE_BEAM_CALL=1)],
 'strict':[base,variant('strict','fixture_strict',POST_BEAM=1,POST_BEAM_WIDTH=4,POST_BEAM_CPU=10,POST_BEAM_CALL_CPU=0),
           variant('strict_plus1','fixture_strict',POST_BEAM=1,POST_BEAM_WIDTH=4,POST_BEAM_CPU=10,POST_BEAM_CALL_CPU=0,POST_BEAM_EXTRA_LOANS=1)],
 'validated':[base,variant('validated','fixture_validated',POST_BEAM=1,POST_BEAM_WIDTH=4,POST_BEAM_CPU=10,POST_BEAM_CALL_CPU=0,BEAM_VALIDATE=1,BEAM_VALIDATE_CPU=10)],
}
for name,variants in grids.items():
    (p/f'{name}_spec.json').write_text(json.dumps(variants,indent=2),encoding='utf-8')

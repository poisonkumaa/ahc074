from pathlib import Path
import json
p=Path(__file__).resolve().parent
common=dict(BEAM_FIXTURE_DIR='/mnt/c/Users/dokuk/Documents/Competitive_programing/Atcoder/AHC/ahc074/experiments/beam_v1/fixtures',NOV8_ROUTE_END=30,NOV8_RELAX_END=30,NOV8_SHORT_END=30,NOV5_RELAX_BUDGET=10)
base=dict(name='baseline',exe='experiments/beam_v1/fixture_baseline_linux',env=common)
variants=[base]
for count in [1,3]:
    variants.append(dict(name=f'portfolio{count}',exe='experiments/design_v3/fixture_portfolio_linux',env=common|dict(ROUTE_PORTFOLIO=1,PORTFOLIO_CPU=10,PORTFOLIO_TRIES=count)))
(p/'portfolio_spec.json').write_text(json.dumps(variants,indent=2),encoding='utf-8')
def variant(name,exe,**opts):
    return dict(name=name,exe=f'experiments/design_v3/{exe}_linux',env=common|opts)
development=[base,
    variant('quick3','fixture_quick_portfolio',ROUTE_PORTFOLIO=1,PORTFOLIO_CPU=10,PORTFOLIO_TRIES=3),
    variant('bank_donor','fixture_global',V3_BANK_ORDER=1,V3_ORDER_CPU=1,V3_ORDER_END=30),
    variant('bank_coverage','fixture_global',V3_BANK_ORDER=1,V3_ORDER_CPU=1,V3_ORDER_END=30,V3_ORDER_DONOR=0),
    variant('bucket_dp','fixture_bucket',BUCKET_JOINT_REPAIR=1,BUCKET_JOINT_CPU=10)]
(p/'development_spec.json').write_text(json.dumps(development,indent=2),encoding='utf-8')
(p/'validation_spec.json').write_text(json.dumps(development[:3],indent=2),encoding='utf-8')
full_common=dict(NOV8_INIT_END=30,NOV8_MDP_END=30,NOV8_ROUTE_END=30,NOV8_RELAX_END=30,NOV8_SHORT_END=30)
selection=[]
for name,options in [('baseline',dict(FINAL_RACE=0)),('final128',dict(FINAL_RACE=1,FINAL_RACE_QUERIES=128)),('final256',dict(FINAL_RACE=1,FINAL_RACE_QUERIES=256))]:
    selection.append(dict(name=name,exe='experiments/design_v3/selection_candidate_linux',env=full_common|dict(FINAL_RACE_CPU=10,FINAL_RACE_END=30)|options))
(p/'selection_spec.json').write_text(json.dumps(selection,indent=2),encoding='utf-8')
normal=[dict(name='baseline',exe='experiments/beam_v1/baseline_linux',env={})]
for name,options in [('portfolio1',dict(PORTFOLIO_TRIES=1,PORTFOLIO_PAIRS=100,PORTFOLIO_TRIPLES=24,PORTFOLIO_CPU=.4,NOV5_RELAX_BUDGET=.7)),('quick3',dict(PORTFOLIO_TRIES=3,PORTFOLIO_CPU=.55,NOV5_RELAX_BUDGET=.9))]:
    normal.append(dict(name=name,exe='experiments/design_v3/quick_portfolio_linux',env=dict(ROUTE_PORTFOLIO=1)|options))
(p/'normal_spec.json').write_text(json.dumps(normal,indent=2),encoding='utf-8')
weighted=[base,variant('weighted12','fixture_weighted',V3_BANK_ORDER=1,V3_ORDER_CPU=1,V3_ORDER_END=30),variant('weighted24','fixture_weighted',V3_BANK_ORDER=1,V3_ORDER_CPU=1,V3_ORDER_END=30,V3_ORDER_HEADS=24)]
(p/'weighted_spec.json').write_text(json.dumps(weighted,indent=2),encoding='utf-8')
charged=[]
fixture_env=dict(BEAM_FIXTURE_DIR=common['BEAM_FIXTURE_DIR'])
for charge in [1.2,1.35]:
    for name in ['baseline','adaptive']:
        charged.append(dict(name=f'{name}_{charge}',exe=f'experiments/design_v3/charged_{name}_linux',env=fixture_env|dict(FIXTURE_CHARGE=charge)))
(p/'charged_spec.json').write_text(json.dumps(charged,indent=2),encoding='utf-8')
final=[dict(name='baseline',exe='experiments/beam_v1/baseline_linux',env={}),dict(name='adaptive',exe='experiments/design_v3/adaptive_portfolio_linux',env={})]
(p/'final_spec.json').write_text(json.dumps(final,indent=2),encoding='utf-8')
official=[dict(name='baseline',exe='experiments/beam_v1/baseline.exe',env={}),dict(name='adaptive',exe='experiments/design_v3/adaptive_portfolio.exe',env={})]
(p/'official_spec.json').write_text(json.dumps(official,indent=2),encoding='utf-8')
guarded=[base]
for mode in [0,1,2]:
    guarded.append(variant(f'guard{mode}','fixture_guarded',PORTFOLIO_CPU=10,PORTFOLIO_ACCEPT_MODE=mode))
(p/'guarded_spec.json').write_text(json.dumps(guarded,indent=2),encoding='utf-8')
metric=[dict(name=f'weight{w}',exe='experiments/design_v3/metric_candidate_linux',env=full_common|dict(PAIR_CORE_WEIGHT=w)) for w in [4,2,6,8]]
(p/'metric_spec.json').write_text(json.dumps(metric,indent=2),encoding='utf-8')
legal=[metric[0],
    dict(name='legal_sample',exe='experiments/design_v3/legal_metric_candidate_linux',env=full_common|dict(PAIR_LEGAL_METRIC=1,PAIR_LEGAL_CAP=1)),
    dict(name='legal_all',exe='experiments/design_v3/legal_metric_candidate_linux',env=full_common|dict(PAIR_LEGAL_METRIC=1,PAIR_LEGAL_CAP=1,PAIR_LEGAL_ALL=1)),
    dict(name='mix10',exe='experiments/design_v3/metric_mix_linux',env=full_common|dict(PAIR_ALT_MASK=10)),
    dict(name='mix5',exe='experiments/design_v3/metric_mix_linux',env=full_common|dict(PAIR_ALT_MASK=5))]
(p/'legal_mix_spec.json').write_text(json.dumps(legal,indent=2),encoding='utf-8')
design_final=[metric[0],metric[1],legal[1],dict(name='snapshot2',exe='experiments/design_v3/pair_snapshot_candidate_linux',env=full_common|dict(PAIR_SNAPSHOT=1))]
(p/'design_validation_spec.json').write_text(json.dumps(design_final,indent=2),encoding='utf-8')
append=[dict(name='baseline',exe='experiments/design_v3/pair_snapshot_append_linux',env=dict(PAIR_SNAPSHOT=0))]
for count in [1,2]:
    append.append(dict(name=f'append{count}',exe='experiments/design_v3/pair_snapshot_append_linux',env=dict(PAIR_SNAPSHOT=1,PAIR_APPEND_COUNT=count)))
(p/'append_spec.json').write_text(json.dumps(append,indent=2),encoding='utf-8')
confirm=[dict(name='baseline',exe='experiments/beam_v1/baseline_linux',env={}),dict(name='candidate',exe='experiments/design_v3/main_candidate_linux',env={})]
(p/'confirm_spec.json').write_text(json.dumps(confirm,indent=2),encoding='utf-8')
official_confirm=[dict(name='baseline',exe='experiments/beam_v1/baseline.exe',env={}),dict(name='candidate',exe='experiments/design_v3/main_candidate.exe',env={})]
(p/'confirm_official_spec.json').write_text(json.dumps(official_confirm,indent=2),encoding='utf-8')

from pathlib import Path
import json
here=Path(__file__).resolve().parent
root='/mnt/c/Users/dokuk/Documents/Competitive_programing/Atcoder/AHC/ahc074'
prefix='''// Apply the target before standard headers, including bitset::count.
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC target("popcnt")
#endif
'''
(here/'design_popcnt.cpp').write_text(prefix+(here/'design_candidate.cpp').read_text(encoding='utf-8'),encoding='utf-8')
def save(name,variants):
    (here/f'{name}_spec.json').write_text(json.dumps(variants,indent=2))
fixed=dict(BEAM_FIXTURE_DIR=f'{root}/experiments/beam_v1/fixtures',NOV8_ROUTE_END=30,NOV8_RELAX_END=30,NOV8_SHORT_END=30,NOV5_RELAX_BUDGET=10)
variants=[dict(name='baseline',exe='experiments/renewed_v5/main_baseline_fixture_linux',env=fixed)]
for name,options in [('popcnt',{}),('fast_verify',dict(FAST_FIXED_DECK=1,FAST_FIXED_VERIFY=1)),('all',dict(FAST_FIXED_DECK=1,ROUTE_FIXED_ALL=1,ROUTE_FIXED_CPU=10)),('all_capped',dict(FAST_FIXED_DECK=1,ROUTE_FIXED_ALL=1))]:
    variants.append(dict(name=name,exe='experiments/renewed_v5/route_fastall_fixture_linux',env=fixed|options))
save('fixed',variants)
save('design',[dict(name=name,exe='experiments/renewed_v5/design_popcnt_linux',env=dict(DESIGN_MATCHING=mode)) for name,mode in [('baseline',0),('matching',1)]])
save('reloan',[dict(name=name,exe='experiments/renewed_v5/route_reloan_fixture_linux',env=fixed|dict(RELOAN_ENABLE=reloan,TAIL_EXACT_RUN=exact)) for name,reloan,exact in [('baseline',0,0),('reloan',1,0),('exact',0,1),('both',1,1)]])
save('selection_budget',[dict(name=name,exe='experiments/renewed_v5/design_popcnt_linux',env=dict(NOV8_QUERIES=queries)) for name,queries in [('baseline',8192),('race512',512),('race1024',1024)]])
(here/'popcnt_top.cpp').write_text(prefix+(here/'main_baseline.cpp').read_text(encoding='utf-8'),encoding='utf-8')
save('hardware',[dict(name='baseline',exe='experiments/renewed_v5/main_baseline_linux',env={}),dict(name='popcnt',exe='experiments/renewed_v5/popcnt_top_linux',env={})])
save('official',[dict(name='baseline',exe='experiments/renewed_v5/main_baseline.exe',env={}),dict(name='popcnt',exe='experiments/renewed_v5/popcnt_top.exe',env={})])

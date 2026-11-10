from pathlib import Path
import json

here=Path(__file__).resolve().parent
base='/mnt/c/Users/dokuk/Documents/Competitive_programing/Atcoder/AHC/ahc074'
def write(name, variants):
    (here/(name+'.json')).write_text(json.dumps(variants,indent=2),encoding='utf-8')
def variant(name,exe,**env):
    return dict(name=name,exe='experiments/v25_v6/'+exe+'_linux',env=env)

common=dict(BEAM_FIXTURE_DIR=base+'/experiments/beam_v1/fixtures',NOV8_ROUTE_END=30,NOV8_RELAX_END=30,NOV8_SHORT_END=30)
write('route_fixed_spec', [variant(name,'route_quota_fixture',**common,ROUTE_QUOTA_RESERVE=reserve,ROUTE_ALT_FIXPOINT=repeat)
     for name,reserve,repeat in [('control',0,0),('reserve',1,0),('repeat',0,1),('both',1,1)]])
write('design_spec', [variant(name,'design_candidate',DESIGN_FLIP=mix) for name,mix in [('control',0),('half',.5),('exact',1)]])
write('design_verify_spec',[variant('verify','design_candidate',DESIGN_FLIP=1,DESIGN_FLIP_VERIFY=1)])
write('design_validate_spec', [variant('baseline','main_baseline'),variant('half','design_candidate',DESIGN_FLIP=.5)])
write('terminal_fixed_spec',[variant(name,'fast_terminal_fixture',**common,NOV5_RELAX_BUDGET=10,FAST_TERMINAL=level,FAST_TERMINAL_VERIFY=verify)
     for name,level,verify in [('control',0,0),('fused',1,0),('memo',2,0),('verify',2,1)]])
write('anchor_fixed_spec',[variant(name,'route_anchor_fixture',**common,NOV5_RELAX_BUDGET=10,ROUTE_ANCHORED=level)
     for name,level in [('control',0),('anchored',1)]])
write('genesis_spec',[variant(name,'fast_genesis',FAST_GENESIS=level) for name,level in [('control',0),('direct',1)]])
write('genesis_verify_spec',[variant('verify','fast_genesis',FAST_GENESIS=1,FAST_GENESIS_VERIFY=1)])
write('release_spec',[variant('baseline','main_baseline'),variant('candidate','release')])
write('official_spec',[dict(name='candidate',exe='experiments/v25_v6/release.exe',env={})])
write('core_spec',[variant('control','design_core'),variant('refit','design_core',DESIGN_CORE=1)])
write('core_verify_spec',[variant('verify','design_core',DESIGN_CORE=1,DESIGN_CORE_VERIFY=1)])
write('selection_budget_spec',[variant('baseline','main_baseline'),
    variant('q512','main_baseline',NOV8_QUERIES=512),
    variant('q1024','main_baseline',NOV8_QUERIES=1024),
    variant('early','main_baseline',NOV8_SELECT_END=1.95,NOV8_SELECT_HARD=2.05)])
write('selection_validate_spec',[variant('baseline','main_baseline'),
    variant('q512','main_baseline',NOV8_QUERIES=512),variant('q1024','main_baseline',NOV8_QUERIES=1024)])
write('diverse_spec',[variant('baseline','main_baseline'),variant('diverse','design_diverse',DESIGN_FLIP=.5)])
write('critic_fixed_spec',[variant(name,'route_critic_fixture',**common,NOV5_RELAX_BUDGET=10,ROUTE_CRITIC_ROLLOUT=level)
    for name,level in [('control',0),('critic',1)]])
write('vector_spec',[variant('baseline','main_baseline'),variant('vector','vector')])
write('native_design_spec',[variant('baseline','native_baseline'),
    variant('diverse','native_diverse',DESIGN_FLIP=.5),variant('half','native_half')])
write('headers_spec',[variant('baseline','native_baseline'),variant('headers','headers')])
write('duo_fixed_spec',[variant(name,'route_duo_fixture',**common,NOV5_RELAX_BUDGET=10,NOV5_DUO=enable,NOV5_DUO_CPU=cap,NOV5_DUO_VERIFY=verify)
    for name,enable,cap,verify in [('control',0,.03,0),('duo',1,.03,0),('wide',1,.3,0),('verify',1,.03,1)]])
write('discount_fixed_spec',[variant(name,'route_discount_fixture',**common,NOV5_RELAX_BUDGET=10,**env)
    for name,env in [('control',{}),('short',dict(FUTURE_HORIZON=.5)),('long',dict(FUTURE_HORIZON=1.5)),
        ('equal',dict(FUTURE_EQUAL_ONLY=1)),('blendhalf',dict(FUTURE_BLEND=.5)),('learned',dict(FUTURE_BLEND=1))]])
write('equal_fixed_spec',[variant(name,'route_discount_fixture',**common,NOV5_RELAX_BUDGET=10,FUTURE_EQUAL_ONLY=level)
    for name,level in [('control',0),('equal',1)]])
write('mdp_fixed_spec',[variant(name,'route_discount_fixture',**common,NOV5_RELAX_BUDGET=10,BMDP_CPU=3,**env)
    for name,env in [('control',{}),('h6',dict(BMDP_HORIZON=6)),('h12',dict(BMDP_HORIZON=12)),
       ('r25',dict(BMDP_REPAIR=.25)),('r75',dict(BMDP_REPAIR=.75))]])
write('equal_normal_spec',[variant('baseline','native_baseline'),variant('equal','equal_release')])
write('width_spec',[variant('baseline','native_baseline'),variant('w39','native_baseline',PACK_W=39),variant('w40','native_baseline',PACK_W=40)])
write('wide_child_spec',[variant('baseline','design_wide'),variant('replace','design_wide',CHILDMAP_WIDE=1),variant('append','design_wide',CHILDMAP_WIDE=2)])
write('core_state_fixed_spec',[variant(name,'core_state_fixture',**common,NOV5_RELAX_BUDGET=10,CORE_STATE_WEIGHT=weight,CORE_STATE_VERIFY=verify)
    for name,weight,verify in [('control',0,0),('w1',1,0),('w2',2,0),('verify',1,1)]])
write('core_state_hold_spec',[variant(name,'core_state_fixture',**common,NOV5_RELAX_BUDGET=10,CORE_STATE_WEIGHT=weight)
    for name,weight in [('control',0),('w1',1)]])
write('core_state_normal_spec',[variant('baseline','native_baseline'),variant('w1','core_state',CORE_STATE_WEIGHT=1)])
write('collector_spec',[variant('collect','route_collect_fixture',**common,NOV5_RELAX_BUDGET=60,CRITIC_COLLECT=1)])

from pathlib import Path
import json
here=Path(__file__).resolve().parent
s=(here/'popcnt.cpp').read_text(encoding='utf-8')
needle='    int end_deck_saved=0;'
assert s.count(needle)==1
s=s.replace(needle,'''    const double tail_limit=search_budget::real("TAIL_CPU_BUDGET",-1);
    int tail_calls=0,tail_skipped=0,tail_first_skip=-1;
'''+needle)
needle='        if(!routing_budget::expired(actual.route_deadline))loan_tail::improve(actual,a,trace_start,moves,actual.route_deadline);'
assert s.count(needle)==1
s=s.replace(needle,'''        if(!routing_budget::expired(actual.route_deadline)) {
            if(tail_limit<0 || loan_tail::cpu<search_budget::seconds(tail_limit)) {
                tail_calls++;loan_tail::improve(actual,a,trace_start,moves,actual.route_deadline);
            }else{tail_skipped++;if(tail_first_skip<0)tail_first_skip=q;}
        }''')
needle='    cerr<<"FINAL_DECK saved="<<end_deck_saved<<"\\n";'
assert s.count(needle)==1
s=s.replace(needle,needle+'''\n    cerr<<"TAIL_BUDGET calls="<<tail_calls<<" skipped="<<tail_skipped<<" first_skip="<<tail_first_skip<<" limit="<<tail_limit<<"\\n";''')
(here/'tail_budget.cpp').write_text(s,encoding='utf-8')
variants=[dict(name=name,exe='experiments/renewed_v5/tail_budget_linux',env=dict(TAIL_CPU_BUDGET=cap)) for name,cap in [('unlimited',-1),('tail_off',0),('tail15',.15),('tail35',.35)]]
(here/'tail_spec.json').write_text(json.dumps(variants,indent=2))

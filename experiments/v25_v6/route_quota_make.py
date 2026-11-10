from pathlib import Path
import subprocess
import sys

directory = Path(__file__).resolve().parent
source = (directory / "main_baseline.cpp").read_text(encoding="utf-8")
source = '''// Apply the target before standard headers, including bitset::count.
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC target("popcnt")
#endif
''' + source


def replace_once(old, new):
    global source
    assert source.count(old) == 1, (old[:100], source.count(old))
    source = source.replace(old, new, 1)


replace_once('''    clock_t core_ticks=0, primary_ticks=0, regular_ticks=0;
    for(int q=0;q<R;q++) {''', '''    clock_t core_ticks=0, primary_ticks=0, regular_ticks=0;
    const bool quota_reserve=search_budget::integer("ROUTE_QUOTA_RESERVE",0)!=0;
    const bool alternate_fixpoint=search_budget::integer("ROUTE_ALT_FIXPOINT",0)!=0;
    const double relax_quota=search_budget::seconds(getenv("NOV5_RELAX_BUDGET")?atof(getenv("NOV5_RELAX_BUDGET")):.52);
    const double relax_cpu_initial=nov5_relaxed_trace::stats().cpu;
    double primary_relax_cpu=0;
    long long quota_refusals=0,quota_limited=0,alternate_calls=0,repeat_calls=0,repeat_saved=0;
    for(int q=0;q<R;q++) {''')
replace_once('''        int previous_length;
        do{''', '''        const double primary_relax_started=nov5_relaxed_trace::stats().cpu;
        int previous_length;
        do{''')
replace_once('''        }while(int(moves.size())<previous_length && !routing_budget::expired(deadline));

        // Spend secondary-search''', '''        }while(int(moves.size())<previous_length && !routing_budget::expired(deadline));
        primary_relax_cpu+=nov5_relaxed_trace::stats().cpu-primary_relax_started;

        // Spend secondary-search''')
replace_once('''        auto compare_route=[&](const OnlineRouter::RouteState& challenger,vector<pair<int,int>>& trace,clock_t limit){
            const auto first_state=actual.save_route_state();''', '''        auto compare_route=[&](const OnlineRouter::RouteState& challenger,vector<pair<int,int>>& trace,clock_t limit){
            // Absolute CPU reservations do not reserve optimize()'s separate
            // cumulative quota. Keep the observed PRIMARY shortening cost for
            // the remaining queries; only alternatives consume the surplus.
            if(quota_reserve && q>=15 && q+1<R) {
                const double pending=primary_relax_cpu*double(R-1-q)/(q+1);
                const double available=relax_quota-nov5_relaxed_trace::stats().cpu-pending;
                if(available<=0){quota_refusals++;return;}
                const clock_t now=clock();
                const clock_t quota_end=now+clock_t(available*CLOCKS_PER_SEC);
                if(quota_end<limit){limit=quota_end;quota_limited++;}
                if(now>=limit){quota_refusals++;return;}
            }
            alternate_calls++;
            const auto first_state=actual.save_route_state();''')
replace_once('''            actual.restore_route_state(challenger);
            auto alt=nov5_relaxed_trace::optimize(actual,a,trace_start,move(trace),limit);
            const auto other=actual.save_route_state();''', '''            actual.restore_route_state(challenger);
            int previous_alt=int(trace.size());
            auto alt=nov5_relaxed_trace::optimize(actual,a,trace_start,move(trace),limit);
            // Match the incumbent's fixed-point rule: optimize() recurses
            // internally after >=2 saved moves, but not after a single gain.
            if(alternate_fixpoint)while(int(alt.size())<previous_alt && !routing_budget::expired(limit)) {
                previous_alt=int(alt.size());repeat_calls++;
                alt=nov5_relaxed_trace::optimize(actual,a,trace_start,move(alt),limit);
                repeat_saved+=previous_alt-int(alt.size());
            }
            const auto other=actual.save_route_state();''')
anchor = '    cerr<<"FINAL_DECK saved="<<end_deck_saved<<"\\n";'
replace_once(anchor, r'''    cerr<<"ROUTE_QUOTA reserve="<<quota_reserve<<" fixedpoint="<<alternate_fixpoint
        <<" primary_cpu="<<primary_relax_cpu
        <<" secondary_cpu="<<nov5_relaxed_trace::stats().cpu-relax_cpu_initial-primary_relax_cpu
        <<" budget="<<relax_quota<<" refusals="<<quota_refusals<<" limited="<<quota_limited
        <<" alternatives="<<alternate_calls<<" repeat_calls="<<repeat_calls<<" repeat_saved="<<repeat_saved<<"\n";
''' + anchor)
target = directory / "route_quota.cpp"
target.write_text(source, encoding="utf-8")
fixture = directory / "route_quota_fixture.cpp"
subprocess.run([sys.executable, str(directory.parent / "beam_v1/make_fixture.py"), str(target), str(fixture)], check=True)
fixture_source = fixture.read_text(encoding="utf-8")
assert fixture_source.count('#include "fixture_io.inc"') == 1
fixture.write_text(fixture_source.replace('#include "fixture_io.inc"', '#include "../beam_v1/fixture_io.inc"'), encoding="utf-8")

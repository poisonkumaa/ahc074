from pathlib import Path
import hashlib
import subprocess
import sys

directory = Path(__file__).resolve().parent
baseline = directory / "main_baseline.cpp"
assert hashlib.sha256(baseline.read_bytes()).hexdigest() == "a9e5cb6b45ee1562c60721be09bde2d0246ad8eeb95585c7721048d74143d7da"
source = baseline.read_text(encoding="utf-8")


def replace_once(old, new):
    global source
    assert source.count(old) == 1, (old[:100], source.count(old))
    source = source.replace(old, new, 1)


replace_once("    clock_t core_ticks=0, primary_ticks=0, regular_ticks=0;", """    clock_t core_ticks=0, primary_ticks=0, regular_ticks=0;
    const double future_blend=clamp(search_budget::real("FUTURE_BLEND",.75),0.0,1.0);
    const double future_horizon=clamp(search_budget::real("FUTURE_HORIZON",1.0),0.0,4.0);
    const bool future_equal_only=search_budget::integer("FUTURE_EQUAL_ONLY",0)!=0;
    long long future_comparisons=0,future_overrides=0;""")
replace_once("            bool choose=alt.size()+alt_value<first_score-1e-9;", """            bool choose=alt.size()+alt_value<first_score-1e-9;
            const bool raw_choice=choose;""")
replace_once("            if(abs(lengths)<=1 && abs(alt.size()+alt_value-first_score)<1.15 && other.order!=first_state.order){", """            if((!future_equal_only || lengths==0) && abs(lengths)<=1 && abs(alt.size()+alt_value-first_score)<1.15 && other.order!=first_state.order){
                ++future_comparisons;""")
replace_once("                const double forecast=lengths+.75*avg+.25*prior;", """                // Keep the original expression and floating-point grouping in
                // the default control. Other settings scale only future cost.
                const double forecast=future_blend==.75 && future_horizon==1.0
                    ? lengths+.75*avg+.25*prior
                    : lengths+future_horizon*(future_blend*avg+(1.0-future_blend)*prior);""")
replace_once("                if(forecast>.05)choose=false;", """                if(forecast>.05)choose=false;
                future_overrides+=choose!=raw_choice;""")
anchor = '    cerr<<"FINAL_DECK saved="<<end_deck_saved<<"\\n";'
replace_once(anchor, r'''    cerr<<"ROUTE_DISCOUNT blend="<<future_blend<<" horizon="<<future_horizon
        <<" equal_only="<<future_equal_only<<" comparisons="<<future_comparisons
        <<" overrides="<<future_overrides<<"\n";
''' + anchor)
source = '''// Apply the target before standard headers, including bitset::count.
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC target("popcnt")
#endif
''' + source
target = directory / "route_discount.cpp"
target.write_text(source, encoding="utf-8")
fixture = directory / "route_discount_fixture.cpp"
subprocess.run([sys.executable, str(directory.parent / "beam_v1/make_fixture.py"), str(target), str(fixture)], check=True)
fixture_source = fixture.read_text(encoding="utf-8")
assert fixture_source.count('#include "fixture_io.inc"') == 1
fixture.write_text(fixture_source.replace('#include "fixture_io.inc"', '#include "../beam_v1/fixture_io.inc"'), encoding="utf-8")
for path in (target, fixture):
    print(path.name, hashlib.sha256(path.read_bytes()).hexdigest())

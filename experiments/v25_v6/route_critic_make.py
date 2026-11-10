from pathlib import Path
import hashlib
import subprocess
import sys
import textwrap

directory = Path(__file__).resolve().parent
baseline = directory / "main_baseline.cpp"
assert hashlib.sha256(baseline.read_bytes()).hexdigest() == "a9e5cb6b45ee1562c60721be09bde2d0246ad8eeb95585c7721048d74143d7da"
source = baseline.read_text(encoding="utf-8")

# Reuse the exact existing feature arithmetic for the one-step leaf critic.
begin = source.index("                auto proxy=[&](const OnlineRouter::RouteState& st){", source.index("        auto compare_route="))
end = source.index("                double avg=0;", begin)
feature = textwrap.dedent(source[begin:end])
feature = feature.replace("auto proxy1=proxy(first_state),proxy2=proxy(other);", "auto predicted=proxy(st);")
feature = feature.replace("auto fa=features(first_state,proxy1,first_score-moves.size()),fb=features(other,proxy2,alt_value);", "auto values=features(st,predicted,routeval);")
helper = "namespace route_critic {\nstatic double terminal_value(OnlineRouter& actual,const OnlineRouter::RouteState& st,int a,double routeval){\n"
helper += textwrap.indent(feature, "    ")
helper += "    double learned=0;for(int i=0;i<int(values.size());i++)learned+=weight[i]*values[i];\n    return .75*learned+.25*routeval;\n}\n"
helper += (directory / "route_critic_runtime.inc").read_text(encoding="utf-8")
helper += "\n}\n\n"
assert source.count("int main() {") == 1
source = source.replace("int main() {", helper + "int main() {", 1)

def replace_once(old, new):
    global source
    assert source.count(old) == 1, (old[:100], source.count(old))
    source = source.replace(old, new, 1)

replace_once("    clock_t core_ticks=0, primary_ticks=0, regular_ticks=0;", "    clock_t core_ticks=0, primary_ticks=0, regular_ticks=0;\n    route_critic::Search critic_search;")
replace_once("            bool choose=alt.size()+alt_value<first_score-1e-9;", "            bool choose=alt.size()+alt_value<first_score-1e-9;\n            const bool raw_choose=choose;")
replace_once("                if(forecast>.05)choose=false;", """                if(forecast>.05)choose=false;
                if(critic_search.enabled && lengths==0 && choose!=raw_choose && abs(forecast)<=.35)
                    choose=critic_search.compare(actual,a,q,first_state,other,min(limit,actual.route_deadline),choose);""")
anchor = '    cerr<<"FINAL_DECK saved="<<end_deck_saved<<"\\n";'
replace_once(anchor, "    critic_search.report();\n" + anchor)
source = '''// Apply the target before standard headers, including bitset::count.
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC target("popcnt")
#endif
''' + source
target = directory / "route_critic.cpp"
target.write_text(source, encoding="utf-8")
fixture = directory / "route_critic_fixture.cpp"
subprocess.run([sys.executable, str(directory.parent / "beam_v1/make_fixture.py"), str(target), str(fixture)], check=True)
fixture_source = fixture.read_text(encoding="utf-8")
assert fixture_source.count('#include "fixture_io.inc"') == 1
fixture.write_text(fixture_source.replace('#include "fixture_io.inc"', '#include "../beam_v1/fixture_io.inc"'), encoding="utf-8")
for path in (target, fixture):
    print(path.name, hashlib.sha256(path.read_bytes()).hexdigest())

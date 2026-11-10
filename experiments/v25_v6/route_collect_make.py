from pathlib import Path
import hashlib
import subprocess
import sys

directory = Path(__file__).resolve().parent
baseline = directory / "main_baseline.cpp"
assert hashlib.sha256(baseline.read_bytes()).hexdigest() == "a9e5cb6b45ee1562c60721be09bde2d0246ad8eeb95585c7721048d74143d7da"
target = directory / "route_collect_fixture.cpp"
subprocess.run([sys.executable, str(directory.parent / "beam_v1/make_fixture.py"), str(baseline), str(target)], check=True)
source = target.read_text(encoding="utf-8")

def once(old, new):
    global source
    assert source.count(old) == 1, (old[:100], source.count(old))
    source = source.replace(old, new, 1)

once('#include "fixture_io.inc"', '#include "../beam_v1/fixture_io.inc"')
helper = (directory / "route_collect_runtime.inc").read_text(encoding="utf-8")
once('int main() {', helper + '\nint main() {')
once('    clock_t core_ticks=0, primary_ticks=0, regular_ticks=0;',
     '    clock_t core_ticks=0, primary_ticks=0, regular_ticks=0;\n    route_collect::Collector critic_collector;')
once('                auto fa=features(first_state,proxy1,first_score-moves.size()),fb=features(other,proxy2,alt_value);',
     '''                auto fa=features(first_state,proxy1,first_score-moves.size()),fb=features(other,proxy2,alt_value);
                if(critic_collector.enabled)critic_collector.capture(actual,a,q,lengths,first_state,other,fa,fb);''')
once('    cerr<<"LOAN_TAIL wins="', '    critic_collector.flush(actual);\n    cerr<<"LOAN_TAIL wins="')
source = '''// Research fixture only; labels never use the actual future requests.
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC target("popcnt")
#endif
''' + source
target.write_text(source, encoding="utf-8")
print(target.name, hashlib.sha256(target.read_bytes()).hexdigest())

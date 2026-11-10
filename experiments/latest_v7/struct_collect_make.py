"""Build a fixture-only critic collector from the frozen latest solver."""
from pathlib import Path
import hashlib
import subprocess
import sys

directory=Path(__file__).resolve().parent
baseline=directory/'main_baseline.cpp'
assert hashlib.sha256(baseline.read_bytes()).hexdigest()=='407a5355d6a75bbe07bd332a265fcb694b532dc7f35de0dee465006bf136a7e7'
target=directory/'struct_collect_fixture.cpp'
subprocess.run([sys.executable,str(directory.parent/'beam_v1/make_fixture.py'),str(baseline),str(target)],check=True)
source=target.read_text(encoding='utf-8')

def once(old,new):
    global source
    assert source.count(old)==1,(old[:100],source.count(old))
    source=source.replace(old,new,1)

once('#include "fixture_io.inc"','#include "../beam_v1/fixture_io.inc"')
helper=(directory/'struct_collect_runtime.inc').read_text(encoding='utf-8')
once('int main() {',helper+'\nint main() {')
once('    clock_t core_ticks=0, primary_ticks=0, regular_ticks=0;',
     '    clock_t core_ticks=0, primary_ticks=0, regular_ticks=0;\n    route_collect::Collector critic_collector;')
once('''                auto fb=features(other,alt_value);
''','''                auto fb=features(other,alt_value);
                if(critic_collector.enabled)critic_collector.capture(actual,a,q,lengths,
                    first_state,other,fa,fb,trace_start,moves,alt);
''')
once('    cerr<<"U48 eligible="','    critic_collector.flush(actual);\n    cerr<<"U48 eligible="')
source='// Research fixture only: labels use catalog-only independent future requests.\n'+source
target.write_text(source,encoding='utf-8')
print(target.name,hashlib.sha256(target.read_bytes()).hexdigest())

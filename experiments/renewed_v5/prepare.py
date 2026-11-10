from pathlib import Path
import hashlib
import json
import subprocess
import sys

here=Path(__file__).resolve().parent
root=here.parents[1]
baseline=here/'main_baseline.cpp'
if not baseline.exists():baseline.write_bytes((root/'main.cpp').read_bytes())
assert hashlib.sha256(baseline.read_bytes()).hexdigest()=='2a36bd6c8d4944aa61068d4ac85fb7773dd1f1ff59970bc84cf37512cd3278e7'
source=baseline.read_text(encoding='utf-8')
needle='// Exact accelerations: DAG metric memoization'
assert source.count(needle)==1
source=source.replace(needle,'''// Use hardware population count for the 64-window masks on x86 judges.
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC target("popcnt")
#endif
'''+needle)
(here/'popcnt.cpp').write_text(source,encoding='utf-8')
for name in ['main_baseline','popcnt']:
    subprocess.run([sys.executable,str(root/'experiments/beam_v1/make_fixture.py'),str(here/(name+'.cpp')),str(here/(name+'_fixture.cpp'))],check=True)
variants=[dict(name='baseline',exe='experiments/renewed_v5/main_baseline_linux',env={}),dict(name='popcnt',exe='experiments/renewed_v5/popcnt_linux',env={})]
(here/'normal_spec.json').write_text(json.dumps(variants,indent=2))

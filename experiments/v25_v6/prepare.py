from pathlib import Path
import hashlib,json,difflib,subprocess,sys
here=Path(__file__).resolve().parent
root=here.parents[1]
baseline=here/'main_baseline.cpp'
if not baseline.exists():baseline.write_bytes((root/'main.cpp').read_bytes())
assert hashlib.sha256(baseline.read_bytes()).hexdigest()=='a9e5cb6b45ee1562c60721be09bde2d0246ad8eeb95585c7721048d74143d7da'
s=baseline.read_text(encoding='utf-8')
prefix='''// Enable population count before all standard-library definitions.
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC target("popcnt")
#endif
'''
(here/'popcnt.cpp').write_text(prefix+s,encoding='utf-8')
old=(root/'experiments/renewed_v5/main_baseline.cpp').read_text(encoding='utf-8')
(here/'user_changes.patch').write_text(''.join(difflib.unified_diff(old.splitlines(True),s.splitlines(True),fromfile='previous.cpp',tofile='v25.cpp')),encoding='utf-8')
spec=[dict(name=name,exe=f'experiments/v25_v6/{file}_linux',env={}) for name,file in [('baseline','main_baseline'),('popcnt','popcnt')]]
(here/'normal_spec.json').write_text(json.dumps(spec,indent=2),encoding='utf-8')
for name in ['main_baseline','popcnt']:
    subprocess.run([sys.executable,str(root/'experiments/beam_v1/make_fixture.py'),str(here/f'{name}.cpp'),str(here/f'{name}_fixture.cpp')],check=True)

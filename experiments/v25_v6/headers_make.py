from pathlib import Path
import subprocess,sys
here=Path(__file__).resolve().parent
s=(here/'main_baseline.cpp').read_text(encoding='utf-8')
s='''// Apply the optimizer before instantiating standard-library templates.
#if defined(__GNUC__)
#pragma GCC optimize("O3")
#endif
'''+s
(here/'headers.cpp').write_text(s,encoding='utf-8')
subprocess.run([sys.executable,str(here.parent/'beam_v1/make_fixture.py'),str(here/'headers.cpp'),str(here/'headers_fixture.cpp')],check=True)

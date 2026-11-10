from pathlib import Path
import hashlib

HERE=Path(__file__).resolve().parent
baseline=HERE/'main_baseline.cpp'
assert hashlib.sha256(baseline.read_bytes()).hexdigest()=='a9e5cb6b45ee1562c60721be09bde2d0246ad8eeb95585c7721048d74143d7da'
s=baseline.read_text(encoding='utf-8')
s='''#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC target("popcnt")
#endif
'''+s
marker='int main() {'
assert s.count(marker)==1
s=s.replace(marker,'#include "design_core.inc"\n\n'+marker,1)
marker='    cerr<<"CHOSEN W="<<chosen.W'
assert s.count(marker)==1
s=s.replace(marker,'''    design_core::optimize(chosen,absolute_deadline(search_budget::real("DESIGN_CORE_END",2.43)));
'''+marker,1)
(HERE/'design_core.cpp').write_text(s,encoding='utf-8',newline='\n')
print('Wrote design_core.cpp')

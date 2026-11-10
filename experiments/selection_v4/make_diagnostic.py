from pathlib import Path
import hashlib
import json

here = Path(__file__).resolve().parent
root = here.parents[1]
frozen = here / 'main_before_v4.cpp'
if not frozen.exists():
    frozen.write_bytes((root / 'main.cpp').read_bytes())
assert hashlib.sha256(frozen.read_bytes()).hexdigest() == '40f66c0166537ad258bd20bb4d95b4b7dfb451fddb36c3eb7fff0db67e4bac55'
s = frozen.read_text(encoding='utf-8')
s = s.replace('int main() {', '#include "../beam_v1/fixture_io.inc"\n#include <filesystem>\nint main() {', 1)
s = s.replace('    if(!feasible[selected])return fallback();', '''    if(!feasible[selected])return fallback();
    for(int i=0;i<int(candidates.size());++i)
        cerr<<"SELECTION_DIAG index="<<i<<" W="<<candidates[i].W<<" B="<<candidates[i].tot_B<<" live="<<feasible[i]<<" score="<<scores[i]<<" selected="<<(i==selected)<<" queries="<<synthetic_queries<<"\\n";
    const int diagnostic_selected=selected;
    const string diagnostic_root=getenv("DIAGNOSTIC_ROOT")?getenv("DIAGNOSTIC_ROOT"):"experiments/selection_v4/fixtures";
    for(int diagnostic_index=0;diagnostic_index<int(candidates.size());++diagnostic_index){
    selected=diagnostic_index;
''')
needle='    cerr<<"CHOSEN W="<<chosen.W'
end=s.index(needle)
s=s[:end]+'''    string fixture_dir=diagnostic_root+"/"+to_string(diagnostic_index);
    std::filesystem::create_directories(fixture_dir);
#if defined(_WIN32)
    _putenv_s("BEAM_FIXTURE_DIR",fixture_dir.c_str());
#else
    setenv("BEAM_FIXTURE_DIR",fixture_dir.c_str(),1);
#endif
    save_fixture(chosen,heads,A);
    }
    cerr<<"DIAGNOSTIC_COMPLETE selected="<<diagnostic_selected<<" candidates="<<candidates.size()<<"\\n";
    return 0;
}
'''
(here/'selection_diagnostic.cpp').write_text(s,encoding='utf-8')
base='/mnt/c/Users/dokuk/Documents/Competitive_programing/Atcoder/AHC/ahc074'
common=dict(NOV8_ROUTE_END=30,NOV8_RELAX_END=30,NOV8_SHORT_END=30,NOV5_RELAX_BUDGET=10)
spec=[dict(name=f'index{i}',exe='experiments/beam_v1/fixture_baseline_linux',env=common|dict(BEAM_FIXTURE_DIR=f'{base}/experiments/selection_v4/fixtures/{i}')) for i in range(8)]
(here/'diagnostic_spec.json').write_text(json.dumps(spec,indent=2),encoding='utf-8')

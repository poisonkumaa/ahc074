from pathlib import Path
import hashlib

d = Path(__file__).resolve().parent
root = d.parents[1]
s = (d / 'multi_round_submit.cpp').read_text()
s = s.replace('static double multi_training_budget=1.0;', '''#ifndef AHC_LEARNING_BUDGET
#define AHC_LEARNING_BUDGET 1.0
#endif
#ifndef AHC_SELECTION_BUDGET
#define AHC_SELECTION_BUDGET 1.4
#endif
static double multi_training_budget=AHC_LEARNING_BUDGET;''')
s = s.replace('const double deadline=1.4;multi_training_budget=1.0;',
              'const double deadline=AHC_SELECTION_BUDGET;multi_training_budget=AHC_LEARNING_BUDGET;')
s = s.replace(' vector<unique_ptr<ModelSimulation>>sim;',
              ' int winner=0,states=0;\n if(models.size()>1){\n vector<unique_ptr<ModelSimulation>>sim;')
s = s.replace(' int states=0;bool expired=false;', ' bool expired=false;')
s = s.replace(' int winner=0;for(int i=0;i<int(sim.size());i++)',
              ' for(int i=0;i<int(sim.size());i++)')
s = s.replace(' cerr<<"Round selected="', ' }\n cerr<<"Round selected="')
s = s.replace('''// Among shortest-path moves minimize the number of mandatory removals averaged
// over 64 previous goals. A suffix union counts correlated choices exactly.''',
'''// Take the first visible paper admitting a shortest-path move. Among its
// insertion positions, minimize mandatory removals over 64 previous goals.
// A suffix union counts correlated choices exactly.''')
s = s.replace('vector<int> direct_final(', '''// Internal public-data simulation only: construct a goal permutation directly.
// Actual responses use the legal, distance-decreasing moves above.
vector<int> direct_final(''')
assert 'desired_models=2' in s and 'int main()' in s
assert s.count('if(models.size()>1)') == 1
(d / 'multi_final.cpp').write_text(s)
old = root / 'main.cpp'
assert hashlib.sha256(old.read_bytes()).hexdigest() == '53bf60f2f9397217ef30b26a25ccf464f5a0bc66292b8e643b3b5aadfff0051b', 'main.cpp changed; inspect first'
for filename in ['validation.json', 'holdout/results.json', 'final_holdout/results.json']:
    src = d / filename
    dst = src.with_name('baseline_13906_' + src.name)
    if src.exists() and not dst.exists():
        dst.write_bytes(src.read_bytes())
old.write_text(s)
print('Installed main.cpp SHA256:', hashlib.sha256(old.read_bytes()).hexdigest())

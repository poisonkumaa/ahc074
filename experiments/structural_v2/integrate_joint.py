from pathlib import Path
from difflib import SequenceMatcher

root = Path(__file__).resolve().parent
old = (root.parent / 'improve_eval/main_baseline.cpp').read_text(encoding='utf-8').splitlines(True)
generic = (root.parent / 'improve_eval/ternary_router.cpp').read_text(encoding='utf-8').splitlines(True)
s = (root / 'baseline.cpp').read_text(encoding='utf-8')
for tag, i, j, a, b in SequenceMatcher(None, old, generic, autojunk=False).get_opcodes():
    if tag == 'equal':
        continue
    before, after = ''.join(old[i:j]), ''.join(generic[a:b])
    if not before:
        before = ''.join(old[i-2:i+2])
        after = ''.join(old[i-2:i]) + after + ''.join(old[i:i+2])
    assert s.count(before) == 1, (tag, i, j, s.count(before))
    s = s.replace(before, after, 1)
includes = '#include "pair_repair.inc"\n#include "joint_core_search.inc"\n'
s = s.replace('int main() {', includes + '\nint main() {', 1)
oldhook = '    vector<HeadSample> heads;\n    if(!trials.empty()){'
hook = '''    if(search_budget::integer("CORE_JOINT",0)) {
        optimize_joint_core(chosen,absolute_deadline(search_budget::real("CORE_JOINT_END",2.30)));
        if(!trials.empty())trials[selected]=make_unique<OnlineRouter>(chosen,chosen.internal_catalog,absolute_deadline(search_budget::real("NOV8_INIT_END",2.47)));
    }
    vector<HeadSample> heads;
    if(!trials.empty()){'''
assert oldhook in s
s = s.replace(oldhook, hook, 1)
(root / 'joint.cpp').write_text(s, encoding='utf-8')

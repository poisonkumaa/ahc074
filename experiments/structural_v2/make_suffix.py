from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'baseline.cpp').read_text(encoding='utf-8')
s=s.replace('int main() {','#include "suffix_holes.inc"\n\nint main() {',1)
needle='    cerr<<"CHOSEN W="<<chosen.W'
assert needle in s
s=s.replace(needle,'    suffix_hole_optimizer::optimize(chosen,absolute_deadline(search_budget::real("SUFFIX_HOLE_END",2.40)));\n'+needle,1)
(p/'suffix.cpp').write_text(s,encoding='utf-8')

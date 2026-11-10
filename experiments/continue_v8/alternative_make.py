"""Isolate v68's new searches and head-value model without changing coefficients.

alternative_v68.cpp is a byte-for-byte frozen copy of the supplied Downloads
source. The pair/head/both experiments retain the current main's 18 weights.
All extracted v68 functions are preserved verbatim in these first ablations.
"""
from pathlib import Path
import hashlib

here = Path(__file__).resolve().parent
baseline = here / 'main_baseline.cpp'
original = here / 'alternative_v68.cpp'
assert hashlib.sha256(baseline.read_bytes()).hexdigest() == 'd6810b13fa94eb0bdfb28135d8551340d97648df9b170b49f84d1d668ec3d049'
assert hashlib.sha256(original.read_bytes()).hexdigest() == '0626aef727512ec46f2201e15674c3fac6c6ac2f9b7765a04b3227d5c93368e1'
base = baseline.read_text(encoding='utf-8')
v68 = original.read_text(encoding='utf-8-sig')
shift_begin = v68.index('namespace shift68 {')
head_begin = v68.index('namespace headvalue64 {')
main_begin = v68.index('int main() {')
pair_functions = v68[shift_begin:head_begin]
head_functions = v68[head_begin:main_begin]
head_call_begin = v68.index('                if(use_pred64){')
head_call_end = v68.index('                if(forecast<-.05)choose=true;', head_call_begin)
head_call = v68[head_call_begin:head_call_end]
head_setup = '    headvalue64::Value value64(chosen);bool use_pred64=!getenv("NO_HEADVALUE64");int actions64=0,changed64=0;\n'
old_cards = '        auto old68=actual.active_supports;if(actual.active_main>=0)old68.push_back(actual.active_main);\n'
pair_call = '        pair68::improve(actual,a,trace_start,moves,old68,tail_deadline);\n'
pair_log = ''.join(line + '\n' for line in v68.splitlines() if 'cerr<<"PAIR68 ' in line or 'cerr<<"SHIFT68 ' in line)
head_log = ''.join(line + '\n' for line in v68.splitlines() if 'cerr<<"HEADV64 ' in line)
assert head_setup in v68 and old_cards in v68 and pair_call in v68
assert pair_log.count('cerr') == 2 and head_log.count('cerr') == 1

for name, use_pair, use_head in [('pair', True, False), ('head', False, True), ('both', True, True)]:
    s = base
    def once(old, new):
        global s
        assert s.count(old) == 1, (name, old[:80], s.count(old))
        s = s.replace(old, new, 1)
    functions = (pair_functions if use_pair else '') + (head_functions if use_head else '')
    once('int main() {', functions + 'int main() {')
    if use_pair:
        anchor = '        auto trace_start=actual.sim.order;uint64_t previous_heads=feedback.background(actual);\n'
        once(anchor, old_cards + anchor)
        anchor = '        shelf58::apply(actual,trace_start,moves,a);\n'
        once(anchor, anchor + pair_call)
        anchor = '    cerr<<"SH58 '
        once(anchor, pair_log + anchor)
    if use_head:
        anchor = '    actual.route_branching=true;\n'
        once(anchor, anchor + head_setup)
        anchor = '                if(forecast<-.05)choose=true;'
        once(anchor, head_call + anchor)
        anchor = '    cerr<<"LOAN_TAIL wins="'
        once(anchor, head_log + anchor)
    target = here / ('alternative_' + name + '.cpp')
    target.write_text(s, encoding='utf-8')
    print(target.name, hashlib.sha256(target.read_bytes()).hexdigest())

# Apart from harmless whitespace around inserted namespace definitions, the
# combined candidate must equal v68 with exactly the baseline critic weights.
import re
pattern = r'static constexpr array<double,18> weight=\{[^}]+\};'
baseline_weights = re.search(pattern, base).group(0)
expected = re.sub(pattern, lambda _: baseline_weights, v68)
actual = (here / 'alternative_both.cpp').read_text(encoding='utf-8')
assert ''.join(actual.split()) == ''.join(expected.split())

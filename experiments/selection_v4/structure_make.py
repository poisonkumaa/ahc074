from pathlib import Path
import argparse

HERE = Path(__file__).resolve().parent
parser = argparse.ArgumentParser()
parser.add_argument('--source', type=Path, default=HERE / 'main_before_v4.cpp')
parser.add_argument('--target', type=Path, default=HERE / 'structure_candidate.cpp')
args = parser.parse_args()
source = args.source.read_text(encoding='utf-8')
anchor = '        double alpha=1.0,beta=.5;\n'
assert source.count(anchor) == 1
source = source.replace(anchor, '''        double alpha=search_budget::real("STRUCTURE_ANCHOR_ALPHA",1.0),
            beta=search_budget::real("STRUCTURE_ANCHOR_BETA",.5);
''')
anchor = '            const double route_scale=1.0;\n'
assert source.count(anchor) == 1
source = source.replace(anchor, '            const double route_scale=search_budget::real("STRUCTURE_JOINT_GAMMA",1.0);\n')
anchor = 'int main() {'
assert source.count(anchor) == 1
source = source.replace(anchor, (HERE / 'structure_assign.inc').read_text(encoding='utf-8') + '\n' + anchor)
anchor = '    for(const auto &s:chosen.S_init) {\n'
assert source.count(anchor) == 1
source = source.replace(anchor, '    structure_assign::optimize(chosen,heads,absolute_deadline(search_budget::real("STRUCTURE_END",2.45)));\n' + anchor)
args.target.write_text(source, encoding='utf-8', newline='\n')
print(args.target)

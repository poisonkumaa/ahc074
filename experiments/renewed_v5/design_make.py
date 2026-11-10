from pathlib import Path
import hashlib

HERE=Path(__file__).resolve().parent
baseline=HERE/'main_baseline.cpp'
assert hashlib.sha256(baseline.read_bytes()).hexdigest()=='2a36bd6c8d4944aa61068d4ac85fb7773dd1f1ff59970bc84cf37512cd3278e7'
s=baseline.read_text(encoding='utf-8')
marker='namespace fixed_deck {\n'
assert s.count(marker)==1
s=s.replace(marker,'#include "design_matching.inc"\n\n'+marker,1)
old='            snap.pairs=merge_matching(first,second,snap.route);'
assert s.count(old)==1
s=s.replace(old,'''            snap.pairs=design_matching::offspring(first,second,snap.route,
                absolute_deadline(search_budget::real("DESIGN_MATCHING_END",layout_deadline-.02)));''',1)
(HERE/'design_candidate.cpp').write_text(s,encoding='utf-8',newline='\n')
print('Wrote design_candidate.cpp')

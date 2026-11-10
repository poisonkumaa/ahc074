from pathlib import Path
import hashlib

here=Path(__file__).resolve().parent
s=(here/'design_candidate.cpp').read_text(encoding='utf-8')
assert s.count('#include "design_flip.inc"')==1
s=s.replace('#include "design_flip.inc"',(here/'design_flip.inc').read_text(encoding='utf-8'))
assert s.count('search_budget::real("DESIGN_FLIP",0.)')==1
s=s.replace('search_budget::real("DESIGN_FLIP",0.)','search_budget::real("DESIGN_FLIP",.5)')
s=s.replace('// AHC074 v25. Shelf-color pricing, batched comparison maps and budgeted route alternatives.',
    '// AHC074 v26. Blend exact nonlinear exposure demand into support-aware anchor search.\n'
    '// Based on v25 shelf-color pricing, batched comparison maps and budgeted routes.')
(here/'release.cpp').write_text(s,encoding='utf-8',newline='\n')
print(hashlib.sha256(s.encode()).hexdigest())

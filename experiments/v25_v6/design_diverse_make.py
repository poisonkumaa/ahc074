from pathlib import Path
import hashlib

HERE=Path(__file__).resolve().parent
source=HERE/'design_candidate.cpp'
assert hashlib.sha256(source.read_bytes()).hexdigest()=='d1700d6d841341d528f7926f866c9288445774bfbf48c56826b46b4160705246'
s=source.read_text(encoding='utf-8')
old='            const double exact_flip_mix=std::clamp(search_budget::real("DESIGN_FLIP",0.),0.,1.);'
assert s.count(old)==1
s=s.replace(old,'''            // Keep the original root layouts; diversify only later replay/child layouts.
            const bool nonlinear_family=graph_seed_offset>=std::max(1,search_budget::integer("NOV8_POOL",4));
            const double exact_flip_mix=nonlinear_family?
                std::clamp(search_budget::real("DESIGN_FLIP",0.),0.,1.):0.;''',1)
(HERE/'design_diverse.cpp').write_text(s,encoding='utf-8',newline='\n')
print('Wrote design_diverse.cpp')

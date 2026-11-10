from pathlib import Path
here=Path(__file__).resolve().parent
s=(here/'main_baseline.cpp').read_text(encoding='utf-8')
old='if(abs(lengths)<=1 && abs(alt.size()+alt_value-first_score)<1.15 && other.order!=first_state.order)'
assert s.count(old)==1
s=s.replace(old,'if(lengths==0 && abs(alt.size()+alt_value-first_score)<1.15 && other.order!=first_state.order)')
(here/'equal_release.cpp').write_text(s,encoding='utf-8')

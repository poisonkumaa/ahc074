from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'round7_maskdp.cpp').read_text()
s=s.replace(' vector<int>solve_prepared(', ' __attribute__((target_clones("popcnt","default"))) vector<int>solve_prepared(')
(d/'round7_maskpop.cpp').write_text(s)

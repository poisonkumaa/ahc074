from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'main_13906_final.cpp').read_text()
s=s.replace('inline std::pair<int,int> best_move(', '__attribute__((target_clones("popcnt","default")))\ninline std::pair<int,int> best_move(')
(d/'popcnt_submit.cpp').write_text(s)

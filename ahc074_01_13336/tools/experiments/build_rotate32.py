from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'main_13906_final.cpp').read_text()
s=s.replace('cols=bestcols;for(int j=1;', 'cols=bestcols;rotate(cols.begin(),cols.begin()+32,cols.end());for(int j=1;')
(d/'rotate32_submit.cpp').write_text(s)

from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'refit4_reward.cpp').read_text()
s=s.replace('for(U64&v:A){','for(int &f:flip)f^=1;\n for(U64&v:A){')
(d/'refit4_complement.cpp').write_text(s)

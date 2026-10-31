from pathlib import Path
p=Path(__file__).parent
s=(p.parent.parent/'main.cpp').read_text()
s=s.replace('int main(){','#include "round7_lengthlocal.h"\nint main(int argc,char**argv){int passes=argc>1?atoi(argv[1]):5,samples=argc>2?atoi(argv[2]):64,repetitions=argc>3?atoi(argv[3]):8;')
s=s.replace('exact_state_refit(A,papers,1,43,32,-1,0);','exact_state_refit(A,papers,1,43,32,repetitions,0);\n Round7LengthLocal::search(A,papers,passes,samples);')
(p/'round7_lengthlocal.cpp').write_text(s)

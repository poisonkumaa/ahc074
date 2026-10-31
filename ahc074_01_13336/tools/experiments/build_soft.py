from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'main_13906_final.cpp').read_text()
s=s.replace('int main(){','#include "soft_dictionary.h"\nint main(int argc,char**argv){')
s=s.replace('refit_all_prefix_papers(A,papers,4,32);','refit_all_prefix_papers(A,papers,4,32);\n soft_refit(A,papers,argc>2?atoi(argv[2]):1,argc>1?atof(argv[1]):3,argc>3?atoi(argv[3]):1);')
(d/'soft_submit.cpp').write_text(s)

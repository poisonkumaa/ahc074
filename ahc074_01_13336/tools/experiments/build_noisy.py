from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'main_13906_final.cpp').read_text().replace('int main(){','#include "noisy_dictionary.h"\nint main(int argc,char**argv){')
s=s.replace('refit_all_prefix_papers(A,papers,4,32);','refit_all_prefix_papers(A,papers,4,32);\n noisy_refit(A,papers,1,argc>1?atoi(argv[1]):3,argc>2?atoi(argv[2]):8,argc>3?atoi(argv[3]):16);')
(d/'noisy_submit.cpp').write_text(s)

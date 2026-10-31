from pathlib import Path
root=Path(__file__).resolve().parents[2]
text=(root/'main.cpp').read_text()
text=text.replace('int main(){','#include "round7_source_swap.h"\nint main(int argc,char**argv){',1)
text=text.replace('exact_state_refit(A,papers,1,43,32,-1,0);','exact_state_refit(A,papers,1,43,32,argc>4?atoi(argv[4]):32,0);\n source_product_swap(A,papers,argc>1?atoi(argv[1]):1,argc>2?atoi(argv[2]):500,argc>3?atoi(argv[3]):256,argc>5?atoi(argv[5]):0);',1)
(root/'tools/experiments/round7_source_swap.cpp').write_text(text)

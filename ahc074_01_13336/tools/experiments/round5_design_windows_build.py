from pathlib import Path
root=Path(__file__).resolve().parents[2]
text=(root/'tools/experiments/round5_design.cpp').read_text()
text=text.replace('int main(int argc,char**argv){','#include "round5_design_windows.h"\nint main(int argc,char**argv){',1)
text=text.replace('exact_state_refit(A,papers,1,43,32,round5_repetitions,0);',
 'exact_state_refit(A,papers,1,43,32,round5_repetitions,0);\n optimize_adjacent_window(A,papers,cols,flip,argc>7?atoi(argv[7]):3,argc>8?atoi(argv[8]):64);')
(root/'tools/experiments/round5_design_windows.cpp').write_text(text)

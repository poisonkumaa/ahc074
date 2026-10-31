from pathlib import Path
root=Path(__file__).resolve().parents[2]
text=(root/'main.cpp').read_text()
text=text.replace('int main(){', '#include "round6_joint_exact.h"\nint main(int argc,char**argv){',1)
text=text.replace('exact_state_refit(A,papers,1,43,32,-1,0);','exact_state_refit(A,papers,1,43,32,8,0);\n joint_adjacent_exact(A,papers,argc>1?atoi(argv[1]):256,argc>2?atoi(argv[2]):1,argc>3?atoi(argv[3]):43,argc>4?atoi(argv[4]):1);',1)
(root/'tools/experiments/round6_joint_exact.cpp').write_text(text)

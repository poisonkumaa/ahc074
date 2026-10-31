from pathlib import Path
root=Path(__file__).resolve().parents[2]
text=(root/'main.cpp').read_text()
text=text.replace('int main(){', '#include "round6_joint_intrinsic.h"\nint main(int argc,char**argv){',1)
text=text.replace('exact_state_refit(A,papers,1,43,32,-1,0);','exact_state_refit(A,papers,1,43,32,8,0);\n joint_adjacent_intrinsic(A,papers,argc>1?atoi(argv[1]):1,argc>2?atoi(argv[2]):64);',1)
(root/'tools/experiments/round6_joint_intrinsic.cpp').write_text(text)

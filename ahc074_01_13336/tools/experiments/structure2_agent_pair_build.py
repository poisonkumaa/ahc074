from pathlib import Path
root=Path(__file__).resolve().parents[2]
text=(root/'main.cpp').read_text()
marker='int main(){'
text=text.replace(marker,'#include "structure2_agent_pair.h"\nint main(int argc,char**argv){\n const int pair_alpha=argc>1?atoi(argv[1]):2;')
text=text.replace('int samples=64;', 'PairGoalDP pair_dp(n,papers);\n int samples=64;')
text=text.replace('int moves;auto chosen=dp.solve(order,A[product],moves);','int moves;auto chosen=pair_alpha?pair_dp.solve(order,A[product],moves,future,pair_alpha):dp.solve(order,A[product],moves);')
(root/'tools/experiments/structure2_agent_pair.cpp').write_text(text)

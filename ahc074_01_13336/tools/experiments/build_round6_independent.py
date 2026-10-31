from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'round6_initial_fast.cpp').read_text()
s=s.replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100').replace('#define AHC_REFINEMENT_BUDGET 1.65','#define AHC_REFINEMENT_BUDGET 100')
old='int t=r<128?rng()%P:targets[(r-128)%P];'
assert old in s
s=s.replace('  for(int r=0;r<128+M;r++){','  mt19937 simulation_rng(725713);int simulation_previous=-1;\n  for(int r=0;r<128+M;r++){',1)
s=s.replace(old,'int t=simulation_rng()%P;while(t==simulation_previous)t=simulation_rng()%P;simulation_previous=t;',1)
s=s.replace('s.target=t;for(int i=0;i<K;i++)s.pos[order[i]]=i;','s.target=targets[(r-128)%P];for(int i=0;i<K;i++)s.pos[order[i]]=i;',1)
s=s.replace('int main(){','int main(int argc,char**argv){')
s=s.replace('exact_state_refit(A,papers,1,43,32,-1,0);','exact_state_refit(A,papers,1,43,32,argc>1?atoi(argv[1]):8,0);')
(d/'round6_independent.cpp').write_text(s)

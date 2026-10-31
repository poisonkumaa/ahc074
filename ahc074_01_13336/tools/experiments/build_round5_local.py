from pathlib import Path
d=Path(__file__).resolve().parent;root=d.parents[1]
s=(root/'main.cpp').read_text()
s=s.replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100')
s=s.replace('#define AHC_REFINEMENT_BUDGET 1.45','#define AHC_REFINEMENT_BUDGET 100')
s=s.replace('using namespace std;using U64=uint64_t;', 'using namespace std;using U64=uint64_t;int local_cycles=2,local_reverse=0;')
a=s.index('  for(int q=0;q<Q;q++){\n   int z=level[l][q];')
brace=s.index('{',a);depth=1;b=brace+1
while depth:
 if s[b]=='{':depth+=1
 if s[b]=='}':depth-=1
 b+=1
block=s[a:b].replace('for(int q=0;q<Q;q++){','for(int qi=0;qi<Q;qi++){int q=local_reverse&&(cycle&1)?Q-1-qi:qi;')
s=s[:a]+' for(int cycle=0;cycle<local_cycles;cycle++){int before_updates=accepted;\n'+block+'\n if(accepted==before_updates)break;\n }'+s[b:]
s=s.replace('int main(){','int main(int argc,char**argv){')
s=s.replace(' exact_state_refit(A,papers,1,43,32,8,0);',' local_cycles=argc>1?atoi(argv[1]):2;int repetitions=argc>2?atoi(argv[2]):8;local_reverse=argc>3?atoi(argv[3]):0;\n exact_state_refit(A,papers,1,43,32,repetitions,0);')
(d/'round5_local.cpp').write_text(s)

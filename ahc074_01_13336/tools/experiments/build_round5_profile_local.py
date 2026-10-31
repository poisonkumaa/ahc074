from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'round5_sampling_profile_sparse.cpp').read_text()
a=s.index('  for(int q=0;q<Q;q++){\n   int z=level[l][q];')
brace=s.index('{',a);depth=1;b=brace+1
while depth:
 if s[b]=='{':depth+=1
 if s[b]=='}':depth-=1
 b+=1
block=s[a:b]
s=s[:a]+' for(int local_cycle=0;local_cycle<2;local_cycle++){int before_updates=accepted;\n'+block+'\n if(accepted==before_updates)break;\n }'+s[b:]
(d/'round5_profile_local.cpp').write_text(s)

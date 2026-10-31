from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'round6_triefast_final.cpp').read_text()
assert s.count('(4.0*max(per_repetition,1e-6))')==1
s=s.replace('(4.0*max(per_repetition,1e-6))','(5.0*max(per_repetition,1e-6))')
start=s.index('inline void exact_state_refit(')
prefix,refit=s[:start],s[start:]
needle='   for(int e=0;e<M;e++){\n    auto&s=scenarios[e];'
assert refit.count(needle)==1
refit=refit.replace(needle,'   for(int e=0;e<M;e++){\n    if((e&63)==0&&expired())return;\n    auto&s=scenarios[e];')
needle='  for(int q=0;q<Q;q++){\n   int z=level[l][q];'
assert refit.count(needle)==1
refit=refit.replace(needle,'  for(int q=0;q<Q;q++){\n   if(expired())return;\n   int z=level[l][q];')
s=prefix+refit
(d/'round6_calibrated.cpp').write_text(s)

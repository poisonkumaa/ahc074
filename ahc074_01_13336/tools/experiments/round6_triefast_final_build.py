from pathlib import Path
p=Path(__file__).parent
s=(p/'round6_initial_triefast.cpp').read_text()
exact=(p/'round6_exact_triefast.cpp').read_text()
start=s.index('inline void exact_state_refit(');end=s.index('inline bool direct_moves(',start)
es=exact.index('inline void exact_state_refit(');ee=exact.index('inline bool direct_moves(',es)
s=s[:start]+exact[es:ee]+s[end:]
header=(p/'round6_trie_topology.h').read_text().replace('#pragma once\n','',1)
assert s.count('#include "round6_trie_topology.h"')==1
s=s.replace('#include "round6_trie_topology.h"',header)
assert '#include "' not in s and 'int main(){' in s
(p/'round6_triefast_final.cpp').write_text(s)
diag=s.replace('int main(){','int main(int argc,char**argv){').replace('exact_state_refit(A,papers,1,43,32,-1,0);','cerr<<"InitialCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\\n";exact_state_refit(A,papers,1,43,32,argc>1?atoi(argv[1]):8,0);')
(p/'round6_triefast_diag.cpp').write_text(diag)
comparison=(p/'round6_initial_compare.py').read_text().replace('round6_initial_fast_diag','round6_triefast_diag').replace('round6_initial_fast_check','round6_triefast_check').replace('round6_initial_compare.json','round6_triefast_compare.json')
comparison=comparison.replace('dict(name=name,prep=', 'dict(name=name,initial=float(re.findall(r\'InitialCPU=([\\d.]+)\',log)[-1]),prep=')
(p/'round6_triefast_compare.py').write_text(comparison)

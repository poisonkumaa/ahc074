from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'templates_agent_trim.cpp').read_text().replace('#include "global_agent_weighted.h"','#include "prefix_allocate_train.h"')
s=s.replace('train_prefix_dictionary(A,a,n,cap,1,8);','train_prefix_dictionary(A,a,n,64,1,8,0,6,0);')
s=s.replace('trie_patterns(A,l,cap,mult)','trie_patterns(A,l,16,mult)')
s=s.replace('dp.portmask|=1ULL<<j;','if(argc>14)j=atoi(argv[14])+portscheme*q;dp.portmask|=1ULL<<j;')
(d/'prefix_ports_run.cpp').write_text(s)

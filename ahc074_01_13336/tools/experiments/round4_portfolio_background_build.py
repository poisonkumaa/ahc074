from pathlib import Path
p=Path(__file__).parent
s=(p/'round4_exact_trie.cpp').read_text()
config='''
#ifndef R4_BASE_MODE
#define R4_BASE_MODE 0
#endif
#ifndef R4_INITIAL_PASSES
#define R4_INITIAL_PASSES 4
#endif
'''
s=s.replace('inline void train_backbone_dictionary(',config+'\ninline void train_backbone_dictionary(',1)
old='papers.clear();for(int l=1;l<=n;l++)papers.push_back({l,PrefixDP::low(l-1)});papers.push_back({n,PrefixDP::low(n)});'
new='U64 background=R4_BASE_MODE?0:~0ULL;papers.clear();for(int l=1;l<=n;l++)papers.push_back({l,(background&PrefixDP::low(l))^(1ULL<<(l-1))});papers.push_back({n,background&PrefixDP::low(n)});'
assert old in s
s=s.replace(old,new).replace('refit_all_prefix_papers(A,papers,4,32);','refit_all_prefix_papers(A,papers,R4_INITIAL_PASSES,32);')
(p/'round4_portfolio_background.cpp').write_text(s)

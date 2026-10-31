from pathlib import Path
p=Path(__file__).resolve().parent
run=(p/'prefix_run.cpp').read_text()
trie=run[run.index('vector<U64> trie_patterns'):run.index('int fixed_dist')]
src='#pragma GCC optimize("O3")\n'+(p/'prefix_dp.h').read_text().replace('#pragma once','')+(p/'global_agent_prefix.h').read_text().replace('#pragma once','')+trie+(p/'prefix_policy_agent_search_main.txt').read_text()
(p/'prefix_policy_agent_search.cpp').write_text(src)

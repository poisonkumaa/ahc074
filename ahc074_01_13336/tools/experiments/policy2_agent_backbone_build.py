from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'structure2_agent_backbone.cpp').read_text()
for header in ['global_agent_allocate_tune.h','backbone_dictionary.h','structure2_agent_refit.h','structure2_agent_exchange.h']:
    s=s.replace(f'#include "{header}"',(p/header).read_text().replace('#pragma once',''))
s=s.replace('train_backbone_dictionary(A,papers,n,argc>4?atoi(argv[4]):8,argc>1?atoi(argv[1]):3,argc>2?atoi(argv[2]):8);','train_backbone_dictionary(A,papers,n,6,1,8);')
s=s.replace('refit_all_prefix_papers(A,papers,argc>7?atoi(argv[7]):3,argc>8?atoi(argv[8]):8);','refit_all_prefix_papers(A,papers,1,8);')
s=s.replace('if(argc>9&&atoi(argv[9]))exchange_prefix_dictionary(A,papers,atoi(argv[9]),16,3,4);','')
(p/'policy2_agent_backbone_base.cpp').write_text(s)
w=(p/'policy2_agent_weighted.cpp').read_text()
b=s.index('namespace GlobalPrefix');e=s.index('vector<U64> trie_patterns',b)
wb=w.index('namespace GlobalPrefix');we=w.index('vector<U64> trie_patterns',wb)
s=s[:b]+w[wb:we]+s[e:]
b=s.index(' for(int k=0;k<K;k++){\n  string pattern');e=s.index(' for(int r=0;r<R;r++){',b)
design=s[b:e];design=design[:design.index(' PrefixDP dp(n,papers);')]
wb=w.index(' PrefixDP dp(n,papers);vector<int>order');we=w.index(' for(int k=0;k<K;k++){\n  string pattern',wb)
s=s[:b]+w[wb:we]+design+s[e:]
s=s.replace('GlobalPrefix::best_move(order,length,selected);if(from<0)return 0;', 'GlobalPrefix::best_move(order,length,selected,.5,argc>3&&atoi(argv[3])?nullptr:&chance);if(from<0)return 0;')
(p/'policy2_agent_backbone_weighted.cpp').write_text(s)
# Identical learned-value experiment against the new family.
s=(p/'policy2_agent_backbone_base.cpp').read_text();td=(p/'policy2_agent_td.cpp').read_text()
b=s.index('struct PrefixDP');e=s.index('#include <algorithm>',b);tb=td.index('struct PrefixDP');te=td.index('#include <algorithm>',tb)
s=s[:b]+td[tb:te]+s[e:]
b=s.index(' for(int k=0;k<K;k++){\n  string pattern');e=s.index(' for(int r=0;r<R;r++){',b);block=s[b:e];d=block.index(' PrefixDP dp(n,papers);')
s=s[:b]+block[d:]+(p/'policy2_agent_td_train.txt').read_text()+block[:d]+s[e:]
(p/'policy2_agent_backbone_td.cpp').write_text(s)

from pathlib import Path
p=Path(__file__).parent
s=(p/'round4_portfolio_fast.cpp').read_text()
a=s.index('inline void train_backbone_dictionary(');b=s.index('#ifndef ENTROPY_WIDTH',a)
f=s[a:b].replace('std::array<int,17>dp,split','std::array<int,9>dp,split')
f=f.replace('int child[2]{-1,-1},weight=0,sample=0;','int child[2]{-1,-1},weight=0,sample=0,available=1;')
f=f.replace('for(int q=1;q<=amount;q++)for(int left=0;left<=q;left++){int right=q-left;', 'int ca=a<0?0:trie[a].available,cb=b<0?0:trie[b].available;v.available=std::min(amount,ca+cb);for(int q=1;q<=v.available;q++)for(int left=std::max(0,q-cb);left<=std::min(q,ca);left++){int right=q-left;',1)
f=f.replace('for(int q=1;q<=amount;q++)for(int left=0;left<=q;left++){int right=q-left;', 'int ca=a<0?0:trie[a].available,cb=b<0?0:trie[b].available;p.available=std::min(amount,ca+cb);for(int q=1;q<=p.available;q++)for(int left=std::max(0,q-cb);left<=std::min(q,ca);left++){int right=q-left;',1)
s=s[:a]+f+s[b:]
(p/'round4_portfolio_trie_fast.cpp').write_text(s)

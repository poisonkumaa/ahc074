from pathlib import Path
p=Path(__file__).parent
s=(p/'round4_exact_iter.cpp').read_text()
a=s.index('inline void train_backbone_dictionary(');b=s.index('vector<int> direct_final(',a)
f=s[a:b]
assert f.count('int child[2]{-1,-1},weight=0,sample=0;')==2
f=f.replace('int child[2]{-1,-1},weight=0,sample=0;','int child[2]{-1,-1},weight=0,sample=0,available=1;')
old='for(int q=1;q<=amount;q++)for(int left=0;left<=q;left++){int right=q-left;'
assert f.count(old)==2
f=f.replace(old,'int ca=a<0?0:trie[a].available,cb=b<0?0:trie[b].available;v.available=std::min(amount,ca+cb);for(int q=1;q<=v.available;q++)for(int left=std::max(0,q-cb);left<=std::min(q,ca);left++){int right=q-left;',1)
f=f.replace(old,'int ca=a<0?0:trie[a].available,cb=b<0?0:trie[b].available;p.available=std::min(amount,ca+cb);for(int q=1;q<=p.available;q++)for(int left=std::max(0,q-cb);left<=std::min(q,ca);left++){int right=q-left;',1)
(p/'round4_exact_trie.cpp').write_text(s[:a]+f+s[b:])

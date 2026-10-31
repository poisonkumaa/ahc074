from pathlib import Path
p=Path(__file__).parent
s=(p/'round6_initial_fast_diag.cpp').read_text()
for name,next_marker,tag in [('train_backbone_dictionary','inline void refit_all_prefix_papers','backbone'),('refit_all_prefix_papers','// Construct the final permutation','refit')]:
 start=s.index('inline void '+name+'(');end=s.index(next_marker,start);f=s[start:end]
 f=f.replace(' struct Node{',' double phase[4]{};clock_t phase_start=clock();auto mark=[&](int q){auto t=clock();phase[q]+=double(t-phase_start)/CLOCKS_PER_SEC;phase_start=t;};\n struct Node{',1)
 f=f.replace('check(l);std::vector<Node>trie(1);','check(l);phase_start=clock();std::vector<Node>trie(1);')
 if tag=='backbone':
  f=f.replace('  for(int node=int(trie.size())-1;', '  mark(0);for(int node=int(trie.size())-1;')
  f=f.replace('  int use=amount;', '  mark(1);int use=amount;')
 else:
  f=f.replace('  for(int x=int(trie.size())-1;', '  mark(0);for(int x=int(trie.size())-1;')
  f=f.replace('  if(trie[0].dp[amount]<0)', '  mark(1);if(trie[0].dp[amount]<0)')
 f=f.replace('  advance(l);\n', '  mark(2);advance(l);mark(3);\n')
 f=f.replace(' std::cerr<<"'+tag+' "', ' std::cerr<<"InitialPhases '+tag+' build="<<phase[0]<<" dp="<<phase[1]<<" commit="<<phase[2]<<" advance="<<phase[3]<<"\\n";\n std::cerr<<"'+tag+' "')
 s=s[:start]+f+s[end:]
(p/'round6_initial_profile.cpp').write_text(s)

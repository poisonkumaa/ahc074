from pathlib import Path
p=Path(__file__).parent
s=(p/'round6_initial_fast.cpp').read_text()
s=s.replace('inline void train_backbone_dictionary(', '''#ifdef INITIAL_TRIE_CHECK
#define ROUND6_TRIE_CHECK
#endif
#include "round6_trie_topology.h"
inline void train_backbone_dictionary(''',1)
construct='''const auto&topology=round6_trie_topology(A,l);std::vector<Node>trie(topology.nodes.size());
  for(int x=0;x<int(trie.size());x++){trie[x].child[0]=topology.nodes[x].child[0];trie[x].child[1]=topology.nodes[x].child[1];trie[x].sample=topology.nodes[x].sample;}
#ifdef INITIAL_TRIE_CHECK
  std::vector<Node>rawtrie(topology.raw_nodes.size());for(int x=0;x<int(rawtrie.size());x++){rawtrie[x].child[0]=topology.raw_nodes[x].child[0];rawtrie[x].child[1]=topology.raw_nodes[x].child[1];rawtrie[x].sample=topology.raw_nodes[x].sample;}
#endif
'''
check='''
  auto leaf_sequence=[&](const std::vector<Node>&tree,int count){std::vector<int>out;std::function<void(int,int)>walk=[&](int x,int q){if(!q)return;const auto&node=tree[x];if(node.child[0]<0&&node.child[1]<0){out.push_back(node.sample);return;}int left=node.split[q];if(left)walk(node.child[0],left);if(q-left)walk(node.child[1],q-left);};walk(0,count);return out;};
  for(int q=0;q<=amount;q++){assert(trie[0].dp[q]==rawtrie[0].dp[q]);if(q<=trie[0].available)assert(leaf_sequence(trie,q)==leaf_sequence(rawtrie,q));}
#endif
'''
for name,next_marker,backbone in [('train_backbone_dictionary','inline void refit_all_prefix_papers',True),('refit_all_prefix_papers','// Construct the final permutation',False)]:
 start=s.index('inline void '+name+'(');end=s.index(next_marker,start);f=s[start:end]
 assert 'std::vector<Node>trie(1);' in f
 f=f.replace('std::vector<Node>trie(1);',construct)
 if backbone:
  old='int b=A[v]>>(l-d)&1;if(trie[node].child[b]<0){int next=trie.size();trie[node].child[b]=next;trie.emplace_back();}node=trie[node].child[b];trie[node].sample=v;'
  assert old in f;f=f.replace(old,'node=topology.path[v][d];')
  old='trie[node].weight+=importance*(benefit-prev)+1;prev=benefit;'
  assert old in f;f=f.replace(old,'''int delta=importance*(benefit-prev)+1;trie[node].weight+=delta;
#ifdef INITIAL_TRIE_CHECK
    rawtrie[topology.raw_path[v][d]].weight+=delta;
#endif
    prev=benefit;''')
  ds=f.index('  for(int node=int(trie.size())-1;');de=f.index('  int use=amount;',ds)
 else:
  old='int bit=A[v]>>(l-d)&1;if(trie[x].child[bit]<0){int ni=trie.size();trie[x].child[bit]=ni;trie.emplace_back();}x=trie[x].child[bit];trie[x].sample=v;'
  assert old in f;f=f.replace(old,'x=topology.path[v][d];')
  old='trie[x].weight+=benefit-previous+1;previous=benefit;'
  assert old in f;f=f.replace(old,'''int delta=benefit-previous+1;trie[x].weight+=delta;
#ifdef INITIAL_TRIE_CHECK
    rawtrie[topology.raw_path[v][d]].weight+=delta;
#endif
    previous=benefit;''')
  ds=f.index('  for(int x=int(trie.size())-1;');de=f.index('  if(trie[0].dp[amount]<0)',ds)
 code=f[ds:de]
 f=f[:de]+'#ifdef INITIAL_TRIE_CHECK\n'+code.replace('trie','rawtrie')+check+f[de:]
 s=s[:start]+f+s[end:]
(p/'round6_initial_triefast.cpp').write_text(s)
diag=s.replace('int main(){','int main(int argc,char**argv){').replace('exact_state_refit(A,papers,1,43,32,-1,0);','cerr<<"InitialCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\\n";exact_state_refit(A,papers,1,43,32,argc>1?atoi(argv[1]):8,0);')
(p/'round6_initial_triefast_diag.cpp').write_text(diag)

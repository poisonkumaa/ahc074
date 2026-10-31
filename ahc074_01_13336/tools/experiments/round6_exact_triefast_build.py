from pathlib import Path
import argparse
root=Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser();p.add_argument('--source',default='main.cpp');a=p.parse_args()
source=(root/a.source).read_text()
source=source.replace('int main(){','int main(int argc,char**argv){',1)
source=source.replace('exact_state_refit(A,papers,1,43,32,-1,0);','exact_state_refit(A,papers,1,43,32,argc>1?atoi(argv[1]):8,0);',1)
(root/'tools/experiments/round6_exact_trie_reference.cpp').write_text(source)
old=r'''  struct Node{int child[2]{-1,-1},sample=0;long long weight=0;};vector<Node>trie(1);vector<array<int,65>>path(P);
  for(int v=0;v<P;v++){int x=0;for(int d=1;d<=l;d++){int b=A[v]>>(l-d)&1;if(trie[x].child[b]<0){int ni=trie.size();trie[x].child[b]=ni;trie.emplace_back();}x=trie[x].child[b];path[v][d]=x;trie[x].sample=v;}}'''
new=r'''  struct Node{int child[2]{-1,-1},sample=0;long long weight=0;};
  const auto&topology=round6_trie_topology(A,l);const auto&path=topology.path;vector<Node>trie(topology.nodes.size());
  for(int x=0;x<int(trie.size());x++){trie[x].child[0]=topology.nodes[x].child[0];trie[x].child[1]=topology.nodes[x].child[1];trie[x].sample=topology.nodes[x].sample;}
#ifdef EXACT_TRIE_CHECK
  vector<Node>reference_trie(1);vector<array<int,65>>reference_path(P);
  for(int v=0;v<P;v++){int x=0;for(int d=1;d<=l;d++){int b=A[v]>>(l-d)&1;if(reference_trie[x].child[b]<0){int ni=reference_trie.size();reference_trie[x].child[b]=ni;reference_trie.emplace_back();}x=reference_trie[x].child[b];reference_path[v][d]=x;reference_trie[x].sample=v;}}
#endif'''
assert source.count(old)==1
source=source.replace(old,new,1)
source=source.replace('inline void exact_state_refit(', '#include "round6_trie_topology.h"\ninline void exact_state_refit(',1)
old='int z=level[l][q];for(auto&node:trie)node.weight=0;long long oldcost=0;'
new=old+'\n#ifdef EXACT_TRIE_CHECK\n   for(auto&node:reference_trie)node.weight=0;\n#endif'
assert source.count(old)==1;source=source.replace(old,new,1)
old='trie[path[v][d]].weight+=gain-previous;previous=gain;'
new='''trie[path[v][d]].weight+=gain-previous;
#ifdef EXACT_TRIE_CHECK
    reference_trie[reference_path[v][d]].weight+=gain-previous;
#endif
    previous=gain;'''
assert source.count(old)==1;source=source.replace(old,new,1)
old="   U64 candidate=A[sample]&PrefixDP::low(l);long long newcost=0;bool valid=true;"
new=r'''#ifdef EXACT_TRIE_CHECK
   long long reference_best=LLONG_MIN;int reference_sample=-1;
   for(int x=0;x<int(reference_trie.size());x++){auto&node=reference_trie[x];bool leaf=true;for(int bit=0;bit<2;bit++)if(node.child[bit]>=0){reference_trie[node.child[bit]].weight+=node.weight;leaf=false;}if(leaf&&node.weight>reference_best)reference_best=node.weight,reference_sample=node.sample;}
   assert(best==reference_best&&sample==reference_sample);
   for(int v=0;v<P;v++)for(int d=1;d<=l;d++)if(d==l||path[v][d]!=path[v][d+1])assert(trie[path[v][d]].weight==reference_trie[reference_path[v][d]].weight);
#endif
'''+old
assert source.count(old)==1;source=source.replace(old,new,1)
(root/'tools/experiments/round6_exact_triefast.cpp').write_text(source)

#pragma once
#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

// Unary paths contribute their weight whenever at least one descendant leaf
// is selected. Mapping those nodes to the first retained descendant preserves
// every leaf/path weight and every k-leaf DP value. Retained nodes keep their
// original insertion order, which preserves the original leaf tie order.
struct Round6TrieTopology {
 struct Node {int child[2]{-1,-1};int sample=0;};
 std::vector<Node>nodes;
 std::vector<std::array<uint16_t,65>>path;
#ifdef ROUND6_TRIE_CHECK
 std::vector<Node>raw_nodes;
 std::vector<std::array<uint16_t,65>>raw_path;
#endif
};
inline const Round6TrieTopology& round6_trie_topology(const std::vector<uint64_t>&A,int length){
 struct Cache {std::vector<uint64_t>data;std::array<std::unique_ptr<Round6TrieTopology>,65>topology;};
 static Cache cache;
 if(cache.data!=A){cache.data=A;for(auto&entry:cache.topology)entry.reset();}
 assert(length>=1&&length<=64);
 if(cache.topology[length])return *cache.topology[length];
 auto result=std::make_unique<Round6TrieTopology>();
 std::vector<Round6TrieTopology::Node>raw(1);
 std::vector<std::array<uint16_t,65>>raw_path(A.size());
 for(int v=0;v<int(A.size());v++){
  int x=0;raw_path[v][0]=0;
  for(int d=1;d<=length;d++){
   int bit=(A[v]>>(length-d))&1;
   if(raw[x].child[bit]<0){int next=raw.size();raw[x].child[bit]=next;raw.emplace_back();}
   x=raw[x].child[bit];raw[x].sample=v;assert(x<65536);raw_path[v][d]=x;
  }
 }
 std::vector<int>map(raw.size(),-1);
 for(int x=0;x<int(raw.size());x++){
  bool left=raw[x].child[0]>=0,right=raw[x].child[1]>=0;
  if(left==right){map[x]=result->nodes.size();result->nodes.emplace_back();}
 }
 for(int x=int(raw.size())-1;x>=0;x--)if(map[x]<0){int child=raw[x].child[0]>=0?raw[x].child[0]:raw[x].child[1];map[x]=map[child];}
 assert(map[0]==0&&result->nodes.size()<=2*A.size()-1);
 for(int x=0;x<int(raw.size());x++){
  bool left=raw[x].child[0]>=0,right=raw[x].child[1]>=0;if(left!=right)continue;
  auto&node=result->nodes[map[x]];node.sample=raw[x].sample;
  for(int bit=0;bit<2;bit++)if(raw[x].child[bit]>=0){node.child[bit]=map[raw[x].child[bit]];assert(node.child[bit]>map[x]);}
 }
 result->path.resize(A.size());
 for(int v=0;v<int(A.size());v++)for(int d=0;d<=length;d++)result->path[v][d]=map[raw_path[v][d]];
#ifdef ROUND6_TRIE_CHECK
 result->raw_nodes=std::move(raw);result->raw_path=std::move(raw_path);
#endif
 cache.topology[length]=std::move(result);return *cache.topology[length];
}

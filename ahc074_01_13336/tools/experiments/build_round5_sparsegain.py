from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'round5_local.cpp').read_text()
s=s.replace('vector<array<int,65>>route(M*Q);','vector<array<int,65>>route(M*Q);vector<int>change_index(M*Q+1);vector<uint16_t>changes;')
s=s.replace('auto&rr=route[e*Q+q];rr.fill(INF);','auto&rr=route[e*Q+q];rr.fill(INF);change_index[e*Q+q]=changes.size();')
s=s.replace('rr[j]=value;}', 'assert(value<=K||value>=INF);if(value<rr[j+1]&&value<INF)changes.push_back(((l-j)<<8)|value);rr[j]=value;}')
s=s.replace('  struct Node{int child[2]{-1,-1},sample=0;long long weight=0;};vector<Node>trie(1);','  change_index[M*Q]=changes.size();\n  struct Node{int child[2]{-1,-1},sample=0;long long weight=0;};vector<Node>trie(1);')
s=s.replace('for(int d=1;d<=l;d++){int value=route[e*Q+q][l-d];long long gain=', 'for(int ix=change_index[e*Q+q];ix<change_index[e*Q+q+1];ix++){int d=changes[ix]>>8,value=changes[ix]&255;long long gain=')
assert 'change_index[M*Q]' in s
(d/'round5_sparsegain.cpp').write_text(s)

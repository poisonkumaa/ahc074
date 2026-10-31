from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'round5_sampling_adaptive.cpp').read_text()
s=s.replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100').replace('#define AHC_REFINEMENT_BUDGET 1.65','#define AHC_REFINEMENT_BUDGET 100')
s=s.replace('inline void exact_state_refit(', 'static int joint_pairs=2,joint_stride=1;\ninline void exact_state_refit(',1)
marker='  for(int e=0;e<M;e++){int v=scenarios[e].target;auto&t=*tables[e];for(int q=0;q<Q;q++){int z=level[l][q],best=INF;'
assert marker in s
s=s.replace(marker,r'''
  if(Q>=2)for(int pair=0;pair<min(joint_pairs,Q/2);pair++){
   int p0=(2*pair+l)%Q,p1=(p0+1)%Q,z0=level[l][p0],z1=level[l][p1];vector<int>base0(M),base(M);long long old=0,best_total=LLONG_MAX;int best_a=-1,best_b=-1;
   for(int e=0;e<M;e++){int v=bypass[e];for(int q=0;q<Q;q++)if(q!=p0&&q!=p1)v=min(v,through[e*Q+q]);base0[e]=v;old+=min(v,min(through[e*Q+p0],through[e*Q+p1]));}
   for(int a=0;a<P;a+=joint_stride){
    for(auto&node:trie)node.weight=0;long long baseline=0;
    for(int e=0;e<M;e++){int v=scenarios[e].target;U64 diff=(A[v]^A[a])&PrefixDP::low(l);int st=diff?64-__builtin_clzll(diff):0;int b=min(base0[e],int(route[e*Q+p0][st]));base[e]=b;long long cost=b>=INF?1000000000LL:b;baseline+=cost;long long previous=0;
     for(int ix=change_index[e*Q+p1];ix<change_index[e*Q+p1+1];ix++){int depth=changes[ix]>>8,value=changes[ix]&255;long long gain=value>=INF?0:max(0LL,cost-value);trie[path[v][depth]].weight+=gain-previous;previous=gain;}
    }
    long long gain=LLONG_MIN;int sample=-1;for(int x=0;x<int(trie.size());x++){auto&node=trie[x];bool leaf=true;for(int bit=0;bit<2;bit++)if(node.child[bit]>=0){trie[node.child[bit]].weight+=node.weight;leaf=false;}if(leaf&&node.weight>gain){gain=node.weight;sample=node.sample;}}
    long long value=baseline-gain;if(value<best_total){best_total=value;best_a=a;best_b=sample;}
   }
   if(best_total<old){
    long long check=0;for(int e=0;e<M;e++){int v=scenarios[e].target;U64 d0=(A[v]^A[best_a])&PrefixDP::low(l),d1=(A[v]^A[best_b])&PrefixDP::low(l);int s0=d0?64-__builtin_clzll(d0):0,s1=d1?64-__builtin_clzll(d1):0;int value=min(base0[e],min(int(route[e*Q+p0][s0]),int(route[e*Q+p1][s1])));assert(value<INF);check+=value;}assert(check==best_total);
    papers[z0].color=A[best_a]&PrefixDP::low(l);papers[z1].color=A[best_b]&PrefixDP::low(l);update(z0);update(z1);total_gain+=old-best_total;accepted+=2;
    for(int e=0;e<M;e++){int v=scenarios[e].target;through[e*Q+p0]=route[e*Q+p0][start[v][z0]];through[e*Q+p1]=route[e*Q+p1][start[v][z1]];}
   }
  }
'''+marker)
s=s.replace('int main(){','int main(int argc,char**argv){joint_pairs=argc>2?atoi(argv[2]):2;joint_stride=argc>3?atoi(argv[3]):1;')
s=s.replace('exact_state_refit(A,papers,1,43,32,-1,0);','exact_state_refit(A,papers,1,43,32,argc>1?atoi(argv[1]):8,0);')
(d/'round6_joint_same.cpp').write_text(s)

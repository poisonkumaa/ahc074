from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'round4_exact_refit.h').read_text().replace('INF=100000','INF=30000')
a=s.index(' for(int pass=0;pass<passes;pass++)')
b=s.index('  struct Node{',a)
replacement=r'''
 struct Table{array<array<uint16_t,193>,65>F,B;array<vector<uint8_t>,65>active;};
 vector<unique_ptr<Table>>tables;for(int e=0;e<M;e++)tables.push_back(make_unique<Table>());
 for(int pass=0;pass<passes;pass++){
  // Higher lengths are updated first. Prefix distances below the current
  // length remain unchanged, and suffix distances are finalized once each.
  for(int e=0;e<M;e++){
   auto&s=scenarios[e];int v=s.target;auto&t=*tables[e];for(auto&r:t.F)r.fill(INF);for(auto&r:t.active)r.clear();t.B[N].fill(0);
   t.F[0][0]=0;t.active[0].push_back(0);vector<int>cand[64];for(int z=0;z<K;z++)for(int j=start[v][z];j<length[z];j++)cand[j].push_back(z);
   for(int j=0;j<N;j++)for(int z:cand[j]){int len=length[z],q=s.pos[z];for(int h:t.active[j]){int cost=regularizer;if(q>=h){int moved=s.cnt[len][q]-s.cnt[len][h],upper=s.cnt[j+1][q]-s.cnt[j+1][h];cost+=unit*(moved+(upper>moved));}int nh=max(q+1,h),val=t.F[j][h]+cost;if(t.F[len][nh]==INF)t.active[len].push_back(nh);if(val<t.F[len][nh])t.F[len][nh]=val;}}
  }
  for(int l=N;l>=bottom;l--){
   int Q=level[l].size();
   vector<int>bypass(M,INF),through(M*Q);vector<array<int,65>>route(M*Q);
   for(int e=0;e<M;e++){
    auto&s=scenarios[e];int v=s.target;auto&t=*tables[e];auto&F=t.F;auto&B=t.B;
    auto add=[&](int j,int h,int z){int q=s.pos[z],len=length[z];if(q<h)return regularizer;int moved=s.cnt[len][q]-s.cnt[len][h],upper=s.cnt[j+1][q]-s.cnt[j+1][h];return unit*(moved+(upper>moved))+regularizer;};
    if(l<N){B[l].fill(INF);for(int z=0;z<K;z++)if(length[z]>l&&start[v][z]<=l){int len=length[z],q=s.pos[z];for(int h=0;h<=K;h++){int val=add(l,h,z)+B[len][max(q+1,h)];if(val<B[l][h])B[l][h]=val;}}}
    for(int z=0;z<K;z++)if(length[z]>l&&start[v][z]<l){int len=length[z],q=s.pos[z];for(int j=start[v][z];j<l;j++)for(int h:t.active[j])bypass[e]=min(bypass[e],F[j][h]+add(j,h,z)+B[len][max(q+1,h)]);}
    for(int q=0;q<Q;q++){int z=level[l][q],pos=s.pos[z];auto&rr=route[e*Q+q];rr.fill(INF);
     for(int j=l-1;j>=0;j--){int value=rr[j+1];for(int h:t.active[j])value=min(value,F[j][h]+add(j,h,z)+B[l][max(pos+1,h)]);rr[j]=value;}
     through[e*Q+q]=rr[start[v][z]];
    }
   }
'''
s=s[:a]+replacement+s[b:]
s=s.replace(' cerr<<"exact_refit accepted="',' }\n cerr<<"exact_refit accepted="')
(d/'round4_exact_fast.h').write_text(s)
source=(d/'round4_exact_regularized.cpp').read_text().replace('round4_exact_refit.h','round4_exact_fast.h')
(d/'round4_exact_fast.cpp').write_text(source)

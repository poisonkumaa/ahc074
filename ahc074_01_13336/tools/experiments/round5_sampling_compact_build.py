from pathlib import Path
p=Path(__file__).parent
s=(p/'round5_sampling.cpp').read_text()
beg=s.index('inline void exact_state_refit(')
end=s.index('\n#include <algorithm>',beg)
a=s[beg:end]
a=a.replace(' const int N=64,K=papers.size(),P=A.size(),M=P*repetitions,INF=30000,unit=regularizer?32:1;',
''' assert(regularizer==0);const auto entry=clock();
 const int N=64,K=papers.size(),P=A.size(),M=P*repetitions,INF=255,unit=1;''')
a=a.replace('array<int,192>pos;array<array<uint8_t,193>,66>cnt;', 'array<uint8_t,192>pos;array<array<uint8_t,193>,65>cnt;')
a=a.replace('l<=N+1;l++','l<=N;l++')
a=a.replace('auto began=clock();','auto began=clock();double forward_time=0;size_t finite_states=0;')
a=a.replace('struct Table{array<array<uint16_t,193>,65>F,B;array<vector<uint8_t>,65>active;};',
'''struct Table{vector<uint16_t>F;array<uint16_t,65>offset;vector<array<uint8_t,193>>B;explicit Table(int rows):B(rows){}};''')
a=a.replace('make_unique<Table>()','make_unique<Table>(N-bottom+1)')
old='''   auto&s=scenarios[e];int v=s.target;auto&t=*tables[e];for(auto&r:t.F)r.fill(INF);for(auto&r:t.active)r.clear();t.B[N].fill(0);
   t.F[0][0]=0;t.active[0].push_back(0);vector<int>cand[64];for(int z=0;z<K;z++)for(int j=start[v][z];j<length[z];j++)cand[j].push_back(z);
   for(int j=0;j<N;j++)for(int z:cand[j]){int len=length[z],q=s.pos[z];for(int h:t.active[j]){int cost=regularizer;if(q>=h){int moved=s.cnt[len][q]-s.cnt[len][h],upper=s.cnt[j+1][q]-s.cnt[j+1][h];cost+=unit*(moved+(upper>moved));}int nh=max(q+1,h),val=t.F[j][h]+cost;if(t.F[len][nh]==INF)t.active[len].push_back(nh);if(val<t.F[len][nh])t.F[len][nh]=val;}}'''
new='''   auto&s=scenarios[e];int v=s.target;auto&t=*tables[e];t.B[N-bottom].fill(0);
   uint8_t fw[65][193],ah[65][193];int ac[65]{};memset(fw,INF,sizeof(fw));fw[0][0]=0;ah[0][ac[0]++]=0;
   vector<int>cand[64];for(int z=0;z<K;z++)for(int j=start[v][z];j<length[z];j++)cand[j].push_back(z);
   for(int j=0;j<N;j++)for(int z:cand[j]){int len=length[z],q=s.pos[z];for(int ix=0;ix<ac[j];ix++){int h=ah[j][ix],cost=0;if(q>=h){int moved=s.cnt[len][q]-s.cnt[len][h],upper=s.cnt[j+1][q]-s.cnt[j+1][h];cost=moved+(upper>moved);}int nh=max(q+1,h),val=fw[j][h]+cost;assert(val<=K);if(fw[len][nh]==INF)ah[len][ac[len]++]=nh;if(val<fw[len][nh])fw[len][nh]=val;}}
   int size=0;for(int j=0;j<N;j++){t.offset[j]=size;size+=ac[j];}t.offset[N]=size;t.F.resize(size);finite_states+=size;
   for(int j=0;j<N;j++)for(int ix=0;ix<ac[j];ix++){int h=ah[j][ix];t.F[t.offset[j]+ix]=(h<<8)|fw[j][h];}'''
assert old in a
a=a.replace(old,new)
a=a.replace('  for(int e=0;e<M;e++){\n   if((e&63)', '  const auto forward_start=clock();\n  for(int e=0;e<M;e++){\n   if((e&63)',1)
a=a.replace('  for(int l=N;l>=bottom;l--){','  forward_time+=double(clock()-forward_start)/CLOCKS_PER_SEC;\n  for(int l=N;l>=bottom;l--){')
a=a.replace('vector<array<int,65>>route','vector<array<uint8_t,65>>route')
a=a.replace('auto&F=t.F;auto&B=t.B;','auto&B=t.B;')
a=a.replace('B[l]', 'B[l-bottom]').replace('B[len]', 'B[len-bottom]')
a=a.replace('for(int h:t.active[j])bypass[e]=min(bypass[e],F[j][h]+add(j,h,z)+B[len-bottom][max(q+1,h)]);',
'''for(int fi=t.offset[j];fi<t.offset[j+1];fi++){int packed=t.F[fi],h=packed>>8,f=packed&255;bypass[e]=min(bypass[e],f+add(j,h,z)+B[len-bottom][max(q+1,h)]);}''')
a=a.replace('for(int h:t.active[j])value=min(value,F[j][h]+add(j,h,z)+B[l-bottom][max(pos+1,h)]);',
'''for(int fi=t.offset[j];fi<t.offset[j+1];fi++){int packed=t.F[fi],h=packed>>8,f=packed&255;value=min(value,f+add(j,h,z)+B[l-bottom][max(pos+1,h)]);}''')
a=a.replace('value=min(base,route[e*Q+q][st]);','value=min(base,int(route[e*Q+q][st]));')
a=a.replace('<<" CPU="<<double(clock()-began)/CLOCKS_PER_SEC', '<<" snapshot="<<double(began-entry)/CLOCKS_PER_SEC<<" forward="<<forward_time<<" finite="<<double(finite_states)/M<<" CPU="<<double(clock()-began)/CLOCKS_PER_SEC')
assert 't.active' not in a and 'F[j][h]' not in a
s=s[:beg]+a+s[end:]
(p/'round5_sampling_compact.cpp').write_text(s)

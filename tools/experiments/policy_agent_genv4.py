from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy_agent_v4.cpp').read_text()
s='#pragma GCC optimize("Ofast,unroll-loops")\n'+s
s=s.replace('auto ast=chrono::steady_clock::now();', 'clock_t ast=clock();')
s=s.replace('chrono::duration<double>(chrono::steady_clock::now()-ast).count()', 'double(clock()-ast)/CLOCKS_PER_SEC')
s=s.replace('Graph g;vector<unique_ptr<Policy>>pol;', 'cerr<<"SA="<<iter<<" CPU="<<double(clock())/CLOCKS_PER_SEC<<endl; Graph g;cerr<<"GraphCPU="<<double(clock())/CLOCKS_PER_SEC<<endl;vector<unique_ptr<Policy>>pol;')
s=s.replace('total+=ops.size();', 'if(r==R-1)cerr<<"FinishCPU="<<double(clock())/CLOCKS_PER_SEC<<endl; total+=ops.size();')
(p/'policy_agent_v4opt.cpp').write_text(s)
s=s.replace('struct Edge{uint32_t next;uint8_t paper,pos;};', 'struct Edge{uint32_t next:19,paper:4,pos:4;};')
s=s.replace('if(i!=d){auto p=perm[s];', 'if(i!=d){if(i&&d==i-1&&(vis>>perm[s][i-1]&1))continue;auto p=perm[s];')
s=s.replace('const int STEPS=160000;', 'const int STEPS=250000;')
(p/'policy_agent_v4packed.cpp').write_text(s)
s=s.replace('const int STEPS=250000;', 'const int STEPS=160000;')
s=s.replace('pass<2;', 'pass<6;')
(p/'policy_agent_v4finish.cpp').write_text(s)
s=s.replace('pass<6;', 'pass<2;')
for temp in ['.01','.02','.08']:
    (p/('policy_agent_v4temp'+temp[1:]+'.cpp')).write_text(s.replace('temp=.04*pow(.008,progress)',f'temp={temp}*pow(.008,progress)'))
s=s.replace('temp=.04*pow(.008,progress)', 'temp=.02*pow(.008,progress)')
s=s.replace('const double BUDGET=.65;', 'const double BUDGET=.40;')
s=s.replace('Graph&g;vector<int>f;', 'bool fast=false;Graph&g;vector<int>f;')
s=s.replace('Graph::Edge step(int t,int s){int val=calc(t,s),d=g.dist[s*256+t];', '''Graph::Edge step(int t,int s){
 if(fast){int d=g.dist[s*256+t],best=INT_MAX,be=-1;for(int e=g.off[s];e<g.off[s+1];e++){auto op=g.edges[e];if(g.dist[op.next*256+t]+1!=d)continue;int q=quality[op.next]==65535?g.uq[op.next]:quality[op.next];if(q<best){best=q;be=e;}}if(be<0)abort();return g.edges[be];}
 int val=calc(t,s),d=g.dist[s*256+t];''')
s=s.replace('vector<pair<int,int>>ops;for(int z=0;z<8;z++)', 'bool fast=double(clock())/CLOCKS_PER_SEC>1.65;vector<pair<int,int>>ops;for(int z=0;z<8;z++){pol[z]->fast=fast;}for(int z=0;z<8;z++)')
(p/'policy_agent_v4guard.cpp').write_text(s)
s=s.replace(' Graph g;cerr<<"GraphCPU="', ' cerr<<"AfterSA="')
s=s.replace('array<int,64>col,flip{};', 'Graph g;cerr<<"GraphCPU="<<double(clock())/CLOCKS_PER_SEC<<endl;array<int,64>col,flip{};')
s=s.replace('const double BUDGET=.40;', 'const double BUDGET=max(.02,min(.40,1.35-double(clock())/CLOCKS_PER_SEC));')
(p/'policy_agent_v4safe.cpp').write_text(s)
s=(p/'policy_agent_v4guard.cpp').read_text()
s=s.replace('int pc[512],fac[10];','''int pc[512],fac[10];
 inline int distance(int s,int t)const{return (dist[s*128+(t>>1)]>>((t&1)*4))&15;}
 inline void setdist(int s,int t,int d){auto&v=dist[s*128+(t>>1)];int shift=(t&1)*4;v=(v&~(15<<shift))|(d<<shift);}
''')
s=s.replace('dist.assign(S*256,255)', 'dist.assign(S*128,255)')
s=s.replace('dist[s*256+display[s]]=0', 'setdist(s,display[s],0)')
s=s.replace('dist[s*256+k*64+b]=d', 'setdist(s,k*64+b,d)')
s=s.replace('g.dist[s*256+t]', 'g.distance(s,t)')
s=s.replace('g.dist[op.next*256+t]', 'g.distance(op.next,t)')
s=s.replace('for(int t=0;t<256;t++)sum+=f[t]*g.distance(s,t);', 'for(int t=0;t<128;t++){int v=g.dist[s*128+t];sum+=f[t*2]*(v&15)+f[t*2+1]*(v>>4);}')
(p/'policy_agent_v4nibble.cpp').write_text(s)

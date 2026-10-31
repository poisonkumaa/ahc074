from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy2_agent_perf_arrays_dp.cpp').read_text()
old='''   auto excluded=fw[v];excluded[l]=INF;
   for(int k:level[l])if(k<fixed)for(int j=start[v][k];j<l;j++)excluded[l]=std::min(excluded[l],excluded[j]+1);
   for(int u=l+1;u<=n;u++){excluded[u]=INF;for(int k:level[u])for(int j=start[v][k];j<u;j++)excluded[u]=std::min(excluded[u],excluded[j]+1);}
   int bypass=excluded[n],node=0,bestprefix=INF,prev=0;'''
new='''   int prefix_min[65];prefix_min[l]=INF;for(int j=l-1;j>=0;j--)prefix_min[j]=std::min(prefix_min[j+1],fw[v][j]);
   int bypass=INF;for(int u=l+1;u<=n;u++)for(int k:level[u])if(start[v][k]<l)bypass=std::min(bypass,prefix_min[start[v][k]]+1+bw[v][u]);
   for(int k:level[l])if(k<fixed&&start[v][k]<l)bypass=std::min(bypass,prefix_min[start[v][k]]+1+bw[v][l]);
   int node=0,bestprefix=INF,prev=0;'''
assert old in s;s=s.replace(old,new)
a=s.index('inline void refit_all_prefix_papers(')
t=s[a:]
t=t.replace('if(!amount)continue;distances();std::vector<Node>trie(1);','if(!amount)continue;distances();std::vector<Node>trie(1);std::vector<int>without_level(P);')
old='''   auto f=F[v];f[l]=INF;for(int upper=l+1;upper<=N;upper++){f[upper]=INF;for(int k:level[upper])for(int j=start[v][k];j<upper;j++)f[upper]=std::min(f[upper],f[j]+1);}
   int bypass=f[N],bestprefix=INF,previous=0,x=0;'''
new='''   int prefix_min[65];prefix_min[l]=INF;for(int j=l-1;j>=0;j--)prefix_min[j]=std::min(prefix_min[j+1],F[v][j]);
   int bypass=INF;for(int upper=l+1;upper<=N;upper++)for(int k:level[upper])if(start[v][k]<l)bypass=std::min(bypass,prefix_min[start[v][k]]+1+B[v][upper]);
   without_level[v]=bypass;int bestprefix=INF,previous=0,x=0;'''
assert old in t;t=t.replace(old,new)
old='''  bool valid=true;for(int v=0;v<P&&valid;v++){auto f=F[v];f[l]=INF;for(uint64_t c:pattern){uint64_t d=(c^A[v])&PrefixDP::low(l);int st=d?64-__builtin_clzll(d):0;for(int j=st;j<l;j++)f[l]=std::min(f[l],f[j]+1);}for(int upper=l+1;upper<=N;upper++){f[upper]=INF;for(int k:level[upper])for(int j=start[v][k];j<upper;j++)f[upper]=std::min(f[upper],f[j]+1);}if(f[N]>=INF)valid=false;}'''
new='''  bool valid=true;for(int v=0;v<P&&valid;v++)if(without_level[v]>=INF){int route=INF;for(uint64_t c:pattern){uint64_t d=(c^A[v])&PrefixDP::low(l);int st=d?64-__builtin_clzll(d):0;for(int j=st;j<l;j++)route=std::min(route,F[v][j]+1+B[v][l]);}if(route>=INF)valid=false;}'''
assert old in t;t=t.replace(old,new)
s=s[:a]+t
(p/'policy2_agent_perf_training.cpp').write_text(s)
s=(p/'policy2_agent_perf_compare.py').read_text().replace("['policy2_agent_perf_base','policy2_agent_perf_arrays','policy2_agent_perf_arrays_dp']","['policy2_agent_perf_base','policy2_agent_perf_training']").replace('policy2_agent_perf_compare.json','policy2_agent_perf_training_compare.json')
(p/'policy2_agent_perf_training_compare.py').write_text(s)

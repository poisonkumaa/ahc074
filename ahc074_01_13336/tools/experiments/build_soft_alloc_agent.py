from pathlib import Path
p=Path(__file__).parent
h=(p/'greedy_alloc_agent.h').read_text()
h=h.replace('int lengthpenalty,int minlen)','int lengthpenalty,int minlen,double beta)')
h=h.replace('vector<array<int,192>>start(P);vector<array<int,65>>F(P),B(P);','vector<array<int,192>>start(P);vector<array<double,65>>F(P),B(P);double factor=exp(-beta);')
a=h.index(' auto distances=');b=h.index('\n struct Node',a)
h=h[:a]+''' auto distances=[&](){double total=0;for(int v=0;v<P;v++){auto&f=F[v];auto&b=B[v];f.fill(0);b.fill(0);f[0]=1;b[N]=1;for(int l=1;l<=N;l++)for(int k:level[l])for(int j=start[v][k];j<l;j++)f[l]+=f[j]*factor;for(int l=N;l>0;l--)for(int k:level[l])for(int j=start[v][k];j<l;j++)b[j]+=b[l]*factor;total-=log(f[N])/beta;}return total;};'''+h[b:]
h=h.replace('int child[2]{-1,-1},weight=0,best=0,sample=0,chosen=0;','int child[2]{-1,-1},sample=0,chosen=0;double weight=0,best=0;')
h=h.replace('int initial=distances(),last=initial;','double initial=distances(),last=initial;')
h=h.replace('distances();int bestscore=INT_MIN,bestl=-1,bestv=-1;','distances();double bestscore=-1e300;int bestl=-1,bestv=-1;')
h=h.replace('if(extras[l]>=maxextra)continue;','if(extras[l]>=maxextra||level[l].size()>=8)continue;')
h=h.replace('int pre=INF,prev=0;','double contribution=0,prev=0;')
h=h.replace('pre=min(pre,F[v][l-d]);int benefit=max(0,F[v][N]-(pre+1+B[v][l]));nodes[path[l][v][d-1]].weight+=importance*(benefit-prev)+1;prev=benefit;','contribution+=F[v][l-d]*factor*B[v][l];double benefit=log1p(contribution/F[v][N]);nodes[path[l][v][d-1]].weight+=benefit-prev;prev=benefit;')
h=h.replace('node.best=node.weight;node.chosen=node.sample;','node.best=node.weight;node.chosen=node.sample;for(int k:level[l])if(papers[k].color==(A[node.sample]&PrefixDP::low(l)))node.best=-1e300;')
h=h.replace('int score=nodes[0].best-lengthpenalty*extras[l];','double score=nodes[0].best-lengthpenalty*extras[l];')
(p/'soft_alloc_agent.h').write_text(h)
s=(p/'greedy_alloc_agent.cpp').read_text().replace('#include "greedy_alloc_agent.h"','#include "soft_alloc_agent.h"')
s=s.replace('argc>7?atoi(argv[7]):1);','argc>7?atoi(argv[7]):1,argc>8?atof(argv[8]):2);')
(p/'soft_alloc_agent.cpp').write_text(s)

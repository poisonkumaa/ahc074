from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy2_agent_union.cpp').read_text()
s=s.replace('const std::vector<std::array<uint64_t,4>>* access=nullptr, int words=4','const std::vector<uint64_t>* access=nullptr')
s=s.replace('uint64_t unions[65][193][4];','uint64_t unions[65][193];')
s=s.replace('for(int l=1;l<=N;l++){for(int w=0;w<words;w++)unions[l][K][w]=0;\n       for(int i=K-1;i>=0;i--){int k=order[i];for(int w=0;w<words;w++)unions[l][i][w]=unions[l][i+1][w]|(length[k]<=l?(*access)[k][w]:0ULL);}', 'for(int l=1;l<=N;l++){unions[l][K]=0;\n       for(int i=K-1;i>=0;i--){int k=order[i];unions[l][i]=unions[l][i+1]|(length[k]<=l?(*access)[k]:0ULL);}')
s=s.replace('if(access){for(int w=0;w<words;w++)initial_quality+=__builtin_popcountll(unions[length[k]][i+1][w]);}', 'if(access)initial_quality+=__builtin_popcountll(unions[length[k]][i+1]);')
s=s.replace('uint64_t own[4]{};int own_count=0;\n            if(access)for(int w=0;w<words;w++){own[w]=unions[kl][i+1][w];own_count+=__builtin_popcountll(own[w]);}', 'uint64_t own=0;int own_count=0;\n            if(access){own=unions[kl][i+1];own_count=__builtin_popcountll(own);}')
s=s.replace('int next_count=0;for(int w=0;w<words;w++)next_count+=__builtin_popcountll(unions[kl][d+1][w]);quality+=next_count-own_count;own_count=next_count;\n                     if(xl>=kl)for(int w=0;w<words;w++)quality+=__builtin_popcountll((*access)[k][w]&~unions[xl][d+1][w]);', 'int next_count=__builtin_popcountll(unions[kl][d+1]);quality+=next_count-own_count;own_count=next_count;\n                     if(xl>=kl)quality+=__builtin_popcountll((*access)[k]&~unions[xl][d+1]);')
s=s.replace('int next_count=0;for(int w=0;w<words;w++){own[w]|=(*access)[x][w];next_count+=__builtin_popcountll(own[w]);}quality+=next_count-own_count;own_count=next_count;', 'own|=(*access)[x];int next_count=__builtin_popcountll(own);quality+=next_count-own_count;own_count=next_count;')
s=s.replace('int samples=argc>1?atoi(argv[1]):64,words=(samples+63)/64;vector<array<uint64_t,4>> future(papers.size());', 'int samples=64;vector<uint64_t> future(papers.size());')
s=s.replace('future[k][(r-128)/64]|=1ULL<<((r-128)%64)', 'future[k]|=1ULL<<(r-128)')
s=s.replace('selected,.5,&future,words','selected,.5,&future')
(p/'policy2_agent_union64.cpp').write_text(s)
# Copy with online feedback. Initial public-only set retains reproducibility.
pos=s.index('  cout<<operations.size()')
s=s[:pos]+'''  if(argc>1&&atoi(argv[1])){int period=atoi(argv[1]);if(r%period==0){int slot=(r/period)%64;uint64_t bit=1ULL<<slot;for(int k=0;k<int(papers.size());k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);}}
''' + s[pos:]
(p/'policy2_agent_union_adapt.cpp').write_text(s)

from pathlib import Path
p=Path(__file__).parent
original=(p.parent.parent/'main.cpp').read_text()
s=original
def transform(body,F,B,N,backbone):
    # Every finalized edge contributes once to suffix costs and to every
    # boundary crossed by that edge. F below the current level is immutable.
    helper=f'''
 std::vector<std::array<int,65>>cross(P);
 auto advance=[&](int l){{for(int v=0;v<P;v++)for(int k:level[l]){{int best=INF;for(int j=start[v][k];j<l;j++){{
  {B}[v][j]=std::min({B}[v][j],1+{B}[v][l]);best=std::min(best,{F}[v][j]);if(j+1<l)cross[v][j+1]=std::min(cross[v][j+1],best+1+{B}[v][l]);
 }}}}}};
 auto check=[&](int l){{
#ifdef INITIAL_DP_CHECK
  auto savedF={F},savedB={B};distances();for(int v=0;v<P;v++){{for(int j=0;j<l;j++)assert(savedF[v][j]=={F}[v][j]);for(int j=l;j<={N};j++)assert(savedB[v][j]=={B}[v][j]);
   int prefix_min[65];prefix_min[l]=INF;for(int j=l-1;j>=0;j--)prefix_min[j]=std::min(prefix_min[j+1],{F}[v][j]);int reference=INF;
   for(int upper=l+1;upper<={N};upper++)for(int k:level[upper])if(start[v][k]<l)reference=std::min(reference,prefix_min[start[v][k]]+1+{B}[v][upper]);assert(reference==cross[v][l]);
  }}{F}=std::move(savedF);{B}=std::move(savedB);
#else
  (void)l;
#endif
 }};
'''
    marker=' struct Node{'
    assert marker in body
    body=body.replace(marker,helper+marker,1)
    old=f' for(int pass=0;pass<passes;pass++)for(int l={N};l>0;l--){{'
    new=f''' for(int pass=0;pass<passes;pass++){{
  if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>STRUCTURE2_TRAINING_CPU_LIMIT){{multi_training_finished=false;break;}}
  distances();for(int v=0;v<P;v++){{{B}[v].fill(INF);{B}[v][{N}]=0;cross[v].fill(INF);}}
  for(int l={N};l>0;l--){{'''
    assert old in body
    body=body.replace(old,new)
    if backbone:
        old='if(!amount)continue;distances();std::vector<Node>trie(1);'
        new='if(!amount){advance(l);continue;}check(l);std::vector<Node>trie(1);'
        assert old in body
        body=body.replace(old,new)
        old='int bypass=INF;for(int u=l+1;u<=n;u++)for(int k:level[u])if(start[v][k]<l)bypass=std::min(bypass,prefix_min[start[v][k]]+1+bw[v][u]);'
        assert old in body
        body=body.replace(old,'int bypass=cross[v][l];')
        end='''  for(int q=0;q<int(pattern.size());q++){int k=extra[l][q];papers[k].color=pattern[q];update(k);}
 }
 std::cerr<<"backbone "'''
        replacement='''  for(int q=0;q<int(pattern.size());q++){int k=extra[l][q];papers[k].color=pattern[q];update(k);}
  advance(l);
 }
 }
 std::cerr<<"backbone "'''
    else:
        body=body.replace('int accepted=0,rejected=0;bool dirty=false;', 'int accepted=0,rejected=0;')
        old='if(!amount)continue;if(dirty){distances();dirty=false;}std::vector<Node>trie(1);'
        assert old in body
        body=body.replace(old,'if(!amount){advance(l);continue;}check(l);std::vector<Node>trie(1);')
        old='int bypass=INF;for(int upper=l+1;upper<=N;upper++)for(int k:level[upper])if(start[v][k]<l)bypass=std::min(bypass,prefix_min[start[v][k]]+1+B[v][upper]);'
        assert old in body
        body=body.replace(old,'int bypass=cross[v][l];')
        body=body.replace('{rejected++;continue;}', '{rejected++;advance(l);continue;}')
        body=body.replace('update(k);dirty=true;', 'update(k);')
        end='''accepted++;
 }
 std::cerr<<"refit "'''
        replacement='''accepted++;
  advance(l);
 }
 }
 std::cerr<<"refit "'''
    assert end in body
    return body.replace(end,replacement)

start=s.index('inline void train_backbone_dictionary(');end=s.index('inline void refit_all_prefix_papers(',start)
s=s[:start]+transform(s[start:end],'fw','bw','n',True)+s[end:]
start=s.index('inline void refit_all_prefix_papers(');end=s.index('// Construct the final permutation',start)
s=s[:start]+transform(s[start:end],'F','B','N',False)+s[end:]
(p/'round6_initial_fast.cpp').write_text(s)
def diagnostic(code):
    code=code.replace('int main(){','int main(int argc,char**argv){')
    code=code.replace('exact_state_refit(A,papers,1,43,32,-1,0);','cerr<<"InitialCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\\n";exact_state_refit(A,papers,1,43,32,argc>1?atoi(argv[1]):8,0);')
    return code
(p/'round6_initial_fast_diag.cpp').write_text(diagnostic(s))
(p/'round6_initial_reference.cpp').write_text(diagnostic(original))

from pathlib import Path
p=Path(__file__).parent
s=(p.parent.parent/'main.cpp').read_text()
s=s.replace('inline void train_backbone_dictionary(', 'static int round6_unique_mode=1,round6_unique_passes=4,round6_unique_repetitions=8;\ninline void train_backbone_dictionary(',1)
start=s.index('inline void train_backbone_dictionary(');end=s.index('inline void refit_all_prefix_papers(',start)
f=s[start:end]
f=f.replace('double initial=distances();','double initial=distances();vector<char>used(P);')
f=f.replace('sample=0,available=1','sample=-1,available=1')
f=f.replace('trie[node].sample=v;', 'if(trie[node].sample<0||!used[v]||round6_unique_mode==0)trie[node].sample=v;')
f=f.replace('if(a<0&&b<0){v.dp[1]=v.weight;continue;}', 'if(a<0&&b<0){if(round6_unique_mode&&used[v.sample]){v.available=0;}else v.dp[1]=v.weight;continue;}')
f=f.replace('pattern.push_back(A[v.sample]&PrefixDP::low(l));return;', 'pattern.push_back(A[v.sample]&PrefixDP::low(l));if(round6_unique_mode)used[v.sample]=true;return;')
s=s[:start]+f+s[end:]
start=s.index('inline void refit_all_prefix_papers(');end=s.index('// Construct the final permutation',start)
f=s[start:end]
f=f.replace('std::vector<Node>trie(1);std::vector<int>without_level(P);', '''std::vector<Node>trie(1);std::vector<int>without_level(P);vector<char>used(P);if(round6_unique_mode>=2&&l>=32)for(int k=0;k<K;k++)if(papers[k].len>=32&&papers[k].len!=l)for(int v=0;v<P;v++)if((A[v]&PrefixDP::low(papers[k].len))==papers[k].color){used[v]=true;break;}''')
f=f.replace('sample=0,available=1','sample=-1,available=1')
f=f.replace('trie[x].sample=v;', 'if(trie[x].sample<0||!used[v])trie[x].sample=v;')
f=f.replace('if(a<0&&b<0){p.dp[1]=p.weight;continue;}', 'if(a<0&&b<0){if(used[p.sample]){p.available=0;}else p.dp[1]=p.weight;continue;}')
s=s[:start]+f+s[end:]
start=s.index('inline void exact_state_refit(');end=s.index('inline bool direct_moves(',start)
f=s[start:end]
f=f.replace('long long best=LLONG_MIN;int sample=-1;', '''vector<char>used(P);if(round6_unique_mode>=3)for(int k=0;k<K;k++)if(k!=z&&length[k]>=32)for(int v=0;v<P;v++)if((A[v]&PrefixDP::low(length[k]))==papers[k].color){used[v]=true;break;}
   long long best=LLONG_MIN;int sample=-1;''')
f=f.replace('if(leaf&&node.weight>best)', 'if(leaf&&!used[node.sample]&&node.weight>best)')
s=s[:start]+f+s[end:]
s=s.replace('refit_all_prefix_papers(A,papers,4,32);','refit_all_prefix_papers(A,papers,round6_unique_passes,32);')
s=s.replace('int main(){', 'int main(int argc,char**argv){\n round6_unique_mode=argc>1?atoi(argv[1]):1;round6_unique_passes=argc>2?atoi(argv[2]):4;round6_unique_repetitions=argc>3?atoi(argv[3]):8;')
s=s.replace('exact_state_refit(A,papers,1,43,32,-1,0);','if(round6_unique_repetitions)exact_state_refit(A,papers,1,43,32,round6_unique_repetitions,0);')
s=s.replace(' auto future=selection_teacher(A,papers);\n PrefixDP dp(n,papers);',''' set<int>represented;int count=0;for(auto paper:papers)if(paper.len>=43){count++;for(int v=0;v<P;v++)if((A[v]&PrefixDP::low(paper.len))==paper.color){represented.insert(v);break;}}cerr<<"Unique representatives="<<represented.size()<<" longpapers="<<count<<"\\n";
 auto future=selection_teacher(A,papers);
 PrefixDP dp(n,papers);''')
(p/'round6_unique.cpp').write_text(s)

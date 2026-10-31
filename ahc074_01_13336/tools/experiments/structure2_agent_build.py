from pathlib import Path
here=Path(__file__).resolve().parent
src=(here/'structure2_agent_backbone.cpp').read_text()
core=src[:src.index('vector<U64> trie_patterns')]
core=core.replace('  for(int j=0;j<n;j++){\n   vector<int>cand;', '  for(int j=0;j<n;j++){\n   vector<int>active;for(int h=0;h<=k;h++)if(dp[j][h]<1000000)active.push_back(h);if(active.empty())continue;\n   vector<int>cand;')
core=core.replace('for(int h=0;h<=k;h++)if(dp[j][h]<1000000){','for(int h:active){')
backbone=(here/'backbone_dictionary.h').read_text().replace('#pragma once','')
refit=(here/'structure2_agent_refit.h').read_text().replace('#pragma once','')
backbone=backbone.replace('std::array<int,33>dp,split','std::array<int,9>dp,split')
refit=refit.replace('std::array<int,65>dp,split','std::array<int,9>dp,split')
guard='if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>STRUCTURE2_TRAINING_CPU_LIMIT)break;'
backbone=backbone.replace('for(int pass=0;pass<passes;pass++)for(int l=n;l>0;l--){','for(int pass=0;pass<passes;pass++)for(int l=n;l>0;l--){'+guard)
refit=refit.replace('for(int pass=0;pass<passes;pass++)for(int l=N;l>0;l--){','for(int pass=0;pass<passes;pass++)for(int l=N;l>0;l--){'+guard)
main=src[src.index('int main(int argc,char**argv){'):]
main=main.replace('int main(int argc,char**argv){','int main(){')
main=main.replace(' mt19937_64 rng(12345);',' structure2_training_origin=clock(); mt19937_64 rng(12345);')
main=main.replace('reverse(cols.begin()+x,cols.begin()+y+1);int next=cost();', '''auto edge=[&](int a,int b){int d=correlation[a][b];return min(d,P-d);};int next=current;
  if(x)next+=edge(cols[x-1],cols[y])-edge(cols[x-1],cols[x]);
  if(y<63)next+=edge(cols[x],cols[y+1])-edge(cols[y],cols[y+1]);
  reverse(cols.begin()+x,cols.begin()+y+1);''')
start=main.index(' vector<PrefixPaper>papers;')
end=main.index(' if(papers.size()>size_t(K))return 0;',start)
main=main[:start]+''' vector<PrefixPaper>papers;
 train_backbone_dictionary(A,papers,n,6,1,8);
 refit_all_prefix_papers(A,papers,1,8);
 cerr<<"PrepCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\\n";
'''+main[end:]
main=main.replace('double(clock())/CLOCKS_PER_SEC','double(clock()-structure2_training_origin)/CLOCKS_PER_SEC')
defs='''
#ifndef STRUCTURE2_TRAINING_CPU_LIMIT
#define STRUCTURE2_TRAINING_CPU_LIMIT 0.9
#endif
static clock_t structure2_training_origin=0;
'''
(here/'structure2_agent_main.cpp').write_text(core+defs+backbone+'\n'+refit+'\n'+main)

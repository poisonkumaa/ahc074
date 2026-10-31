from pathlib import Path
import re
p=Path(__file__).parent
s=(p.parent.parent/'main.cpp').read_text()
s=re.sub(r'#define STRUCTURE2_TRAINING_CPU_LIMIT [\d.]+','#define STRUCTURE2_TRAINING_CPU_LIMIT 100.0',s)
fast=(p/'simrefit_agent.cpp').read_text()
fast=fast[fast.index('struct PrefixDP {'):fast.index('\n#include <algorithm>')]
fast=fast.replace('cnt,work,parent,which;','cnt,work,parent,which;vector<int>reward;')
fast=fast.replace('which(n+1){}','which(n+1){reward.assign(k,16);}')
fast=fast.replace('1000000','1000000000').replace('add*256-1','add*65536-reward[z]').replace('(dp[n][h]+255)/256','(dp[n][h]+65535)/65536')
s=s[:s.index('struct PrefixDP {')]+fast+s[s.index('\n#include <algorithm>'):]
prefix=s[:s.index('int main(){')]
helper=(p/'select_policy_agent.h').read_text()
helper=helper[:helper.index('inline vector<uint64_t> selection_design')].replace('#pragma once','')
a=s.index('mt19937_64 rng(12345);');b=s.index(' cerr<<"PrepCPU=',a)
train=s[a:b].replace('mt19937_64 rng(12345);','mt19937_64 rng(random_seed==32||random_seed==1?12345:random_seed);')
train=train.replace(' for(int it=0;it<300000;it++){',' static vector<int>saved_columns;if(saved_columns.empty()||(random_seed!=12345&&random_seed!=32&&random_seed!=1)){\n for(int it=0;it<300000;it++){')
train=train.replace(' cols=bestcols;for(int j=1;j<n;j++)',' if(random_seed==12345||random_seed==32||random_seed==1)saved_columns=bestcols;}else bestcols=saved_columns;\n cols=bestcols;if(random_seed==32)rotate(cols.begin(),cols.begin()+32,cols.end());if(random_seed==1)reverse(cols.begin(),cols.end());for(int j=1;j<n;j++)')
model='''
struct MultiModel {vector<U64>A;vector<int>cols,flip;vector<PrefixPaper>papers;vector<uint64_t>future;};
MultiModel train_multi_model(const vector<U64>&original,U64 random_seed){
 int n=64,K=192,P=256;vector<U64>A=original;
 '''+train+'''
 auto future=selection_teacher(A,papers);return {move(A),move(cols),move(flip),move(papers),move(future)};
}
'''
body='''
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);int n,K,P,R;if(!(cin>>n>>K>>P>>R)||n!=64||K!=192||P!=256)return 0;
 vector<U64>original(P);for(int v=0;v<P;v++){string text;cin>>text;if(int(text.size())!=n)return 0;for(int j=0;j<n;j++)if(text[j]=='1')original[v]|=1ULL<<j;}
 structure2_training_origin=clock();int count=argc>1?atoi(argv[1]):1024;double guard=argc>2?atof(argv[2]):1.4;U64 seed=argc>3?strtoull(argv[3],nullptr,10):67890;int targetmode=argc>4?atoi(argv[4]):0;
 vector<int>targets;mt19937 sample_rng(377321);if(!targetmode){int last=-1;for(int r=0;r<count;r++){int v=sample_rng()%P;while(v==last)v=sample_rng()%P;targets.push_back(v);last=v;}}
 else{vector<int>batch(P);iota(batch.begin(),batch.end(),0);while(int(targets.size())<count){shuffle(batch.begin(),batch.end(),sample_rng);if(!targets.empty()&&batch[0]==targets.back())swap(batch[0],batch[1]);for(int v:batch)if(int(targets.size())<count)targets.push_back(v);}}
 auto chosen_model=train_multi_model(original,12345);vector<int>partial;int best=selection_evaluate(chosen_model.A,chosen_model.papers,chosen_model.future,targets,guard,partial);int winner=0;
 cerr<<"Model 0 "<<best<<" CPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\\n";
 if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<guard-.15){
  auto candidate=train_multi_model(original,seed);int score=selection_evaluate(candidate.A,candidate.papers,candidate.future,targets,guard,partial);
  cerr<<"Model 1 "<<score<<" CPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\\n";if(score<best){chosen_model=move(candidate);winner=1;}
 }
 cerr<<"Model winner="<<winner<<"\\n";auto A=move(chosen_model.A);auto cols=move(chosen_model.cols);auto flip=move(chosen_model.flip);auto papers=move(chosen_model.papers);auto future=move(chosen_model.future);
 PrefixDP dp(n,papers);vector<int>order(papers.size()),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);
'''
a=s.index(' for(int k=0;k<K;k++){\n  string pattern')
body+=s[a:]
(p/'multi_select_agent.cpp').write_text(prefix+helper+model+body)

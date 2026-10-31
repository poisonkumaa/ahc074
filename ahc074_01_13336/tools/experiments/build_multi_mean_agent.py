from pathlib import Path
p=Path(__file__).parent
s=(p/'multi_select_agent.cpp').read_text()
fast=(p/'policy2_agent_sparse_baseline.cpp').read_text()
fast=fast[fast.index('struct PrefixDP {'):fast.index('\n#include <algorithm>')]
s=s[:s.index('struct PrefixDP {')]+fast+s[s.index('\n#include <algorithm>'):]
s=s.replace('PrefixDP dp(64,papers);','PrefixDP dp(64,papers);dp.public_data(A);').replace('PrefixDP dp(n,papers);','PrefixDP dp(n,papers);dp.public_data(A);')
a=s.index('inline int selection_evaluate(');b=s.index('struct MultiModel',a)
eval=r'''inline int selection_evaluate(const vector<U64>&A,const vector<PrefixPaper>&papers,vector<uint64_t>future,const vector<int>&targets,double guard,vector<int>&partial){
 PrefixDP dp(64,papers);dp.public_data(A);vector<int>order(papers.size()),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);int total=0;partial.clear();
 for(int r=0;r<int(targets.size());r++){
  if((r&15)==0&&double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>guard)return INT_MAX;
  for(int k=0;k<int(papers.size());k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);
  int moves;auto chosen=dp.solve(order,A[targets[r]],moves);vector<char>selected(papers.size());for(int k:chosen)selected[k]=true;
  for(int i=0;i<moves;i++){auto[from,to]=GlobalPrefix::best_move(order,length,selected,.5,&future);if(from<0)abort();int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);}
  uint64_t bit=1ULL<<(r%64);for(int k=0;k<int(papers.size());k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
  if(r>=128&&(r-128)%8==0){dp.prepare(order);for(int v=0;v<int(A.size());v++)if(v!=targets[r]){if((v&31)==0&&double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>guard)return INT_MAX;int distance;dp.solve_prepared(A[v],distance,false);total+=distance;}partial.push_back(total);}
 }return total;
}
'''
s=s[:a]+eval+s[b:]
s=s.replace('int count=argc>1?atoi(argv[1]):1024;','int count=128+8*(argc>1?atoi(argv[1]):64);')
s=s.replace('#ifndef STRUCTURE2_TRAINING_CPU_LIMIT','static double multi_training_budget=100.0;\n#ifndef STRUCTURE2_TRAINING_CPU_LIMIT').replace('#define STRUCTURE2_TRAINING_CPU_LIMIT 100.0','#define STRUCTURE2_TRAINING_CPU_LIMIT multi_training_budget')
s=s.replace('vector<int>targets;mt19937 sample_rng(377321);','multi_training_budget=guard-.08;vector<int>targets;mt19937 sample_rng(377321);')
s=s.replace(' if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<guard-.15){\n  auto candidate=train_multi_model(original,seed);',' vector<U64>alternative{seed};if(argc>5&&strtoull(argv[5],nullptr,10))alternative.push_back(strtoull(argv[5],nullptr,10));\n for(int model=1;model<=int(alternative.size());model++){if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>=guard-.15)break;\n  auto candidate=train_multi_model(original,alternative[model-1]);')
s=s.replace('cerr<<"Model 1 "<<score','cerr<<"Model "<<model<<" "<<score').replace('chosen_model=move(candidate);winner=1;','chosen_model=move(candidate);winner=model;')
s=s.replace('    return answer;\n}', '    return answer;\n}')
(p/'multi_mean_agent.cpp').write_text(s)
# The alternate implementation uses the first legal improving paper and is
# independently checked against the full minimization by the policy agent.
s=s.replace('        }\n    }\n    return answer;', '        }\n        if(answer.first>=0)break;\n    }\n    return answer;')
(p/'multi_mean_first_agent.cpp').write_text(s)

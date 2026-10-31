from pathlib import Path
p=Path(__file__).parent
s=(p/'goal_reward_fast.cpp').read_text()
fast=(p/'simrefit_agent.cpp').read_text()
fast=fast[fast.index('struct PrefixDP {'):fast.index('\n#include <algorithm>')]
fast=fast.replace('cnt,work,parent,which;','cnt,work,parent,which;vector<int>reward;')
fast=fast.replace('which(n+1){}','which(n+1){reward.assign(k,16);}')
fast=fast.replace('1000000','1000000000').replace('add*256-1','add*65536-reward[z]').replace('(dp[n][h]+255)/256','(dp[n][h]+65535)/65536')
s=s[:s.index('struct PrefixDP {')]+fast+s[s.index('\n#include <algorithm>'):]
s=s.replace('#define STRUCTURE2_TRAINING_CPU_LIMIT 0.9','#define STRUCTURE2_TRAINING_CPU_LIMIT 100.0')
s=s.replace('int main(){','#include "select_policy_agent.h"\nint main(int argc,char**argv){')
s=s.replace('refit_all_prefix_papers(A,papers,1,8);','auto future=selection_design(A,papers,argc>1?atoi(argv[1]):512,argc>2?atof(argv[2]):.9,argc>3?atoi(argv[3]):0);')
a=s.index(' int samples=64;vector<uint64_t> future(')
b=s.index(' cerr<<"PolicyCPU=',a)
s=s[:a]+s[b:]
(p/'select_policy_agent.cpp').write_text(s)

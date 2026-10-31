from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'policy2_agent_union_adapt.cpp').read_text()
start=s.index('struct PrefixDP {');end=s.index('\n};',start)+3
p=s[start:end]
p=p.replace('vector<array<int,193>> cnt;','vector<array<int,193>> cnt;vector<int>reward;')
p=p.replace('cnt(n+2){}','cnt(n+2){reward.assign(k,16);}')
p=p.replace('1000000','1000000000')
p=p.replace('add*256-1','add*65536-reward[z]')
p=p.replace('(dp[n][h]+255)/256','(dp[n][h]+65535)/65536')
s=s[:start]+p+s[end:]
s=s.replace('int product;if(!(cin>>product)||product<0||product>=P)return 0;','int product;if(!(cin>>product)||product<0||product>=P)return 0;\n  for(int k=0;k<int(papers.size());k++)dp.reward[k]=16+(argc>1?atoi(argv[1]):1)*__builtin_popcountll(future[k]);')
# Keep period fixed regardless of the reward experiment argument.
s=s.replace('int period=argc>1?atoi(argv[1]):1','int period=1')
(d/'goal_reward_submit.cpp').write_text(s)

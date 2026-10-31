from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'goal_reward_fast.cpp').read_text();t=(p/'policy2_agent_goal_sparse.cpp').read_text()
a=s.index('struct PrefixDP {');b=s.index('\n};',a)+3;c=t.index('struct PrefixDP {');d=t.index('\n};',c)+3
s=s[:a]+t[c:d]+s[b:]
s=s.replace(' PrefixDP dp(n,papers);vector<int>order',' PrefixDP dp(n,papers);dp.public_data(A);vector<int>order')
(p/'policy2_agent_sparse_baseline.cpp').write_text(s)

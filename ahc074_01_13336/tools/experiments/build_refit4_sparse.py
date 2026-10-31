from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'refit4_reward.cpp').read_text()
fast=(d/'policy2_agent_sparse_baseline.cpp').read_text()
start='struct PrefixDP {';end='\n};'
a=s.index(start);b=s.index(end,a)+len(end)
x=fast.index(start);y=fast.index(end,x)+len(end)
s=s[:a]+fast[x:y]+s[b:]
s=s.replace('PrefixDP dp(n,papers);','PrefixDP dp(n,papers);dp.public_data(A);')
(d/'refit4_sparse.cpp').write_text(s)

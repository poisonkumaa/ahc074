from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'prefix_policy_agent_allocate0.cpp').read_text()
s=s.replace('for(int j=0;j<n;j++){\n   vector<int>cand;', 'for(int j=0;j<n;j++){\n   vector<int>active;for(int h=0;h<=k;h++)if(dp[j][h]<1000000)active.push_back(h);if(active.empty())continue;\n   vector<int>cand;')
s=s.replace('for(int h=0;h<=k;h++)if(dp[j][h]<1000000){', 'for(int h:active){')
(p/'prefix_policy_agent_fastdp.cpp').write_text(s)
s=s.replace('STEPS=90000;', 'STEPS=300000;')
(p/'prefix_policy_agent_tsp300.cpp').write_text(s)

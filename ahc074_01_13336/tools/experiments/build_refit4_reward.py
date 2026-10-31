from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'goal_reward_fast.cpp').read_text()
s=s.replace('refit_all_prefix_papers(A,papers,1,8);','refit_all_prefix_papers(A,papers,4,32);')
(d/'refit4_reward.cpp').write_text(s)

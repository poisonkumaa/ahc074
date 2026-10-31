from pathlib import Path
p=Path(__file__).parent
s=(p/'global_agent_prefix_check.cpp').read_text()
s=s.replace('#include "global_agent_prefix.h"','#define main submission_main\n#include "../../main.cpp"\n#undef main')
s=s.replace('int tested=0;','int tested=0;mt19937_64 rng(733151);vector<uint64_t>future(k);')
s=s.replace('auto [i,j]=GlobalPrefix::best_move(ps[s],len,sel);','for(auto &bits:future)bits=rng();auto [i,j]=GlobalPrefix::best_move(ps[s],len,sel,.5,&future);')
(p/'review_agent_future.cpp').write_text(s)

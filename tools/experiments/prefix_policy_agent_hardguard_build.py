from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'prefix_policy_agent_guard.cpp').read_text()
declaration='''
#ifndef TRAINING_CPU_LIMIT
#define TRAINING_CPU_LIMIT 1.05
#endif
static clock_t training_clock_origin=0;
'''
s=s.replace('// Coordinate optimization of dictionaries',declaration+'\n// Coordinate optimization of dictionaries',1)
s=s.replace('double initial=distances();', 'double initial=distances();int completed_levels=0;')
s=s.replace('for(int pass=0;pass<passes;pass++)for(int z=1;z<=n;z++){', 'for(int pass=0;pass<passes;pass++)for(int z=1;z<=n;z++){\n        if(double(clock()-training_clock_origin)/CLOCKS_PER_SEC>TRAINING_CPU_LIMIT)break;\n        completed_levels++;')
s=s.replace('<<" papers="<<K-spare.size()<<"\\n";', '<<" papers="<<K-spare.size()<<" levels="<<completed_levels<<"\\n";')
s=s.replace('clock_t started=clock();mt19937_64', 'clock_t started=clock();training_clock_origin=started;mt19937_64')
(p/'prefix_policy_agent_hardguard.cpp').write_text(s)

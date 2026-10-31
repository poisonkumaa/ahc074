from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy2_agent_optional.cpp').read_text().replace('if(distance!=initial_distance-1)continue;','if(distance!=initial_distance-1)continue;\n                if(access)candidates.push_back({i,d,quality,invsum});').replace('if(local.to>=0)candidates.push_back(local);','')
(p/'policy2_agent_optional_positions.cpp').write_text(s)

from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy4_agent_direct_compare.cpp').read_text();a=s.index('vector<int> direct_final(');b=s.index('\nint main(',a)
f=s[a:b].replace('vector<int> direct_final(', 'inline std::vector<int> final_order(',1)
f=f.replace('const vector<int>&order,const vector<int>&length,const vector<char>&selected,const vector<uint64_t>*future=nullptr){', 'const std::vector<int>&order,const std::vector<int>&length,const std::vector<char>&selected,const std::vector<uint64_t>*future=nullptr){\n using namespace std;',1)
h='#pragma once\n#include <algorithm>\n#include <cmath>\n#include <cstdint>\n#include <climits>\n#include <vector>\nnamespace GlobalPrefix {\n'+f+'\n}\n'
(p/'policy4_agent_direct.h').write_text(h)
s=s.replace(' int different=0;', ' clock_t direct_ticks=0,path_ticks=0;int different=0;')
s=s.replace('  auto probe=direct_final(order,length,selected,&future);', '  auto direct_tic=clock();auto probe=direct_final(order,length,selected,&future);direct_ticks+=clock()-direct_tic;auto path_tic=clock();')
s=s.replace('  if(probe!=order)', '  path_ticks+=clock()-path_tic;\n  if(probe!=order)')
s=s.replace(' cerr<<"Different="', ' cerr<<"DirectPathCPU="<<double(direct_ticks)/CLOCKS_PER_SEC<<" OldPathCPU="<<double(path_ticks)/CLOCKS_PER_SEC<<"\\n";\n cerr<<"Different="')
(p/'policy4_agent_direct_speed.cpp').write_text(s)

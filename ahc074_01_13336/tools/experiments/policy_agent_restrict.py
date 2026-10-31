from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy_agent_v4nibble.cpp').read_text()
s=s.replace('int pc[512],fac[10];','vector<int>ids; int pc[512],fac[10];')
s=s.replace('int rankp(const array<uint8_t,9>&a)', 'int rankfull(const array<uint8_t,9>&a)')
at=s.index(' Graph(){')
s=s[:at]+' int rankp(const array<uint8_t,9>&a){return ids[rankfull(a)];}\n'+s[at:]
s=s.replace('perm.reserve(S);do{perm.push_back(a);}while(next_permutation(a.begin(),a.end()));display.resize(S);', '''perm.reserve(S);ids.assign(S,-1);int fullrank=0;
 do{int p7=find(a.begin(),a.end(),7)-a.begin(),p8=find(a.begin(),a.end(),8)-a.begin();p7-=p8<p7;
 if(p7<=2&&p8<=p7+1){ids[fullrank]=perm.size();perm.push_back(a);}fullrank++;
 }while(next_permutation(a.begin(),a.end()));S=perm.size();display.resize(S);''')
s=s.replace('edges.push_back({(uint32_t)rankp(p),(uint8_t)k,(uint8_t)d});', 'int ns=rankp(p);if(ns>=0)edges.push_back({(uint32_t)ns,(uint8_t)k,(uint8_t)d});')
s=s.replace('vector<string>design(192,string(64,\'.\'));', 'int remap[9];for(int i=0;i<9;i++)remap[g.perm[0][i]]=i;vector<string>design(192,string(64,\'.\'));')
s=s.replace('design[9*z+k]', 'design[9*z+remap[k]]')
s=s.replace('ops.emplace_back(9*z+e.paper,9*z+e.pos)', 'ops.emplace_back(9*z+remap[e.paper],9*z+e.pos)')
s=s.replace('const double BUDGET=.40;', 'const double BUDGET=.55;')
(p/'policy_agent_v4restrict.cpp').write_text(s)

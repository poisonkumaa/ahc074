from pathlib import Path
p=Path(__file__).parent
s=(p/'round5_sampling_compact.cpp').read_text()
s=s.replace('std::array<int,9>dp,split','std::array<int,17>dp,split')
s=s.replace('int amount=std::min(perlevel,int(extra[l].size()+spare.size()));','int amount=std::min(l>=57?8:4,int(extra[l].size()+spare.size()));')
assert 'std::min(l>=57?8:4' in s
(p/'round5_sampling_profile.cpp').write_text(s)

from pathlib import Path
p=Path(__file__).parent
s=(p/'round5_sampling_profile_sparse.cpp').read_text()
assert 'l>=57?8:4' in s
s=s.replace('l>=57?8:4','l>=53?8:4')
(p/'round5_sampling_cut53.cpp').write_text(s)

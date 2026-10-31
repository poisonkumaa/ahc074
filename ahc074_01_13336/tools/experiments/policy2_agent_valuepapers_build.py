from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy2_agent_value.cpp').read_text()
s=s.replace('static constexpr int F=27;', 'static constexpr int F=219;')
s=s.replace('f[bin(z,j)]++;', 'f[bin(z,j)]++;f[26+z]++;')
s=s.replace('f[26]=1;', 'f[F-1]=1;')
s=s.replace('double c=w[bin(z,j)]+', 'double c=w[26+z]+w[bin(z,j)]+')
(p/'policy2_agent_valuepapers.cpp').write_text(s)

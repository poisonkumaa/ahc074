from pathlib import Path
import re
p=Path(__file__).parent
s=(p.parent.parent/'main.cpp').read_text()
s=re.sub(r'#define STRUCTURE2_TRAINING_CPU_LIMIT [\d.]+','#define STRUCTURE2_TRAINING_CPU_LIMIT 100.0',s)
s=s.replace('int main(){','int main(int argc,char**argv){')
s=s.replace('mt19937_64 rng(12345);','mt19937_64 rng(argc>1?strtoull(argv[1],nullptr,10):12345);')
(p/'multi_column_agent.cpp').write_text(s)

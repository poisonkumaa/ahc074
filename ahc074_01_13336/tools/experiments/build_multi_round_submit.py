from pathlib import Path
import re,hashlib
p=Path(__file__).parent
s=(p/'multi_round_fast_agent.cpp').read_text()
a=s.index('inline int selection_evaluate(');b=s.index('struct MultiModel',a)
s=s[:a]+s[b:]
s=s.replace('int main(int argc,char**argv){','int main(){')
s=s.replace('int state_limit=argc>1?atoi(argv[1]):64;double deadline=argc>2?atof(argv[2]):1.4;multi_training_budget=argc>3?atof(argv[3]):1.0;int desired_models=argc>4?atoi(argv[4]):3;','const int state_limit=128;const double deadline=1.4;multi_training_budget=1.0;const int desired_models=2;')
s=s.replace('for(int id:{32,12345,1})','for(int id:{32,12345})')
assert 'argc' not in s and 'argv' not in s
s='// Fixed configuration: two public-data designs, conditional-average selection.\n// Training stops at 1.0 CPU seconds; model evaluation stops at 1.4 seconds.\n// Public products alone generate the internal simulation; real orders remain reactive.\n'+s
(p/'multi_round_submit.cpp').write_text(s)
print(hashlib.sha256(s.encode()).hexdigest())

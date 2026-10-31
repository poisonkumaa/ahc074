from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'goal_reward_submit.cpp').read_text()
fast=(d/'policy2_agent_union_fast.cpp').read_text()
start='namespace GlobalPrefix {'
end='#ifndef STRUCTURE2_TRAINING_CPU_LIMIT'
a=s.index(start);b=s.index(end,a)+len(end)
x=fast.index(start);y=fast.index(end,x)+len(end)
s=s[:a]+fast[x:y]+s[b:]
s=s.replace('int main(int argc,char**argv)', 'int main()')
s=s.replace('(argc>1?atoi(argv[1]):1)', '2')
assert 'argc' not in s and 'argv' not in s
s=s.replace('// A secondary preference keeps more visible records after a shortest transition.', '// Prefer frequently useful records among goals requiring the same minimum moves.')
(d/'goal_reward_fast.cpp').write_text(s)

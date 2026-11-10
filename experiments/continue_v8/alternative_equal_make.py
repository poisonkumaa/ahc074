"""Allow same-length pair68 endpoint changes when its existing state price improves.

Source only. Root generates fixtures and owns builds/benchmarks.
PAIR68_EQUAL=0 reproduces strict-shortening eligibility; 1 is the default.
"""
from pathlib import Path
import hashlib
root=Path(__file__).resolve().parent
src=root/'alternative_guard.cpp'
assert hashlib.sha256(src.read_bytes()).hexdigest()=='be37ec58e2de0fd4d05e50fa01bdec8b9e702096aed0df08369ac0515eb82d05'
s=src.read_text(encoding='utf-8');begin=s.index('namespace pair68 {');end=s.index('namespace headvalue64 {',begin);p=s[begin:end]
def once(old,new):
    global p
    assert p.count(old)==1,(old,p.count(old))
    p=p.replace(old,new,1)
once('static clock_t distance_ticks=0;', 'static clock_t distance_ticks=0;\nstatic long long equal_wins=0,strict_wins=0;')
once('    static const bool on=!getenv("NO_PAIR68");', '    static const bool on=!getenv("NO_PAIR68");\n    static const bool allow_equal=search_budget::integer("PAIR68_EQUAL",1)!=0;')
once('    auto best=init;double score=ops.size()+r.route_state_value();vector<Op> win;', '''    auto best=init;double score=ops.size()+r.route_state_value();vector<Op> win;
    // The same number of moves can leave a better reusable shelf order.
    // Acceptance still requires a strict improvement in the existing price.
    const int max_moves=int(ops.size())-int(!allow_equal);''')
once('        int nt=take.count();if(nt>=int(ops.size()))continue;', '        int nt=take.count();if(nt>max_moves)continue;')
once('            int n=nt+distance[src*S+zc];if(n>=int(ops.size())||n>18||(!win.empty()&&n>int(win.size())))continue;', '            int n=nt+distance[src*S+zc];if(n>max_moves||n>18||(!win.empty()&&n>int(win.size())))continue;')
once('    if(!win.empty()){++wins;saved+=int(ops.size())-int(win.size());', '    if(!win.empty()){++wins;equal_wins+=win.size()==ops.size();strict_wins+=win.size()<ops.size();saved+=int(ops.size())-int(win.size());')
s=s[:begin]+p+s[end:]
needle='    cerr<<"PAIR68 "'
assert s.count(needle)==1
s=s.replace(needle,'    cerr<<"PAIR68_EQUAL equal_wins="<<pair68::equal_wins<<" strict_wins="<<pair68::strict_wins<<"\\n";\n'+needle,1)
target=root/'alternative_equal.cpp';target.write_text(s,encoding='utf-8')
print(hashlib.sha256(target.read_bytes()).hexdigest())

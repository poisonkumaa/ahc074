"""Admit an already-correct core when donor-covered shelf resets save paid moves.

Only this omitted pair68 neighborhood is added. Root owns fixtures/builds.
PAIR68_ZERO=0 is the original strict filter, 1 enables it by default.
"""
from pathlib import Path
import hashlib
root=Path(__file__).resolve().parent
src=root/'alternative_guard.cpp'
assert hashlib.sha256(src.read_bytes()).hexdigest()=='be37ec58e2de0fd4d05e50fa01bdec8b9e702096aed0df08369ac0515eb82d05'
s=src.read_text(encoding='utf-8');a=s.index('namespace pair68 {');b=s.index('namespace headvalue64 {',a);p=s[a:b]
def once(old,new):
    global p
    assert p.count(old)==1,(old,p.count(old))
    p=p.replace(old,new,1)
once('static clock_t distance_ticks=0;', 'static clock_t distance_ticks=0;\nstatic long long zero_goals=0,zero_wins=0;')
once('    static const bool on=!getenv("NO_PAIR68");', '    static const bool on=!getenv("NO_PAIR68");\n    static const bool allow_zero=search_budget::integer("PAIR68_ZERO",1)!=0;')
once('    auto best=init;double score=ops.size()+r.route_state_value();vector<Op> win;', '    auto best=init;double score=ops.size()+r.route_state_value();vector<Op> win;bool best_zero=false;')
once('        for(auto [k,d]:ops)if(k>=B && (z.reset>>r.paper_bucket[k]&1))take.reset(k);', '''        bool reset_saved=false;
        for(auto [k,d]:ops)if(k>=B && (z.reset>>r.paper_bucket[k]&1)){
            reset_saved=reset_saved || take[k];take.reset(k);
        }''')
once('            const auto& c=cores[zc];uint64_t bad=(c.color^want)&sh;if(!bad || (bad&z.missing))continue;', '''            const auto& c=cores[zc];uint64_t bad=(c.color^want)&sh;
            // The donors can save ordinary shelf repairs even when this core
            // already shows every required suffix bit. Require an actual paid
            // card removal so the new case has a concrete shortening benefit.
            if((!bad && !(allow_zero && reset_saved)) || (bad&z.missing))continue;''')
once('                        ++goals;fixed_deck::Work w(r.opaque,start,goal,n);if(!w.orderable(set))continue;', '                        ++goals;zero_goals+=!bad;fixed_deck::Work w(r.opaque,start,goal,n);if(!w.orderable(set))continue;')
once('                        if(v<score-1e-9){score=v;best=st;win=move(w.answer);}', '                        if(v<score-1e-9){score=v;best=st;win=move(w.answer);best_zero=!bad;}')
once('    if(!win.empty()){++wins;saved+=int(ops.size())-int(win.size());','    if(!win.empty()){++wins;zero_wins+=best_zero;saved+=int(ops.size())-int(win.size());')
s=s[:a]+p+s[b:]
needle='    cerr<<"PAIR68 "';assert s.count(needle)==1
s=s.replace(needle,'    cerr<<"PAIR68_ZERO goals="<<pair68::zero_goals<<" wins="<<pair68::zero_wins<<"\\n";\n'+needle,1)
t=root/'alternative_zero.cpp';t.write_text(s,encoding='utf-8')
print(hashlib.sha256(t.read_bytes()).hexdigest())

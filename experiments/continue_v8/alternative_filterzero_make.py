"""Combine exact core-list filtering with the paid-reset zero-bad neighborhood."""
from pathlib import Path
import hashlib,subprocess,sys
root=Path(__file__).resolve().parent
src=root/'alternative_filter.cpp'
assert hashlib.sha256(src.read_bytes()).hexdigest()=='d4e5be258b4e202bd5228d233460318e37046c73f92cf631fc940cc546f50e5b'
s=src.read_text(encoding='utf-8');a=s.index('namespace pair68 {');b=s.index('namespace headvalue64 {',a);p=s[a:b]
def once(old,new):
    global p
    assert p.count(old)==1,(old,p.count(old))
    p=p.replace(old,new,1)
once('static clock_t distance_ticks=0;', 'static clock_t distance_ticks=0;\nstatic long long zero_goals=0,zero_wins=0;')
once('    static const bool on=!getenv("NO_PAIR68");', '    static const bool on=!getenv("NO_PAIR68");\n    static const bool allow_zero=search_budget::integer("PAIR68_ZERO",1)!=0;')
once('            if(!core_bad[zc])continue;', '            if(!core_bad[zc] && !allow_zero)continue;')
once('    auto best=init;double score=ops.size()+r.route_state_value();vector<Op> win;', '    auto best=init;double score=ops.size()+r.route_state_value();vector<Op> win;bool best_zero=false;')
once('        for(auto [k,d]:ops)if(k>=B && (z.reset>>r.paper_bucket[k]&1))take.reset(k);', '''        bool reset_saved=false;
        for(auto [k,d]:ops)if(k>=B && (z.reset>>r.paper_bucket[k]&1)){
            reset_saved=reset_saved || take[k];take.reset(k);
        }''')
once('            const auto& c=cores[zc];uint64_t bad=filter?core_bad[zc]:(c.color^want)&sh;if(!bad || (bad&z.missing))continue;', '''            const auto& c=cores[zc];uint64_t bad=filter?core_bad[zc]:(c.color^want)&sh;
            // A correct core is useful when the donors still remove paid shelf repairs.
            if((!bad && !(allow_zero && reset_saved)) || (bad&z.missing))continue;''')
once('                        ++goals;fixed_deck::Work w(r.opaque,start,goal,n);if(!w.orderable(set))continue;', '                        ++goals;zero_goals+=!bad;fixed_deck::Work w(r.opaque,start,goal,n);if(!w.orderable(set))continue;')
once('                        if(v<score-1e-9){score=v;best=st;win=move(w.answer);}', '                        if(v<score-1e-9){score=v;best=st;win=move(w.answer);best_zero=!bad;}')
once('    if(!win.empty()){++wins;saved+=int(ops.size())-int(win.size());','    if(!win.empty()){++wins;zero_wins+=best_zero;saved+=int(ops.size())-int(win.size());')
s=s[:a]+p+s[b:]
needle='    cerr<<"PAIR68 "';assert s.count(needle)==1
s=s.replace(needle,'    cerr<<"PAIR68_ZERO goals="<<pair68::zero_goals<<" wins="<<pair68::zero_wins<<"\\n";\n'+needle,1)
t=root/'alternative_filterzero.cpp';t.write_text(s,encoding='utf-8')
subprocess.run([sys.executable,str(root/'make_fixture.py'),str(t),str(root/'alternative_filterzero_fixture.cpp')],check=True)
print(hashlib.sha256(t.read_bytes()).hexdigest())

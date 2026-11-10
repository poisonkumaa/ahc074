from pathlib import Path
import hashlib,subprocess,sys
root=Path(__file__).resolve().parent
src=root/'alternative_both.cpp'
assert hashlib.sha256(src.read_bytes()).hexdigest()=='93fe9f99aec1c9f3a8cb6e1c49a3b3df723f85b5bf01bb686ef63ea211c3eeb5'
s=src.read_text(encoding='utf-8')
print('source',hashlib.sha256(src.read_bytes()).hexdigest())
a=s.index('namespace pair68 {');b=s.index('\nnamespace headvalue64',a)
p=s[a:b]
p=p.replace('static long long calls=0,pairs=0,goals=0,reached=0,wins=0,saved=0,backsave=0,ordsave=0,loans=0,spent=0;', 'static long long calls=0,pairs=0,goals=0,reached=0,wins=0,saved=0,backsave=0,ordsave=0,loans=0,spent=0;\nstatic long long slack_checked=0,slack_reached=0,slack_wins=0;',1)
p=p.replace('    static const bool on=!getenv("NO_PAIR68");', '    static const bool on=!getenv("NO_PAIR68");\n    static const bool slack=search_budget::integer("PAIR68_SLACK",0)!=0;',1)
p=p.replace('    auto best=init;double score=ops.size()+r.route_state_value();vector<Op> win;', '    auto best=init;double score=ops.size()+r.route_state_value();vector<Op> win;bool best_slack=false;',1)
old='''                    for(unsigned mask=0;mask<(1u<<B);mask++){
                        if(__builtin_popcount(mask)!=n-nt)continue;'''
new='''                    bool scheduled=false;
                    // A pure-core shortest move set may become inaccessible
                    // when main/support insertions are scheduled with it. One
                    // additional moved core can unlock a joint legal schedule.
                    for(int extra=0;extra<=int(slack) && !scheduled;extra++){
                    int planned=n+extra;
                    if(planned>=int(ops.size()) || planned>18 || (!win.empty() && planned>int(win.size())))break;
                    if(extra && (clock()>=end || spent+clock()-now>clock_t(.060*CLOCKS_PER_SEC)))break;
                    for(unsigned mask=0;mask<(1u<<B);mask++){
                        if(__builtin_popcount(mask)!=planned-nt)continue;'''
assert p.count(old)==1;p=p.replace(old,new,1)
old='''                        ++goals;fixed_deck::Work w(r.opaque,start,goal,n);if(!w.orderable(set))continue;
                        ++reached;r.restore_route_state(st);double v=w.answer.size()+r.route_state_value();
                        if(v<score-1e-9){score=v;best=st;win=move(w.answer);}
                        break;
                    }'''
new='''                        ++goals;slack_checked+=extra!=0;
                        fixed_deck::Work w(r.opaque,start,goal,planned);if(!w.orderable(set))continue;
                        ++reached;slack_reached+=extra!=0;scheduled=true;
                        r.restore_route_state(st);double v=w.answer.size()+r.route_state_value();
                        if(v<score-1e-9){score=v;best=st;win=move(w.answer);best_slack=extra!=0;}break;
                    }
                    }'''
assert p.count(old)==1;p=p.replace(old,new,1)
p=p.replace('    if(!win.empty()){++wins;saved+=int(ops.size())-int(win.size());','    if(!win.empty()){++wins;slack_wins+=best_slack;saved+=int(ops.size())-int(win.size());',1)
s=s[:a]+p+s[b:]
s=s.replace('    cerr<<"PAIR68 "','    cerr<<"PAIR68_SLACK checked="<<pair68::slack_checked<<" reached="<<pair68::slack_reached<<" wins="<<pair68::slack_wins<<"\\n";\n    cerr<<"PAIR68 "',1)
t=root/'route_pairslack.cpp';t.write_text(s,encoding='utf-8')
subprocess.run([sys.executable,str(root/'make_fixture.py'),str(t),str(root/'route_pairslack_fixture.cpp')],check=True)
print('candidate',hashlib.sha256(t.read_bytes()).hexdigest())

from pathlib import Path
import hashlib,subprocess,sys
root=Path(__file__).resolve().parent
src=root/'alternative_both.cpp'
assert hashlib.sha256(src.read_bytes()).hexdigest()=='93fe9f99aec1c9f3a8cb6e1c49a3b3df723f85b5bf01bb686ef63ea211c3eeb5'
s=src.read_text(encoding='utf-8');a=s.index('namespace pair68 {');b=s.index('\nnamespace headvalue64',a);p=s[a:b]
p=p.replace('static long long calls=0,pairs=0,goals=0,reached=0,wins=0,saved=0,backsave=0,ordsave=0,loans=0,spent=0;', 'static long long calls=0,pairs=0,goals=0,reached=0,wins=0,saved=0,backsave=0,ordsave=0,loans=0,spent=0;\nstatic long long partial_tried=0,partial_reached=0,partial_wins=0;',1)
p=p.replace('    static const bool on=!getenv("NO_PAIR68");','    static const bool on=!getenv("NO_PAIR68");\n    static const bool partial=search_budget::integer("PAIR68_PARTIAL",0)!=0;',1)
p=p.replace('    auto best=init;double score=ops.size()+r.route_state_value();vector<Op> win;', '    auto best=init;double score=ops.size()+r.route_state_value();vector<Op> win;bool best_partial=false;',1)
p=p.replace('''    for(auto z:p){
        if(clock()>=end || spent+clock()-now>clock_t(.060*CLOCKS_PER_SEC))break;''','''    array<int,64> moved_home{};
    for(int k=0;k<K;k++)if(paid[k])++moved_home[r.paper_bucket[k]];
    for(auto original_plan:p){
        if(clock()>=end || spent+clock()-now>clock_t(.060*CLOCKS_PER_SEC))break;
        vector<uint64_t> resets{original_plan.reset};
        if(partial){
            vector<pair<int,int>> homes;
            for(uint64_t z=original_plan.reset;z;z&=z-1){int w=__builtin_ctzll(z);if(moved_home[w])homes.push_back({moved_home[w],w});}
            sort(homes.begin(),homes.end());
            if(homes.size()>=2)for(auto [cost,w]:homes)resets.push_back(original_plan.reset&~(1ULL<<w));
        }
        for(uint64_t reset:resets){
        if(clock()>=end || spent+clock()-now>clock_t(.060*CLOCKS_PER_SEC))break;
        auto z=original_plan;z.reset=reset;bool is_partial=reset!=original_plan.reset;
        partial_tried+=is_partial;''',1)
p=p.replace('''                        ++reached;r.restore_route_state(st);double v=w.answer.size()+r.route_state_value();
                        if(v<score-1e-9){score=v;best=st;win=move(w.answer);}''','''                        ++reached;partial_reached+=is_partial;r.restore_route_state(st);double v=w.answer.size()+r.route_state_value();
                        if(v<score-1e-9){score=v;best=st;win=move(w.answer);best_partial=is_partial;}''',1)
p=p.replace('    r.restore_route_state(best);','    }\n    r.restore_route_state(best);',1)
p=p.replace('    if(!win.empty()){++wins;saved+=int(ops.size())-int(win.size());','    if(!win.empty()){++wins;partial_wins+=best_partial;saved+=int(ops.size())-int(win.size());',1)
s=s[:a]+p+s[b:]
s=s.replace('    cerr<<"PAIR68 "','    cerr<<"PAIR68_PARTIAL tried="<<pair68::partial_tried<<" reached="<<pair68::partial_reached<<" wins="<<pair68::partial_wins<<"\\n";\n    cerr<<"PAIR68 "',1)
t=root/'route_pairpartial.cpp';t.write_text(s,encoding='utf-8')
subprocess.run([sys.executable,str(root/'make_fixture.py'),str(t),str(root/'route_pairpartial_fixture.cpp')],check=True)
print(hashlib.sha256(t.read_bytes()).hexdigest())

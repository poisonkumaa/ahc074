from pathlib import Path
import hashlib,subprocess,sys
root=Path(__file__).resolve().parent
src=root/'alternative_lazy.cpp'
assert hashlib.sha256(src.read_bytes()).hexdigest()=='e201a61d0ffe5aef336d7a5258c7909454d64aab53c37d95faf73248682e45d9'
s=src.read_text(encoding='utf-8');a=s.index('namespace pair68 {');b=s.index('\nnamespace headvalue64',a);p=s[a:b]
old='    auto best=init;double score=ops.size()+r.route_state_value();vector<Op> win;'
new='''    // This post-processing runs after the main learned route comparison.
    // Its alternate core permutation therefore needs its own future-cost term.
    static const double core_weight=search_budget::real("PAIR_CORE_WEIGHT",1.0);
    const bool use_core=core_weight!=0 && r.graph->future_value.size()==size_t(S);
    double initial_core=0;
    if(use_core){
        array<uint8_t,9> current{};int count=0;
        for(int k:init.order)if(k<B)current[count++]=uint8_t(k);
        initial_core=r.graph->future_value[tab.rank(current)];
    }
    auto best=init;double score=ops.size()+r.route_state_value();vector<Op> win;
    if(use_core)score+=core_weight*initial_core;'''
assert p.count(old)==1;p=p.replace(old,new,1)
old='''                        ++reached;r.restore_route_state(st);double v=w.answer.size()+r.route_state_value();
                        if(v<score-1e-9){score=v;best=st;win=move(w.answer);}'''
new='''                        ++reached;r.restore_route_state(st);double v=w.answer.size()+r.route_state_value();
                        if(use_core)v+=core_weight*r.graph->future_value[zc];
                        if(v<score-1e-9){score=v;best=st;win=move(w.answer);}'''
assert p.count(old)==1;p=p.replace(old,new,1)
s=s[:a]+p+s[b:]
t=root/'route_pairvalue.cpp';t.write_text(s,encoding='utf-8')
subprocess.run([sys.executable,str(root/'make_fixture.py'),str(t),str(root/'route_pairvalue_fixture.cpp')],check=True)
print(hashlib.sha256(t.read_bytes()).hexdigest())

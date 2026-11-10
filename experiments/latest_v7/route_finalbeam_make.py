from pathlib import Path
import hashlib,subprocess,sys
p=Path(__file__).resolve().parent
s=(p/'main_baseline.cpp').read_text(encoding='utf-8')
helper=(p/'route_finalbeam.inc').read_text(encoding='utf-8-sig')
assert s.count('int main() {')==1
s=s.replace('int main() {',helper+'\nint main() {',1)
old="        regular_ticks+=clock()-before_route-extra_ticks;\n        cout<<moves.size()<<'\\n';"
new="""        regular_ticks+=clock()-before_route-extra_ticks;
        // Only the last response may leave an arbitrary physical deck.
        // All feedback and reusable-state processing above is already complete.
        if(q==R-1)final_physical_beam::apply(actual,a,trace_start,moves,
            design_started+clock_t(1.94*CLOCKS_PER_SEC));
        cout<<moves.size()<<'\\n';"""
assert s.count(old)==1;s=s.replace(old,new,1)
s=s.replace('    cerr<<"U48 eligible="','    cerr<<"FINAL_BEAM calls="<<final_physical_beam::stats.calls<<" expanded="<<final_physical_beam::stats.expanded<<" proposals="<<final_physical_beam::stats.proposals<<" goals="<<final_physical_beam::stats.goals<<" saved="<<final_physical_beam::stats.saved<<" cpu="<<final_physical_beam::stats.cpu<<"\\n";\n    cerr<<"U48 eligible="',1)
t=p/'route_finalbeam.cpp';t.write_text(s,encoding='utf-8')
subprocess.run([sys.executable,str(p.parent/'beam_v1/make_fixture.py'),str(t),str(p/'route_finalbeam_fixture.cpp')],check=True)
print(hashlib.sha256(t.read_bytes()).hexdigest())

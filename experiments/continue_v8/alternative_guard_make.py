"""Check pair68 time only where a candidate survives cheap rejection.

PAIRS_TIME_GUARD=0: outer plan checks only (original behavior).
PAIRS_TIME_GUARD=1: the previous every-32-core / every-scheduler guard.
PAIRS_TIME_GUARD=2 (default): every 32 surviving cores and 16 schedulers.
Distance rows and their independent verification remain unchanged.
"""
from pathlib import Path
import hashlib
import subprocess
import sys

here = Path(__file__).resolve().parent
source = here / 'alternative_lazy.cpp'
assert hashlib.sha256(source.read_bytes()).hexdigest() == 'e201a61d0ffe5aef336d7a5258c7909454d64aab53c37d95faf73248682e45d9'
s = source.read_text(encoding='utf-8')
begin = s.index('namespace pair68 {')
end = s.index('namespace headvalue64 {', begin)
part = s[begin:end]

def once(old,new):
    global part
    assert part.count(old)==1,(old[:90],part.count(old))
    part=part.replace(old,new,1)

once('''static clock_t distance_ticks=0;
''','''static clock_t distance_ticks=0;
static long long inner_clock_checks=0,guard_survivors=0,guard_schedulers=0;
''')
once('''    static const bool guard=search_budget::integer("PAIRS_TIME_GUARD",1)!=0;''',
     '''    static const int guard_mode=search_budget::integer("PAIRS_TIME_GUARD",2);''')
once('''    bool stopped=false;
''','''    bool stopped=false;unsigned surviving=0,schedulers=0;
''')
once('''        if(!guard)return false;
        const clock_t at=clock();''','''        if(guard_mode==0)return false;
        ++inner_clock_checks;const clock_t at=clock();''')
once('''            if((zc&31)==0 && out_of_time())break;''',
     '''            if(guard_mode==1 && (zc&31)==0 && out_of_time())break;''')
once('''            int n=nt+distance[src*S+zc];if(n>=int(ops.size())||n>18||(!win.empty()&&n>int(win.size())))continue;
''','''            int n=nt+distance[src*S+zc];if(n>=int(ops.size())||n>18||(!win.empty()&&n>int(win.size())))continue;
            ++guard_survivors;
            // Reject almost all core orders using bit masks and counts before
            // paying for a clock call. The original outer-plan guard remains.
            if(guard_mode>=2 && (++surviving&31)==1 && out_of_time())break;
''')
once('''                        if(out_of_time())break;
                        ++goals;fixed_deck::Work w''','''                        ++guard_schedulers;
                        if((guard_mode==1 || (guard_mode>=2 && (++schedulers&15)==1)) && out_of_time())break;
                        ++goals;fixed_deck::Work w''')
s=s[:begin]+part+s[end:]
needle='    cerr<<"PAIR_LAZY '
assert s.count(needle)==1
log='    cerr<<"PAIR_GUARD clocks="<<pair68::inner_clock_checks<<" surviving="<<pair68::guard_survivors<<" schedulers="<<pair68::guard_schedulers<<"\\n";\n'
s=s.replace(needle,log+needle,1)
target=here/'alternative_guard.cpp'
target.write_text(s,encoding='utf-8')
if '--no-fixture' not in sys.argv:
    subprocess.run([sys.executable,str(here/'make_fixture.py'),str(target),str(here/'alternative_guard_fixture.cpp')],check=True)
print(hashlib.sha256(target.read_bytes()).hexdigest())

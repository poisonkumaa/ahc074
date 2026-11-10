"""Enable the complementary one-donor / zero-or-one-backing neighborhood.

SHIFT68_MODE=0 disables it, 1 (default) tries it only when pair68 did not shorten
the current trace, and 2 always tries it after pair68. SHIFT_BUDGET retains the
original .07 native CPU seconds cap. No donor-count or card-count tuning.
"""
from pathlib import Path
import hashlib
import subprocess
import sys

here=Path(__file__).resolve().parent
source=here/'alternative_guard.cpp'
assert hashlib.sha256(source.read_bytes()).hexdigest()=='be37ec58e2de0fd4d05e50fa01bdec8b9e702096aed0df08369ac0515eb82d05'
s=source.read_text(encoding='utf-8')
begin=s.index('namespace shift68 {')
end=s.index('namespace pair68 {',begin)
part=s[begin:end]

def once(old,new):
    global part
    assert part.count(old)==1,(old[:100],part.count(old))
    part=part.replace(old,new,1)

once('''struct Core{array<int,7> ord{};array<uint64_t,8> prefix{};uint64_t color=0;int moved=-1;};''',
     '''static long long guard_stops=0;
struct Core{array<int,7> ord{};array<uint64_t,8> prefix{};uint64_t color=0;int moved=-1;};''')
once('''    if(spent>clock_t(.07*CLOCKS_PER_SEC) || clock()>=end)return;''',
     '''    static const clock_t allowance=clock_t(max(0.,search_budget::real("SHIFT_BUDGET",.07))*CLOCKS_PER_SEC);
    if(spent>=allowance || clock()>=end)return;''')
once('''    clock_t began=clock();++calls;
''','''    clock_t began=clock();++calls;
    const clock_t stop=min(end,began+clock_t(max(0LL,static_cast<long long>(allowance)-spent)));
    unsigned guard_ticks=0;bool timed_out=false;
    auto expired=[&](bool force=false){
        if(timed_out)return true;
        if(force || (++guard_ticks&15)==0){
            if(clock()>=stop){timed_out=true;++guard_stops;}
        }
        return timed_out;
    };
''')
once('''    for(int k:donors){
''','''    for(int k:donors){
        if(expired(true))break;
''')
once('''        for(int v=0;v<nc;v++){
''','''        for(int v=0;v<nc && !timed_out;v++){
            if(expired())break;
''')
once('''            ++goals;
            fixed_deck::Work work''','''            if(expired(true))break;
            ++goals;
            fixed_deck::Work work''')
s=s[:begin]+part+s[end:]
needle='''        pair68::improve(actual,a,trace_start,moves,old68,tail_deadline);
'''
assert s.count(needle)==1
s=s.replace(needle,'''        const size_t before_pair68=moves.size();
        pair68::improve(actual,a,trace_start,moves,old68,tail_deadline);
        static const int shift68_mode=search_budget::integer("SHIFT68_MODE",1);
        if(shift68_mode==2 || (shift68_mode==1 && moves.size()==before_pair68))
            shift68::improve(actual,a,trace_start,moves,old68,tail_deadline);
''',1)
needle='    cerr<<"SHIFT68 '
assert s.count(needle)==1
s=s.replace(needle,'    cerr<<"SHIFT_GUARD stops="<<shift68::guard_stops<<"\\n";\n'+needle,1)
target=here/'alternative_shift.cpp'
target.write_text(s,encoding='utf-8')
if '--no-fixture' not in sys.argv:
    subprocess.run([sys.executable,str(here/'make_fixture.py'),str(target),str(here/'alternative_shift_fixture.cpp')],check=True)
print(hashlib.sha256(target.read_bytes()).hexdigest())

"""Pre-filter pair68 core permutations by their exact distance, preserving order.

PAIRS_FILTER=0 keeps the original full permutation scan. At the default 1,
query-wide bad-color masks and ascending lists for each distance limit avoid
repeating the same impossible core tests for every donor pair. All live bounds,
including improvements found during the scan, remain checked unchanged.
"""
from pathlib import Path
import hashlib
import subprocess
import sys

here=Path(__file__).resolve().parent
source=here/'alternative_guard.cpp'
assert hashlib.sha256(source.read_bytes()).hexdigest()=='be37ec58e2de0fd4d05e50fa01bdec8b9e702096aed0df08369ac0515eb82d05'
s=source.read_text(encoding='utf-8')
begin=s.index('namespace pair68 {')
end=s.index('namespace headvalue64 {',begin)
part=s[begin:end]

def once(old,new):
    global part
    assert part.count(old)==1,(old[:100],part.count(old))
    part=part.replace(old,new,1)

once('''static long long inner_clock_checks=0,guard_survivors=0,guard_schedulers=0;
''','''static long long inner_clock_checks=0,guard_survivors=0,guard_schedulers=0;
static long long filter_queries=0,filter_full=0,filter_selected=0;
''')
once('''    auto best=init;double score=ops.size()+r.route_state_value();vector<Op> win;
''','''    static const bool filter=search_budget::integer("PAIRS_FILTER",1)!=0;
    const uint8_t* core_distance=distance.data()+src*S;
    vector<uint64_t> core_bad;
    array<vector<int>,7> admissible;
    if(filter){
        ++filter_queries;core_bad.resize(S);
        for(int limit=0;limit<=B;limit++)admissible[limit].reserve(S);
        for(int zc=0;zc<S;zc++){
            core_bad[zc]=(cores[zc].color^want)&sh;
            if(!core_bad[zc])continue;
            for(int limit=core_distance[zc];limit<=B;limit++)admissible[limit].push_back(zc);
        }
    }
    auto best=init;double score=ops.size()+r.route_state_value();vector<Op> win;
''')
once('''        for(int zc=0;zc<S && !stopped;zc++){
            if(guard_mode==1 && (zc&31)==0 && out_of_time())break;
            const auto& c=cores[zc];uint64_t bad=(c.color^want)&sh;if(!bad || (bad&z.missing))continue;
            int n=nt+distance[src*S+zc];if(n>=int(ops.size())||n>18||(!win.empty()&&n>int(win.size())))continue;
''','''        int maximum_distance=min(B,min(int(ops.size())-nt-1,18-nt));
        if(!win.empty())maximum_distance=min(maximum_distance,int(win.size())-nt);
        if(filter && maximum_distance<0)continue;
        const vector<int>* ids=filter?&admissible[maximum_distance]:nullptr;
        const int count=ids?int(ids->size()):S;
        filter_full+=S;filter_selected+=count;
        for(int index=0;index<count && !stopped;index++){
            const int zc=ids?(*ids)[index]:index;
            if(guard_mode==1 && (zc&31)==0 && out_of_time())break;
            const auto& c=cores[zc];uint64_t bad=filter?core_bad[zc]:(c.color^want)&sh;if(!bad || (bad&z.missing))continue;
            int n=nt+core_distance[zc];if(n>=int(ops.size())||n>18||(!win.empty()&&n>int(win.size())))continue;
''')
s=s[:begin]+part+s[end:]
needle='    cerr<<"PAIR_GUARD '
assert s.count(needle)==1
log='    cerr<<"PAIR_FILTER queries="<<pair68::filter_queries<<" full="<<pair68::filter_full<<" selected="<<pair68::filter_selected<<"\\n";\n'
s=s.replace(needle,log+needle,1)
target=here/'alternative_filter.cpp'
target.write_text(s,encoding='utf-8')
subprocess.run([sys.executable,str(here/'make_fixture.py'),str(target),str(here/'alternative_filter_fixture.cpp')],check=True)
print(hashlib.sha256(target.read_bytes()).hexdigest())

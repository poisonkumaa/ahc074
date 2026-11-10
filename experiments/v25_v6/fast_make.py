"""Generate exact terminal-feature accelerations from the frozen v25 source.

FAST_TERMINAL=0: original arithmetic and evaluation (default).
FAST_TERMINAL=1: fuse the proxy and terminal-feature scans.
FAST_TERMINAL=2: additionally memoize four exact states within each query.
FAST_TERMINAL_VERIFY=1: recompute the original vector and compare all bits.
"""
from pathlib import Path
import hashlib
import subprocess
import sys

here = Path(__file__).resolve().parent
baseline = here / "main_baseline.cpp"
assert hashlib.sha256(baseline.read_bytes()).hexdigest() == "a9e5cb6b45ee1562c60721be09bde2d0246ad8eeb95585c7721048d74143d7da"
source = baseline.read_text(encoding="utf-8")
source = '''// Enable population count before all standard-library definitions.
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC target("popcnt")
#endif
''' + source


def once(old, new):
    global source
    assert source.count(old) == 1, (old[:100], source.count(old))
    source = source.replace(old, new, 1)


once('''    clock_t core_ticks=0, primary_ticks=0, regular_ticks=0;
    for(int q=0;q<R;q++) {''', '''    clock_t core_ticks=0, primary_ticks=0, regular_ticks=0;
    const int fast_terminal=std::clamp(search_budget::integer("FAST_TERMINAL",0),0,2);
    const bool fast_terminal_verify=search_budget::integer("FAST_TERMINAL_VERIFY",0)!=0;
    struct FastTerminalEntry {
        OnlineRouter::RouteState state;
        array<double,18> values{};
    };
    // State vectors keep their capacity across queries. The live entry count
    // is reset before every request, including after learned-price updates.
    array<FastTerminalEntry,4> fast_terminal_entries;
    long long fast_terminal_evaluated=0,fast_terminal_hits=0,fast_terminal_verified=0;
    for(int q=0;q<R;q++) {''')
once('''        auto compare_route=[&](const OnlineRouter::RouteState& challenger,vector<pair<int,int>>& trace,clock_t limit){''', '''        int fast_terminal_count=0,fast_terminal_replace=0;
        auto compare_route=[&](const OnlineRouter::RouteState& challenger,vector<pair<int,int>>& trace,clock_t limit){''')

begin = source.index('                auto features=[&](const OnlineRouter::RouteState& st,const auto& proxy,double routeval){')
end = source.index('                auto fa=features(first_state,proxy1,first_score-moves.size()),fb=features(other,proxy2,alt_value);', begin)
original_features = source[begin:end]
fused = original_features.replace('auto features=[&](const OnlineRouter::RouteState& st,const auto& proxy,double routeval)',
                                  'auto fused_features=[&](const OnlineRouter::RouteState& st,double routeval)', 1)
fused = fused.replace('''                    array<uint8_t,K> loan{};uint64_t heads=0,claimed=0;''', '''                    array<uint8_t,K> loan{};uint64_t heads=0,credit[2]{};
                    array<int,K> depth{};''', 1)
fused = fused.replace('''                    for(int k:st.supports){int w=actual.paper_bucket[k],cut=st.cuts[k];loan[k]=1;''', '''                    for(int k:st.supports){int w=actual.paper_bucket[k],cut=st.cuts[k];loan[k]=1;
                        if(k>=actual.tot_B)credit[(actual.black[k]>>w)&1]|=1ULL<<w;''', 1)
fused = fused.replace('''                    array<int,64> idx{};array<int,64> fz,fo;fz.fill(-1);fo.fill(-1);''', '''                    if(st.main>=actual.tot_B){int w=actual.paper_bucket[st.main];
                        credit[(actual.black[st.main]>>w)&1]|=1ULL<<w;
                    }
                    array<int,64> idx{};array<int,64> fz,fo;fz.fill(-1);fo.fill(-1);''', 1)
fused = fused.replace('''                        int w=actual.paper_bucket[k],d=idx[w]++;''', '''                        int w=actual.paper_bucket[k],d=idx[w]++;depth[k]=d;''', 1)
fused = fused.replace('''                        v[12+int(actual.prod_to_pair_idx[x]>=0)]+=double(proxy[x])/(P-1);
                        const uint64_t h=actual.index_main_mask&~actual.opaque[actual.prod_to_paper[x]];
                        v[14]+=__builtin_popcountll((heads^actual.A[x])&h)/double(P-1);''', '''                        const int k=actual.prod_to_paper[x];const uint64_t aim=actual.A[x];
                        const uint64_t h=actual.index_main_mask&~actual.opaque[k];
                        const uint64_t gift=(credit[1]&aim)|(credit[0]&~aim);
                        const int proxy_value=depth[k]+__builtin_popcountll((heads^aim)&h&~gift);
                        v[12+int(actual.prod_to_pair_idx[x]>=0)]+=double(proxy_value)/(P-1);
                        v[14]+=__builtin_popcountll((heads^actual.A[x])&h)/double(P-1);''', 1)
assert 'proxy[x]' not in fused and 'depth[k]=d;' in fused and 'credit[(actual.black' in fused

evaluation = '''                auto terminal_features=[&](const OnlineRouter::RouteState& st,double routeval){
                    array<double,18> answer{};int hit=-1;
                    if(fast_terminal>=2)for(int i=0;i<fast_terminal_count;i++){
                        const auto& old=fast_terminal_entries[i].state;
                        // Include position and the ordered support list as well:
                        // both are directly consumed by the original arithmetic.
                        if(old.main==st.main && old.order==st.order && old.position==st.position &&
                           old.supports==st.supports && old.cuts==st.cuts){hit=i;break;}
                    }
                    if(hit>=0){
                        answer=fast_terminal_entries[hit].values;
                        answer[0]=routeval;fast_terminal_hits++;
                    }else{
                        if(fast_terminal)answer=fused_features(st,routeval);
                        else answer=features(st,proxy(st),routeval);
                        fast_terminal_evaluated++;
                        if(fast_terminal>=2){
                            int index=fast_terminal_count<4?fast_terminal_count++:fast_terminal_replace;
                            if(fast_terminal_count==4)fast_terminal_replace=(index+1)%4;
                            fast_terminal_entries[index].state=st;
                            fast_terminal_entries[index].values=answer;
                        }
                    }
                    if(fast_terminal_verify){
                        const auto reference=features(st,proxy(st),routeval);
                        for(int i=0;i<18;i++)if(std::memcmp(&answer[i],&reference[i],sizeof(double))!=0){
                            std::cerr<<"FAST_TERMINAL mismatch query="<<q<<" feature="<<i
                                <<" actual="<<answer[i]<<" reference="<<reference[i]<<"\\n";
                            std::abort();
                        }
                        fast_terminal_verified++;
                    }
                    return answer;
                };
                auto fa=terminal_features(first_state,first_score-moves.size()),fb=terminal_features(other,alt_value);
'''
once('                auto proxy1=proxy(first_state),proxy2=proxy(other);\n', '')
once(original_features, original_features + fused)
once('                auto fa=features(first_state,proxy1,first_score-moves.size()),fb=features(other,proxy2,alt_value);\n', evaluation)
once('    cerr<<"FINAL_DECK saved="<<end_deck_saved<<"\\n";', '''    cerr<<"FAST_TERMINAL mode="<<fast_terminal<<" evaluated="<<fast_terminal_evaluated
        <<" hits="<<fast_terminal_hits<<" verified="<<fast_terminal_verified<<"\\n";
    cerr<<"FINAL_DECK saved="<<end_deck_saved<<"\\n";''')

target = here / "fast_terminal.cpp"
target.write_text(source, encoding="utf-8", newline="\n")
fixture = here / "fast_terminal_fixture.cpp"
subprocess.run([sys.executable, str(here.parent / "beam_v1/make_fixture.py"), str(target), str(fixture)], check=True)
s = fixture.read_text(encoding="utf-8")
assert s.count('#include "fixture_io.inc"') == 1
fixture.write_text(s.replace('#include "fixture_io.inc"', '#include "../beam_v1/fixture_io.inc"'), encoding="utf-8", newline="\n")
print("fast_terminal.cpp", hashlib.sha256(target.read_bytes()).hexdigest())

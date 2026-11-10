from pathlib import Path
import hashlib
import subprocess
import sys

here=Path(__file__).resolve().parent
baseline=here/'main_baseline.cpp'
assert hashlib.sha256(baseline.read_bytes()).hexdigest()=='a9e5cb6b45ee1562c60721be09bde2d0246ad8eeb95585c7721048d74143d7da'
s='''#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC target("popcnt")
#endif
'''+baseline.read_text(encoding='utf-8')
def once(old,new):
 global s
 assert s.count(old)==1,(old[:100],s.count(old))
 s=s.replace(old,new,1)
once('''inline vector<pair<int,int>> optimize(OnlineRouter&router,int product,
                                      const vector<int>&start,vector<pair<int,int>> moves,''','''#include "route_duo.inc"

inline vector<pair<int,int>> optimize(OnlineRouter&router,int product,
                                      const vector<int>&start,vector<pair<int,int>> moves,''')
once('''    int repairs_tried=0;
    bool different_core=false,different_left=false;''','''    int repairs_tried=0;
    paid_duo::Pool duo_pool;
    bool different_core=false,different_left=false;''')
once('''        if(displayed==target)return consider(order,result_count);
        if(result_count>=int(best_moves.size())''','''        if(displayed==target)return consider(order,result_count);
        if(result_count<int(best_moves.size()) && order[0]==target_main)
            duo_pool.offer(router,product,order,result,result_count,displayed^target);
        if(result_count>=int(best_moves.size())''')
once('''    router.restore_route_state(best_state);
    if(best_moves.size()<moves.size()) {''','''    duo_pool.run(router,product,start,local_deadline,max_supports,original_state,best_score,best_state,best_moves);
    router.restore_route_state(best_state);
    if(best_moves.size()<moves.size()) {''')
once('''    cerr<<"FINAL_DECK saved="<<end_deck_saved<<"\\n";''','''    const auto& duo=nov5_relaxed_trace::paid_duo::stats();
    cerr<<"NOV5_DUO offered="<<duo.offered<<" kept="<<duo.kept<<" searched="<<duo.searched
        <<" pairs="<<duo.pairs<<" replays="<<duo.replays<<" legal="<<duo.legal
        <<" projection_failures="<<duo.projection_failures<<" wins="<<duo.wins<<" saved="<<duo.saved
        <<" verified="<<duo.verified<<" cpu="<<duo.cpu<<"\\n";
    cerr<<"FINAL_DECK saved="<<end_deck_saved<<"\\n";''')
target=here/'route_duo.cpp'
target.write_text(s,encoding='utf-8',newline='\n')
fixture=here/'route_duo_fixture.cpp'
subprocess.run([sys.executable,str(here.parent/'beam_v1/make_fixture.py'),str(target),str(fixture)],check=True)
s=fixture.read_text(encoding='utf-8')
assert s.count('#include "fixture_io.inc"')==1
fixture.write_text(s.replace('#include "fixture_io.inc"','#include "../beam_v1/fixture_io.inc"'),encoding='utf-8',newline='\n')
print(target.name,hashlib.sha256(target.read_bytes()).hexdigest())
print('route_duo.inc',hashlib.sha256((here/'route_duo.inc').read_bytes()).hexdigest())

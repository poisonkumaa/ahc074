from pathlib import Path
import subprocess
import sys

here = Path(__file__).resolve().parent
source = (here / 'adaptive_portfolio.cpp').read_text(encoding='utf-8')

def replace_once(before, after):
    global source
    assert source.count(before) == 1, (before[:100], source.count(before))
    source = source.replace(before, after, 1)

replace_once('''        const int tries=clamp(search_budget::integer("PORTFOLIO_TRIES",1),1,16);
        // For one try, excluding the raw winner removes at most one distinct
        // trace. The two lowest scores therefore retain the first alternative.
        // Use exact stable ordering here so an epsilon cutoff cannot discard
        // a slightly better second trace merely because the cap is smaller.
        const int cap=clamp(search_budget::integer("PORTFOLIO_KEEP",tries==1?2:8),1,32);
        const double tolerance=tries==1?0.0:1.e-9;
        if(int(portfolio.size())>=cap && score>=portfolio.back().score-tolerance)return;''', '''        int cap=clamp(search_budget::integer("PORTFOLIO_KEEP",8),1,32);
        if(int(portfolio.size())>=cap && score>=portfolio.back().score-1.e-9)return;''')

replace_once('''    const bool portfolio_configured=search_budget::integer("ROUTE_PORTFOLIO",1)!=0;''', '''    const bool portfolio_configured=search_budget::integer("ROUTE_PORTFOLIO",1)!=0;
    const int portfolio_accept_mode=clamp(search_budget::integer("PORTFOLIO_ACCEPT_MODE",0),0,2);''')

replace_once('''                if(score<best_score-1.e-9) {
                    portfolio_wins++;portfolio_saved+=int(moves.size())-int(shortened.size());''', '''                const bool admissible=portfolio_accept_mode==0 ||
                    (shortened.size()<moves.size() &&
                     (portfolio_accept_mode<2 || actual.active_supports.size()<=best_state.supports.size()));
                if(score<best_score-1.e-9 && admissible) {
                    portfolio_wins++;portfolio_saved+=int(moves.size())-int(shortened.size());''')

replace_once('''    cerr<<"PORTFOLIO configured="<<portfolio_configured<<" trials="<<portfolio_trials''', '''    cerr<<"PORTFOLIO configured="<<portfolio_configured<<" accept_mode="<<portfolio_accept_mode<<" trials="<<portfolio_trials''')

target = here / 'guarded_portfolio.cpp'
target.write_text(source, encoding='utf-8', newline='\n')
fixture = here / 'fixture_guarded.cpp'
subprocess.run([sys.executable, str(here.parent / 'beam_v1' / 'make_fixture.py'), str(target), str(fixture)], check=True)
fixture_source = fixture.read_text(encoding='utf-8')
fixture_source = fixture_source.replace('#include "fixture_io.inc"', '#include "../beam_v1/fixture_io.inc"')
fixture.write_text(fixture_source, encoding='utf-8', newline='\n')
print(target)
print(fixture)

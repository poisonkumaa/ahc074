from pathlib import Path

here = Path(__file__).resolve().parent
source = (here / 'portfolio.cpp').read_text(encoding='utf-8')

def replace_once(before, after):
    global source
    assert source.count(before) == 1, (before[:100], source.count(before))
    source = source.replace(before, after, 1)

replace_once('''        int cap=clamp(search_budget::integer("PORTFOLIO_KEEP",8),1,32);
        if(int(portfolio.size())>=cap && score>=portfolio.back().score-1.e-9)return;''', '''        const int tries=clamp(search_budget::integer("PORTFOLIO_TRIES",1),1,16);
        // For one try, excluding the raw winner removes at most one distinct
        // trace. The two lowest scores therefore retain the first alternative.
        // Use exact stable ordering here so an epsilon cutoff cannot discard
        // a slightly better second trace merely because the cap is smaller.
        const int cap=clamp(search_budget::integer("PORTFOLIO_KEEP",tries==1?2:8),1,32);
        const double tolerance=tries==1?0.0:1.e-9;
        if(int(portfolio.size())>=cap && score>=portfolio.back().score-tolerance)return;''')

replace_once('''    static const double budget=search_budget::seconds(getenv("NOV5_RELAX_BUDGET")?atof(getenv("NOV5_RELAX_BUDGET")):.52);''', '''    static const double budget=search_budget::seconds(getenv("NOV5_RELAX_BUDGET")?atof(getenv("NOV5_RELAX_BUDGET")):
        (search_budget::integer("ROUTE_PORTFOLIO",1)!=0?.9:.52));''')

replace_once('''    actual.portfolio_enabled=search_budget::integer("ROUTE_PORTFOLIO",0)!=0;
    double portfolio_cpu=0;
    int portfolio_trials=0,portfolio_wins=0,portfolio_saved=0;''', '''    const bool portfolio_configured=search_budget::integer("ROUTE_PORTFOLIO",1)!=0;
    actual.portfolio_enabled=false;
    const double portfolio_cap=search_budget::seconds(max(0.0,search_budget::real("PORTFOLIO_CPU",.4)));
    const double portfolio_reserve=max(0.0,search_budget::real("PORTFOLIO_RESERVE",1.10));
    const clock_t portfolio_margin=clock_t(.01*CLOCKS_PER_SEC); // Native CPU seconds.
    double portfolio_cpu=0;
    int portfolio_trials=0,portfolio_wins=0,portfolio_saved=0,portfolio_collected=0;
    int portfolio_warmup_skips=0,portfolio_reserve_skips=0,portfolio_cpu_skips=0,portfolio_late_skips=0;''')

replace_once('''    clock_t core_ticks=0;''', '''    clock_t core_ticks=0,primary_ticks=0;''')

replace_once('''        clock_t before_route=clock();
        auto moves=actual.query(a);''', '''        const clock_t gate_time=clock();
        actual.portfolio_enabled=false;
        if(portfolio_configured) {
            if(q<16)portfolio_warmup_skips++;
            else if(portfolio_cpu>=portfolio_cap)portfolio_cpu_skips++;
            else {
                const clock_t reserve=clock_t(ceil(double(primary_ticks)*(R-q)/q*portfolio_reserve))+portfolio_margin;
                if(actual.route_deadline>gate_time && reserve<actual.route_deadline-gate_time) {
                    actual.portfolio_enabled=true;portfolio_collected++;
                }else portfolio_reserve_skips++;
            }
        }
        clock_t before_route=clock();
        auto moves=actual.query(a);''')

replace_once('''        const auto raw_moves=moves;
        const double raw_score=moves.size()+actual.route_state_value();''', '''        vector<pair<int,int>> raw_moves;
        double raw_score=0;
        if(actual.portfolio_enabled) {
            raw_moves=moves;raw_score=moves.size()+actual.route_state_value();
        }''')

before = '''        if(actual.portfolio_enabled && portfolio_cpu<search_budget::seconds(search_budget::real("PORTFOLIO_CPU",.5))
                && !routing_budget::expired(deadline)) {
            const clock_t began=clock();
            auto best_state=actual.save_route_state();
            double best_score=moves.size()+actual.route_state_value();
            int tried=0;
            const double slack=search_budget::real("PORTFOLIO_SLACK",1.5);
            const int limit=clamp(search_budget::integer("PORTFOLIO_TRIES",2),1,16);
            for(const auto& alternative:actual.portfolio) {
                if(tried>=limit || routing_budget::expired(deadline))break;
                if(alternative.moves==raw_moves || alternative.score>raw_score+slack)continue;
                actual.restore_route_state(alternative.state);
                auto shortened=nov5_relaxed_trace::optimize(actual,a,trace_start,alternative.moves,deadline);
                double score=shortened.size()+actual.route_state_value();
                tried++;portfolio_trials++;
                if(score<best_score-1.e-9) {
                    portfolio_wins++;portfolio_saved+=int(moves.size())-int(shortened.size());
                    best_score=score;best_state=actual.save_route_state();moves=move(shortened);
                }
            }
            actual.restore_route_state(best_state);
            portfolio_cpu+=double(clock()-began)/CLOCKS_PER_SEC;
        }'''

after = '''        // Reserve the observed cost of actual.query plus its incumbent
        // relaxation. Alternative work is excluded from this running mean.
        const clock_t primary_done=clock();
        primary_ticks+=primary_done-before_route;
        const clock_t future_reserve=clock_t(ceil(double(primary_ticks)*(R-1-q)/(q+1)*portfolio_reserve))+portfolio_margin;
        const clock_t reserve_deadline=actual.route_deadline-future_reserve;
        const clock_t cpu_deadline=primary_done+clock_t(max(0.0,portfolio_cap-portfolio_cpu)*CLOCKS_PER_SEC);
        const clock_t extra_deadline=min(deadline,min(reserve_deadline,cpu_deadline));
        if(actual.portfolio_enabled && portfolio_cpu<portfolio_cap && primary_done<extra_deadline) {
            const clock_t began=clock();
            auto best_state=actual.save_route_state();
            double best_score=moves.size()+actual.route_state_value();
            int tried=0;
            const double slack=search_budget::real("PORTFOLIO_SLACK",1.5);
            const int limit=clamp(search_budget::integer("PORTFOLIO_TRIES",1),1,16);
            for(const auto& alternative:actual.portfolio) {
                if(tried>=limit || clock()>=extra_deadline)break;
                if(alternative.moves==raw_moves || alternative.score>raw_score+slack)continue;
                actual.restore_route_state(alternative.state);
                auto shortened=nov5_relaxed_trace::optimize(actual,a,trace_start,alternative.moves,extra_deadline);
                double score=shortened.size()+actual.route_state_value();
                tried++;portfolio_trials++;
                if(score<best_score-1.e-9) {
                    portfolio_wins++;portfolio_saved+=int(moves.size())-int(shortened.size());
                    best_score=score;best_state=actual.save_route_state();moves=move(shortened);
                }
            }
            // Even a timeout or a non-improving last trial leaves the chosen
            // incumbent's exact deck, support set, and cuts installed.
            actual.restore_route_state(best_state);
            portfolio_cpu+=double(clock()-began)/CLOCKS_PER_SEC;
        }else if(actual.portfolio_enabled)portfolio_late_skips++;'''
replace_once(before, after)

replace_once('''    if(actual.portfolio_enabled)cerr<<"PORTFOLIO trials="<<portfolio_trials<<" wins="<<portfolio_wins
        <<" saved="<<portfolio_saved<<" cpu="<<portfolio_cpu<<"\\n";''', '''    cerr<<"PORTFOLIO configured="<<portfolio_configured<<" trials="<<portfolio_trials<<" wins="<<portfolio_wins
        <<" saved="<<portfolio_saved<<" cpu="<<portfolio_cpu<<" primary_cpu="<<double(primary_ticks)/CLOCKS_PER_SEC
        <<" collected="<<portfolio_collected<<" warmup_skips="<<portfolio_warmup_skips
        <<" reserve_skips="<<portfolio_reserve_skips<<" cpu_skips="<<portfolio_cpu_skips
        <<" late_skips="<<portfolio_late_skips<<" reserve_factor="<<portfolio_reserve<<"\\n";''')

target = here / 'adaptive_portfolio.cpp'
target.write_text(source, encoding='utf-8', newline='\n')
print(target)

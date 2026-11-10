from pathlib import Path

root=Path(__file__).resolve().parents[2]
s=(root/'experiments/design_v3/main_before_append.cpp').read_text(encoding='utf-8')
marker='    struct SharedTail{'
extra=r'''
    struct PortfolioEntry {
        RouteState state;
        vector<pair<int,int>> moves;
        double score;
    };
    bool portfolio_enabled=false;
    vector<PortfolioEntry> portfolio;
    void collect_portfolio(const vector<pair<int,int>>& moves) {
        if(!portfolio_enabled)return;
        double score=moves.size()+route_state_value();
        int cap=clamp(search_budget::integer("PORTFOLIO_KEEP",8),1,32);
        if(int(portfolio.size())>=cap && score>=portfolio.back().score-1.e-9)return;
        for(const auto& p:portfolio)if(p.moves==moves)return;
        auto it=upper_bound(portfolio.begin(),portfolio.end(),score,
            [](double x,const PortfolioEntry& p){return x<p.score;});
        portfolio.insert(it,PortfolioEntry{save_route_state(),moves,score});
        if(int(portfolio.size())>cap)portfolio.pop_back();
    }
'''
assert s.count(marker)==1
s=s.replace(marker,extra+marker)
start=s.index('    vector<pair<int,int>> query(int a) {')
end=s.index('    // Rank joint destinations',start)
q=s[start:end]
q=q.replace('        used_tails=0;','        portfolio.clear();\n        used_tails=0;',1)
for name in ['moves','alternative','alt']:
    for lhs in ['double score=','double sc=']:
        pattern=lhs+name+'.size()+route_state_value();'
        q=q.replace(pattern,pattern+'collect_portfolio('+name+');')
assert q.count('collect_portfolio(')==7,q.count('collect_portfolio(')
s=s[:start]+q+s[end:]
marker='    actual.route_branching=true;'
s=s.replace(marker,marker+r'''
    actual.portfolio_enabled=search_budget::integer("ROUTE_PORTFOLIO",0)!=0;
    double portfolio_cpu=0;
    int portfolio_trials=0,portfolio_wins=0,portfolio_saved=0;
''')
marker='        moves=nov5_relaxed_trace::optimize(actual,a,trace_start,move(moves),deadline);'
assert s.count(marker)==1
s=s.replace(marker,r'''
        const auto raw_moves=moves;
        const double raw_score=moves.size()+actual.route_state_value();
'''+marker+r'''
        if(actual.portfolio_enabled && portfolio_cpu<search_budget::seconds(search_budget::real("PORTFOLIO_CPU",.5))
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
        }
''')
marker='    const auto &bc=routing_budget::counters();'
s=s.replace(marker,r'''
    if(actual.portfolio_enabled)cerr<<"PORTFOLIO trials="<<portfolio_trials<<" wins="<<portfolio_wins
        <<" saved="<<portfolio_saved<<" cpu="<<portfolio_cpu<<"\n";
'''+marker)
(root/'experiments/design_v3/portfolio.cpp').write_text(s,encoding='utf-8')

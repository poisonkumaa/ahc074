from pathlib import Path
root=Path(__file__).resolve().parent
s=(root/'main_baseline.cpp').read_text(encoding='utf-8')
needle='''    int requested=max(256,search_budget::integer("NOV8_QUERIES",8192)), previous=-1, synthetic_queries=0;'''
replace='''    // Calibrate the raw design race by matched shortening experiments.
    // The real race states remain under the original policy; this estimates
    // how much of each layout's raw cost is removable by the live shortener.
    const double race_short_weight=search_budget::real("RACE_SHORT_WEIGHT",1.0);
    const int race_short_stride=max(1,search_budget::integer("RACE_SHORT_STRIDE",8));
    const double race_short_prior=max(0.,search_budget::real("RACE_SHORT_PRIOR",8.0));
    const double race_short_budget=search_budget::seconds(search_budget::real("RACE_SHORT_BUDGET",.10));
    vector<double> race_saved(candidates.size()),race_saving(candidates.size());
    vector<vector<double>> race_square(candidates.size(),vector<double>(candidates.size()));
    vector<vector<int>> race_counts(candidates.size(),vector<int>(candidates.size()));
    vector<vector<double>> race_pair_sum(candidates.size(),vector<double>(candidates.size()));
    vector<int> race_samples(candidates.size());
    vector<vector<int>> race_start(candidates.size());
    vector<vector<pair<int,int>>> race_moves(candidates.size());
    double race_cpu=0;int race_batches=0,race_aborts=0;
    auto race_value=[&](int i,int count){
        return scores[i]-race_short_weight*count*race_saved[i]/max(1.,race_samples[i]+race_short_prior);
    };
    int requested=max(256,search_budget::integer("NOV8_QUERIES",8192)), previous=-1, synthetic_queries=0;'''
assert s.count(needle)==1;s=s.replace(needle,replace,1)
needle='''        const int last=previous;
        previous=a;
        for(int i=0;i<(int)candidates.size();i++) {'''
replace='''        const int last=previous;
        previous=a;
        const bool sample_short=race_short_weight!=0 && q%race_short_stride==race_short_stride-1 &&
            race_cpu<race_short_budget && design_elapsed()<search_budget::real("NOV8_SELECT_END",2.18);
        for(int i=0;i<(int)candidates.size();i++) {'''
assert s.count(needle)==1;s=s.replace(needle,replace,1)
needle='''            auto moves=trials[i]->query(a);
            scores[i]+=moves.size();'''
replace='''            if(sample_short)race_start[i]=trials[i]->sim.order;
            auto moves=trials[i]->query(a);
            if(sample_short)race_moves[i]=moves;
            scores[i]+=moves.size();'''
assert s.count(needle)==1;s=s.replace(needle,replace,1)
needle='''        for(int i=0;i<int(trials.size());i++)if(feasible[i])
            for(int j=0;j<i;j++)if(feasible[j]){
                double delta=one[j]-one[i];variance[i][j]+=delta*delta;
            }'''
replace='''        if(sample_short){
            const clock_t began=clock();
            const clock_t end=min(absolute_deadline(search_budget::real("NOV8_SELECT_END",2.18)),
                began+clock_t(max(0.,race_short_budget-race_cpu)*CLOCKS_PER_SEC));
            int alive=0;for(bool f:feasible)alive+=f;
            bool complete=alive>=2 && began<end;
            const auto saved_trace_stats=nov5_relaxed_trace::stats();
            const auto saved_route_stats=routing_budget::counters();
            for(int i=0;i<int(trials.size()) && complete;i++)if(feasible[i]){
                if(clock()>=end){complete=false;break;}
                auto& router=*trials[i];auto raw_state=router.save_route_state();
                nov5_relaxed_trace::stats()={};
                auto shortened=nov5_relaxed_trace::optimize(router,a,race_start[i],race_moves[i],end);
                const bool valid=router.sim.current_display()==router.A[a] && shortened.size()<=race_moves[i].size();
                router.restore_route_state(raw_state);
                race_saving[i]=int(race_moves[i].size())-int(shortened.size());
                if(!valid || clock()>=end)complete=false;
            }
            nov5_relaxed_trace::stats()=saved_trace_stats;
            routing_budget::counters()=saved_route_stats;
            race_cpu+=double(clock()-began)/CLOCKS_PER_SEC;
            // Partial batches would favor candidates processed before timeout.
            // Commit only after every still-active layout received its test.
            if(complete){
                ++race_batches;
                for(int i=0;i<int(trials.size());i++)if(feasible[i]){
                    race_saved[i]+=race_saving[i];++race_samples[i];
                    for(int j=0;j<i;j++)if(feasible[j]){
                        double d=race_saving[i]-race_saving[j];
                        race_square[i][j]+=d*d;race_pair_sum[i][j]+=d;++race_counts[i][j];
                    }
                }
            }else ++race_aborts;
        }
        for(int i=0;i<int(trials.size());i++)if(feasible[i])
            for(int j=0;j<i;j++)if(feasible[j]){
                double delta=one[j]-one[i];variance[i][j]+=delta*delta;
            }'''
assert s.count(needle)==1;s=s.replace(needle,replace,1)
needle='''                if(feasible[i] && (best<0 || scores[i]<scores[best]))best=i;'''
replace='''                if(feasible[i] && (best<0 || race_value(i,q+1)<race_value(best,q+1)))best=i;'''
assert s.count(needle)==1;s=s.replace(needle,replace,1)
needle='''                double delta=scores[i]-scores[best];
                double sum=variance[max(best,i)][min(best,i)];
                double threshold=1.4*sqrt(max(0.,sum-delta*delta/(q+1)));'''
replace='''                const int u=max(best,i),v=min(best,i);
                double raw_delta=scores[i]-scores[best],delta=race_value(i,q+1)-race_value(best,q+1);
                double sum=variance[u][v];
                double uncertainty=max(0.,sum-raw_delta*raw_delta/(q+1));
                int n=race_counts[u][v];
                if(n>1){
                    double centered=max(0.,race_square[u][v]-race_pair_sum[u][v]*race_pair_sum[u][v]/n);
                    double scale=race_short_weight*(q+1)/(n+race_short_prior);
                    uncertainty+=scale*scale*centered;
                }
                double threshold=1.4*sqrt(uncertainty);'''
assert s.count(needle)==1;s=s.replace(needle,replace,1)
needle='''        if(feasible[i] && (!feasible[selected] || scores[i]<scores[selected]))selected=i;
    }
    if(!feasible[selected])return fallback();'''
replace='''        if(feasible[i] && (!feasible[selected] || race_value(i,synthetic_queries)<race_value(selected,synthetic_queries)))selected=i;
    }
    cerr<<"RACE_SHORT batches="<<race_batches<<" aborts="<<race_aborts<<" cpu="<<race_cpu<<" chosen="<<selected;
    for(int i=0;i<int(candidates.size());i++)cerr<<" candidate"<<i<<"="<<scores[i]<<","<<race_samples[i]<<","<<race_saved[i]<<","<<race_value(i,synthetic_queries);
    cerr<<"\\n";
    if(!feasible[selected])return fallback();'''
assert s.count(needle)==1;s=s.replace(needle,replace,1)
(root/'route_race.cpp').write_text(s,encoding='utf-8')

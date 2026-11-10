struct Search {
    const bool enabled=search_budget::integer("ROUTE_CRITIC_ROLLOUT",0)!=0;
    static constexpr int samples=12,max_contests=32;
    static constexpr double native_cap=.04,margin=.05;
    clock_t ticks=0;
    int contests=0,completed=0,aborted=0,flips=0,trials=0;
    double sum_difference=0;
    unique_ptr<OnlineRouter> scratch;

    bool compare(OnlineRouter& live,int current,int round,
                 const OnlineRouter::RouteState& first,
                 const OnlineRouter::RouteState& other,
                 clock_t absolute_stop,bool fallback) {
        if(!enabled || contests>=max_contests)return fallback;
        const clock_t cap=clock_t(native_cap*CLOCKS_PER_SEC),started=clock();
        if(ticks>=cap || (absolute_stop>0 && started>=absolute_stop))return fallback;
        clock_t stop=started+cap-ticks;
        if(absolute_stop>0)stop=min(stop,absolute_stop);
        // Mutable state and all route caches belong to scratch. Shared cache
        // entries are immutable; restore the shared diagnostic counters too.
        struct Scope {
            clock_t& elapsed;clock_t started;
            routing_budget::Counters routing;
            requirement_distance_cache::Statistics requirements;
            ~Scope(){
                elapsed+=clock()-started;
                routing_budget::counters()=routing;
                requirement_distance_cache::statistics()=requirements;
            }
        } scope{ticks,started,routing_budget::counters(),requirement_distance_cache::statistics()};
        ++contests;
        if(!scratch) {
            scratch=make_unique<OnlineRouter>(live.design,live.A,max(clock_t(1),clock()-1));
            scratch->route_branching=false;scratch->route_policies=1;
            scratch->save_second=false;scratch->post_support_choices=1;
            scratch->joint_extract_mode=0;scratch->nov9_insert_enabled=false;
            scratch->bucket_mdp_weight=0;scratch->bucket_mdp_tables.clear();
        }
        if(clock()>=stop){++aborted;return fallback;}
        auto& worker=*scratch;
        worker.hole_request_count=live.hole_request_count;
        worker.support_utility=live.support_utility;
        worker.support_order_weight=live.support_order_weight;
        worker.route_queue_weight=live.route_queue_weight;
        worker.route_drop_weight=live.route_drop_weight;
        worker.route_drop_model=live.route_drop_model;
        worker.support_drop=live.support_drop;
        worker.front_support_enabled=live.front_support_enabled;
        worker.joint_extract_support_weight=live.joint_extract_support_weight;
        if(live.graph && worker.graph)worker.graph->future_value=live.graph->future_value;

        // The same twelve unique catalog IDs are used for both states. The
        // seed uses only the known current request, its index and the catalog.
        array<int,samples> requests{};
        uint64_t random=live.A[current]^live.A[0]^0x726f6c6c6f757436ULL^
            (uint64_t(round+1)*0x9e3779b97f4a7c15ULL);
        if(!random)random=1;
        for(int i=0;i<samples;i++)for(;;){
            random^=random<<7;random^=random>>9;
            int next=int(random%P);bool duplicate=next==current;
            for(int j=0;j<i;j++)duplicate|=requests[j]==next;
            if(!duplicate){requests[i]=next;break;}
        }
        double difference=0;
        for(int j=0;j<samples;j++) {
            double cost[2]{};
            for(int side=0;side<2;side++) {
                if(clock()>=stop){++aborted;return fallback;}
                worker.restore_route_state(side?other:first);
                worker.set_route_policy(0);
                worker.route_deadline=stop;
                worker.return_alternative=-1;worker.return_front=false;
                worker.add_backing=false;worker.stay_main=false;
                worker.keep_sources=worker.park_sources=0;worker.extra_source=-1;
                worker.choice_rank=worker.choice_count=0;
                worker.late_donor=worker.late_depth=-1;
                worker.post_support_rank=worker.post_first_option_count=0;
                worker.used_tails=0;worker.tails.clear();worker.batch_sources.clear();
                worker.batch_choice=0;worker.batch_failed=false;
                worker.joint_extract_cache.clear();worker.nov9_insert_positions.clear();
                worker.valued_shape.fill(0);
                if(worker.graph){worker.graph->last_route_product=-1;worker.graph->last_route_moves.clear();}
                const int next=requests[j];
                auto moves=worker.query(next);++trials;
                if(worker.batch_failed || moves.size()>2*K || worker.sim.current_display()!=live.A[next]){
                    ++aborted;return fallback;
                }
                if(clock()>=stop){++aborted;return fallback;}
                auto terminal=worker.save_route_state();
                cost[side]=moves.size()+terminal_value(worker,terminal,next,worker.route_state_value());
            }
            difference+=cost[1]-cost[0];
        }
        if(clock()>=stop){++aborted;return fallback;}
        difference/=samples;sum_difference+=difference;++completed;
        bool choose=fallback;
        if(difference< -margin)choose=true;
        if(difference> margin)choose=false;
        flips+=choose!=fallback;
        return choose;
    }
    void report()const{
        cerr<<"ROUTE_CRITIC enabled="<<enabled<<" contests="<<contests<<" completed="<<completed
            <<" aborted="<<aborted<<" trials="<<trials<<" flips="<<flips
            <<" difference_sum="<<sum_difference<<" cpu="<<double(ticks)/CLOCKS_PER_SEC<<"\n";
    }
};

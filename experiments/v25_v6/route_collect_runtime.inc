// Research-only collector. The generated executable always loads fixed layouts.
// Queue snapshots while answering requests; evaluate independent catalog-only
// continuations after the live run, so sampling CPU cannot consume live quota.
namespace route_collect {
struct Sample {
    int round,current,length_delta;
    array<double,18> features;
    OnlineRouter::RouteState first,other;
    vector<array<int,2>> hole_counts;
    vector<array<double,32>> utility;
};
struct Collector {
    const bool enabled=search_budget::integer("CRITIC_COLLECT",0)!=0;
    vector<Sample> pending;
    int last_round=-1,emitted=0,aborted=0,queries=0;
    double cpu=0;
    void capture(OnlineRouter& live,int current,int round,int lengths,
                 const OnlineRouter::RouteState& first,const OnlineRouter::RouteState& other,
                 const array<double,18>& fa,const array<double,18>& fb) {
        if(!enabled || round%8!=3 || round==last_round || pending.size()>=128)return;
        last_round=round;
        Sample sample;sample.round=round;sample.current=current;sample.length_delta=lengths;
        for(int i=0;i<18;i++)sample.features[i]=fb[i]-fa[i];
        sample.first=first;sample.other=other;
        sample.hole_counts=live.hole_request_count;sample.utility=live.support_utility;
        pending.push_back(move(sample));
    }
    static void reset_query(OnlineRouter& worker,clock_t deadline) {
        worker.route_branching=true;worker.save_second=false;worker.route_deadline=deadline;
        worker.set_route_policy(0);worker.return_alternative=-1;worker.return_front=false;
        worker.add_backing=false;worker.stay_main=false;
        worker.keep_sources=worker.park_sources=0;worker.extra_source=-1;
        worker.choice_rank=worker.choice_count=0;worker.late_donor=worker.late_depth=-1;
        worker.post_support_rank=worker.post_first_option_count=0;
        worker.used_tails=0;worker.tails.clear();worker.batch_sources.clear();
        worker.batch_choice=0;worker.batch_failed=false;
        worker.nov9_insert_positions.clear();worker.joint_extract_cache.clear();worker.valued_shape.fill(0);
        if(worker.graph){worker.graph->last_route_product=-1;worker.graph->last_route_moves.clear();}
    }
    static bool verify(const OnlineRouter& worker,const vector<int>& start,
                       const vector<pair<int,int>>& moves,int product) {
        if(moves.size()>2*K)return false;
        auto order=start;
        for(auto [paper,depth]:moves) {
            if(paper<0 || paper>=K || depth<0 || depth>=K)return false;
            auto it=find(order.begin(),order.end(),paper);
            if(it==order.end())return false;
            uint64_t visible=worker.opaque[paper];
            for(auto p=order.begin();p!=it;p++)visible&=~worker.opaque[*p];
            if(!visible)return false;
            order.erase(it);order.insert(order.begin()+depth,paper);
        }
        return order==worker.sim.order && worker.sim.current_display()==worker.A[product];
    }
    void flush(OnlineRouter& live) {
        if(!enabled || pending.empty())return;
        const clock_t began=clock(),deadline=began+clock_t(60.0*CLOCKS_PER_SEC);
        struct Scope {
            routing_budget::Counters routing=routing_budget::counters();
            requirement_distance_cache::Statistics requirements=requirement_distance_cache::statistics();
            nov5_relaxed_trace::Statistics shortening=nov5_relaxed_trace::stats();
            ~Scope(){routing_budget::counters()=routing;requirement_distance_cache::statistics()=requirements;
                     nov5_relaxed_trace::stats()=shortening;}
        } scope;
        OnlineRouter worker(live.design,live.A,max(clock_t(1),clock()-1));
        worker.route_policies=live.route_policies;
        worker.post_support_choices=live.post_support_choices;
        worker.joint_extract_mode=live.joint_extract_mode;
        worker.joint_extract_support_weight=live.joint_extract_support_weight;
        worker.nov9_insert_enabled=live.nov9_insert_enabled;
        worker.front_support_enabled=live.front_support_enabled;
        worker.support_order_weight=live.support_order_weight;
        worker.route_queue_weight=live.route_queue_weight;worker.route_drop_weight=live.route_drop_weight;
        worker.route_drop_model=live.route_drop_model;worker.support_drop=live.support_drop;
        worker.bucket_mdp_weight=live.bucket_mdp_weight;
        worker.bucket_mdp_projection_cap=live.bucket_mdp_projection_cap;
        worker.bucket_mdp_projected=live.bucket_mdp_projected;
        worker.bucket_mdp_paper_type=live.bucket_mdp_paper_type;
        worker.bucket_mdp_tables.resize(live.bucket_mdp_tables.size());
        for(size_t i=0;i<live.bucket_mdp_tables.size();i++)if(live.bucket_mdp_tables[i])
            worker.bucket_mdp_tables[i]=make_unique<bucket_mdp::Model>(*live.bucket_mdp_tables[i]);
        if(live.graph && worker.graph)worker.graph->future_value=live.graph->future_value;
        const int count=min(64,int(pending.size()));
        const auto old_precision=cerr.precision();cerr.precision(17);
        for(int row=0;row<count;row++) {
            if(clock()>=deadline){++aborted;break;}
            // Midpoints select up to64 samples across the complete case, not
            // the first64 eligible queries. Each source query appears once.
            const int id=int((2LL*row+1)*pending.size()/(2*count));
            const auto& sample=pending[id];
            worker.hole_request_count=sample.hole_counts;worker.support_utility=sample.utility;
            array<int,8> request{};
            uint64_t random=live.A[sample.current]^live.A[0]^0x636f6c6c65637436ULL^
                (uint64_t(sample.round+1)*0x9e3779b97f4a7c15ULL);
            if(!random)random=1;
            int previous=sample.current;
            for(int j=0;j<8;j++) {
                random^=random<<7;random^=random>>9;
                int next=int(random%(P-1));next+=next>=previous;
                request[j]=previous=next;
            }
            int cost[2]{};bool valid=true;
            for(int side=0;side<2 && valid;side++) {
                worker.restore_route_state(side?sample.other:sample.first);
                for(int next:request) {
                    if(clock()>=deadline){valid=false;break;}
                    reset_query(worker,deadline);
                    const auto start=worker.sim.order;
                    auto moves=worker.query(next);++queries;
                    if(worker.batch_failed || clock()>=deadline){valid=false;break;}
                    // Every hypothetical request gets an independent primary
                    // shortening allowance; its repeated calls share one quota.
                    nov5_relaxed_trace::stats()={};
                    int old_length;
                    do {
                        old_length=int(moves.size());
                        moves=nov5_relaxed_trace::optimize(worker,next,start,move(moves),deadline);
                    } while(int(moves.size())<old_length && clock()<deadline);
                    if(clock()>=deadline || !verify(worker,start,moves,next)){valid=false;break;}
                    cost[side]+=int(moves.size());
                }
            }
            if(!valid){++aborted;if(clock()>=deadline)break;continue;}
            cerr<<"CRITIC_SAMPLE "<<sample.round<<' '<<sample.current<<' '<<sample.length_delta;
            for(double x:sample.features)cerr<<' '<<x;
            cerr<<' '<<cost[1]-cost[0]<<'\n';++emitted;
        }
        cerr.precision(old_precision);
        cpu=double(clock()-began)/CLOCKS_PER_SEC;
        cerr<<"CRITIC_COLLECT queued="<<pending.size()<<" emitted="<<emitted<<" aborted="<<aborted
            <<" queries="<<queries<<" cpu="<<cpu<<" future_steps=8\n";
    }
};
} // namespace route_collect

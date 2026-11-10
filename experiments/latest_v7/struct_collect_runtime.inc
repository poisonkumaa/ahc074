// Research-only collector. Snapshot disclosed states during live play, then
// evaluate paired catalog-only continuations after all real orders are answered.
namespace route_collect {
struct Sample {
    int round,current,length_delta;
    array<double,18> features;
    OnlineRouter::RouteState first,other;
    vector<int> start;
    vector<pair<int,int>> first_trace,other_trace;
    vector<array<int,2>> hole_counts;
    vector<array<double,32>> utility;
    vector<double> core_future;
};
struct Scope {
    routing_budget::Counters routing=routing_budget::counters();
    requirement_distance_cache::Statistics requirements=requirement_distance_cache::statistics();
    nov5_relaxed_trace::Statistics shortening=nov5_relaxed_trace::stats();
    array<int,11> tail={loan_tail::wins,loan_tail::saved,loan_tail::vetos,
        loan_tail::by_type[0],loan_tail::by_type[1],loan_tail::by_type[2],loan_tail::by_type[3],
        loan_tail::savings[0],loan_tail::savings[1],loan_tail::savings[2],loan_tail::savings[3]};
    double tail_cpu=loan_tail::cpu;
    array<long long,14> shelf={return_pair53::eligible,return_pair53::edited,return_pair53::rejected,
        ordinary54::proposed,ordinary54::accepted,ordinary54::rejected,ordinary54::changed_head,
        shelf58::calls,shelf58::eligible,shelf58::tried,shelf58::changed,
        shelf58::loanhomes,shelf58::two,shelf58::visfail};
    clock_t shelf_spent=shelf58::spent;
    ~Scope(){
        routing_budget::counters()=routing;requirement_distance_cache::statistics()=requirements;
        nov5_relaxed_trace::stats()=shortening;
        loan_tail::wins=tail[0];loan_tail::saved=tail[1];loan_tail::vetos=tail[2];
        for(int i=0;i<4;i++){loan_tail::by_type[i]=tail[3+i];loan_tail::savings[i]=tail[7+i];}
        loan_tail::cpu=tail_cpu;
        return_pair53::eligible=shelf[0];return_pair53::edited=shelf[1];return_pair53::rejected=shelf[2];
        ordinary54::proposed=shelf[3];ordinary54::accepted=shelf[4];
        ordinary54::rejected=shelf[5];ordinary54::changed_head=shelf[6];
        shelf58::calls=shelf[7];shelf58::eligible=shelf[8];shelf58::tried=shelf[9];
        shelf58::changed=shelf[10];shelf58::loanhomes=shelf[11];shelf58::two=shelf[12];shelf58::visfail=shelf[13];
        shelf58::spent=shelf_spent;
    }
};
struct Collector {
    const bool enabled=search_budget::integer("CRITIC_COLLECT",0)!=0;
    vector<Sample> pending;
    int last_round=-1,emitted=0,aborted=0,queries=0,initial_edits=0;
    double cpu=0;
    void capture(OnlineRouter& live,int current,int round,int lengths,
                 const OnlineRouter::RouteState& first,const OnlineRouter::RouteState& other,
                 const array<double,18>& fa,const array<double,18>& fb,const vector<int>& start,
                 const vector<pair<int,int>>& first_trace,const vector<pair<int,int>>& other_trace){
        if(!enabled || round%8!=3 || round==last_round || pending.size()>=128)return;
        last_round=round;
        Sample sample;sample.round=round;sample.current=current;sample.length_delta=lengths;
        for(int i=0;i<18;i++)sample.features[i]=fb[i]-fa[i];
        sample.first=first;sample.other=other;sample.start=start;
        sample.first_trace=first_trace;sample.other_trace=other_trace;
        sample.hole_counts=live.hole_request_count;sample.utility=live.support_utility;
        if(live.graph)sample.core_future=live.graph->future_value;
        pending.push_back(move(sample));
    }
    static void reset_query(OnlineRouter& worker,clock_t deadline){
        worker.route_branching=true;worker.save_second=false;worker.route_deadline=deadline;
        worker.set_route_policy(0);worker.return_alternative=-1;worker.return_front=false;
        worker.add_backing=false;worker.stay_main=false;
        worker.keep_sources=worker.park_sources=0;worker.extra_source=-1;
        worker.choice_rank=worker.choice_count=0;worker.late_donor=worker.late_depth=-1;
        worker.post_support_rank=worker.post_first_option_count=0;
        worker.used_tails=0;worker.tails.clear();worker.batch_sources.clear();
        worker.batch_choice=0;worker.batch_failed=false;
        worker.test_duo47=false;worker.extra_core47=worker.core_destination47=-1;worker.duo47={};
        worker.nov9_insert_positions.clear();worker.joint_extract_cache.clear();worker.valued_shape.fill(0);
        if(worker.graph){worker.graph->last_route_product=-1;worker.graph->last_route_moves.clear();}
        // Hypothetical queries have their own optional-work quota. One side's
        // search must not exhaust the allowance available to the paired side.
        nov5_relaxed_trace::stats()={};shelf58::spent=0;
    }
    static bool verify(const OnlineRouter& worker,const vector<int>& start,
                       const vector<pair<int,int>>& moves,int product){
        if(moves.size()>2*K)return false;
        auto order=start;
        for(auto [paper,depth]:moves){
            if(paper<0 || paper>=K || depth<0 || depth>=K)return false;
            auto it=find(order.begin(),order.end(),paper);
            if(it==order.end())return false;
            uint64_t visible=worker.opaque[paper];
            for(auto p=order.begin();p!=it && visible;p++)visible&=~worker.opaque[*p];
            if(!visible)return false;
            order.erase(it);order.insert(order.begin()+depth,paper);
        }
        if(order!=worker.sim.order || worker.active_main!=worker.prod_to_paper[product])return false;
        for(int i=0;i<K;i++)if(worker.sim.position[order[i]]!=i)return false;
        return worker.sim.current_display()==worker.A[product];
    }
    static int normalize_paid(OnlineRouter& worker,const vector<int>& start,
                              vector<pair<int,int>>& moves,int product){
        int edits=0;
        edits+=return_pair53::adjust(worker,start,moves,product);
        edits+=ordinary54::apply(worker,start,moves,product);
        edits+=shelf58::apply(worker,start,moves,product);
        return edits;
    }
    void flush(OnlineRouter& live){
        if(!enabled || pending.empty())return;
        const clock_t began=clock(),deadline=began+clock_t(60.0*CLOCKS_PER_SEC);
        Scope scope;
        OnlineRouter worker(live.design,live.A,max(clock_t(1),clock()-1));
        worker.route_policies=live.route_policies;worker.post_support_choices=live.post_support_choices;
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
        if(live.graph && worker.graph){
            worker.graph->req=live.graph->req;
            worker.graph->future_value=live.graph->future_value;
        }
        const int count=min(64,int(pending.size()));
        const int streams=clamp(search_budget::integer("CRITIC_STREAMS",4),1,16);
        const auto old_precision=cerr.precision();cerr.precision(17);
        for(int row=0;row<count;row++){
            if(clock()>=deadline){++aborted;break;}
            const int id=int((2LL*row+1)*pending.size()/(2*count));
            const auto& sample=pending[id];
            worker.hole_request_count=sample.hole_counts;worker.support_utility=sample.utility;
            if(worker.graph)worker.graph->future_value=sample.core_future;
            double target=0;bool valid=true;
            for(int stream=0;stream<streams && valid;stream++){
            array<int,8> request{};
            uint64_t random=live.A[sample.current]^live.A[0]^0x636f6c6c65637436ULL^
                (uint64_t(sample.round+1)*0x9e3779b97f4a7c15ULL)^
                (uint64_t(stream)*0xd1b54a32d192ed03ULL);
            if(!random)random=1;
            int previous=sample.current;
            for(int j=0;j<8;j++){
                random^=random<<7;random^=random>>9;
                int next=int(random%(P-1));next+=next>=previous;
                request[j]=previous=next;
            }
            int cost[2]{};
            for(int side=0;side<2 && valid;side++){
                worker.restore_route_state(side?sample.other:sample.first);
                reset_query(worker,deadline);
                auto current_trace=side?sample.other_trace:sample.first_trace;
                // The captured critic features precede paid-placement edits.
                // Apply those edits before future costs, without changing the
                // length_delta already present in the training record.
                const size_t current_length=current_trace.size();
                initial_edits+=normalize_paid(worker,sample.start,current_trace,sample.current);
                if(current_trace.size()!=current_length || !verify(worker,sample.start,current_trace,sample.current)){
                    valid=false;break;
                }
                for(int next:request){
                    if(clock()>=deadline){valid=false;break;}
                    reset_query(worker,deadline);
                    const auto start=worker.sim.order;
                    auto moves=worker.query(next);++queries;
                    // A rejected last policy can leave batch_failed set even
                    // though query() restored a different valid winning route.
                    // Validate the returned winner itself before shortening.
                    if(clock()>=deadline || !verify(worker,start,moves,next)){valid=false;break;}
                    int old_length;
                    do{
                        old_length=int(moves.size());
                        moves=nov5_relaxed_trace::optimize(worker,next,start,move(moves),deadline);
                    }while(int(moves.size())<old_length && clock()<deadline);
                    moves=trace_shortener::optimize(start,move(moves),worker.opaque,deadline);
                    if(clock()<deadline)settle_loans(worker,next,start,moves);
                    if(clock()<deadline)fixed_deck::optimize(worker,start,moves);
                    if(clock()<deadline)loan_tail::improve(worker,next,start,moves,deadline);
                    if(clock()<deadline)normalize_paid(worker,start,moves,next);
                    if(clock()>=deadline || !verify(worker,start,moves,next)){valid=false;break;}
                    cost[side]+=int(moves.size());
                }
            }
            target+=cost[1]-cost[0];
            }
            if(!valid){
                ++aborted;
                cerr<<"CRITIC_INVALID round="<<sample.round<<" timeout="<<(clock()>=deadline)<<"\n";
                if(clock()>=deadline)break;
                continue;
            }
            cerr<<"CRITIC_SAMPLE "<<sample.round<<' '<<sample.current<<' '<<sample.length_delta;
            for(double x:sample.features)cerr<<' '<<x;
            cerr<<' '<<target/streams<<'\n';++emitted;
        }
        cerr.precision(old_precision);
        cpu=double(clock()-began)/CLOCKS_PER_SEC;
        cerr<<"CRITIC_COLLECT queued="<<pending.size()<<" emitted="<<emitted<<" aborted="<<aborted
            <<" queries="<<queries<<" initial_edits="<<initial_edits<<" cpu="<<cpu
            <<" future_steps=8 streams="<<streams<<"\n";
    }
};
} // namespace route_collect

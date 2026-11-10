// A second race compares the finalized templates and the actual routing policy.
// All orders are generated from the public catalogue; no reactive input is read.
namespace finalist_selection {
static uint64_t fingerprint(const Design& d) {
    uint64_t h=1469598103934665603ULL;
    auto byte=[&](unsigned x){h^=x&255;h*=1099511628211ULL;};
    for(const auto& row:d.S_init)for(unsigned char c:row)byte(c);
    for(int x:d.column_order)byte(x);
    for(int x:d.prod_to_paper)byte(x);
    for(int x:d.paper_bucket)byte(x+1);
    return h;
}
static int refine(Design& chosen,vector<HeadSample>& heads,const vector<Design>& candidates,
                  int selected,const vector<double>& scores,const vector<int>& counts,
                  const vector<bool>& valid,const vector<uint64_t>& raw_hashes,
                  const vector<vector<array<double,32>>>& rank_hist,
                  const vector<uint64_t>& original_catalog,clock_t absolute_deadline) {
    const clock_t started=clock();
    clock_t deadline=started+clock_t(search_budget::seconds(max(0.0,
        search_budget::real("FINAL_RACE_CPU",.32)))*CLOCKS_PER_SEC);
    if(absolute_deadline>0)deadline=min(deadline,absolute_deadline);
    auto expired=[&](){return clock()>=deadline;};
    const int finalist_limit=clamp(search_budget::integer("FINAL_RACE_LIMIT",2),2,4);
    const int warmup=max(0,search_budget::integer("FINAL_RACE_WARMUP",16));
    const int requested=max(1,search_budget::integer("FINAL_RACE_QUERIES",128));
    const bool use_branch=search_budget::integer("FINAL_RACE_BRANCH",1);
    const bool use_mdp=search_budget::integer("FINAL_RACE_MDP",1);
    const bool use_trace=search_budget::integer("FINAL_RACE_TRACE",1);
    const bool use_feedback=search_budget::integer("FINAL_RACE_FEEDBACK",1);
    const double control=search_budget::real("FINAL_RACE_CONTROL",.9);
    const double confidence=max(0.0,search_budget::real("FINAL_RACE_Z",.75));
    struct Summary {
        clock_t started;int original,chosen,finalists=1,queries=0;
        bool complete=false,timeout=false;double gain=0;
        ~Summary(){cerr<<"FINAL_RACE original="<<original<<" chosen="<<chosen
            <<" finalists="<<finalists<<" queries="<<queries<<" complete="<<complete
            <<" timeout="<<timeout<<" gain="<<gain<<" cpu="<<double(clock()-started)/CLOCKS_PER_SEC<<"\n";}
    } summary{started,selected,selected};
    uint64_t catalog_hash=0x9c718d3f62a04b55ULL;
    for(uint64_t x:original_catalog)catalog_hash^=x+0x9e3779b97f4a7c15ULL+(catalog_hash<<6)+(catalog_hash>>2);
    cerr<<"FINAL_RACE_CATALOG hash="<<catalog_hash<<" policy="<<use_branch<<" mdp="<<use_mdp
        <<" trace="<<use_trace<<" feedback="<<use_feedback<<" requested="<<requested<<"\n";
    for(int i=0;i<int(candidates.size());i++)cerr<<"FINAL_RACE_RAW index="<<i
        <<" B="<<candidates[i].tot_B<<" W="<<candidates[i].W<<" samples="<<counts[i]
        <<" valid="<<valid[i]<<" mean="<<(counts[i]?scores[i]/counts[i]:0)
        <<" hash="<<raw_hashes[i]<<"\n";
    if(candidates.size()<2 || expired()){summary.timeout=expired();return selected;}
    vector<int> alternatives;
    for(int i=0;i<int(candidates.size());i++)if(i!=selected && valid[i] && counts[i]>0)alternatives.push_back(i);
    stable_sort(alternatives.begin(),alternatives.end(),[&](int a,int b){return scores[a]/counts[a]<scores[b]/counts[b];});
    struct Finalist {
        Design design;vector<HeadSample> history;int source;
        unique_ptr<OnlineRouter> router;unique_ptr<BankFeedback> feedback;
        vector<array<int,2>> raw_counts;
        nov5_relaxed_trace::Statistics trace;
        double score=0,moves=0,squared_delta=0;
    };
    vector<Finalist> pool;pool.reserve(finalist_limit);
    pool.push_back({chosen,heads,selected});
    for(int id:alternatives) {
        if(int(pool.size())>=finalist_limit)break;
        if(expired()){summary.timeout=true;return selected;}
        Design proposal=candidates[id];vector<HeadSample> history;
        OnlineRouter sampler(proposal,proposal.internal_catalog,deadline);
        sampler.route_deadline=deadline;
        if(expired()){summary.timeout=true;return selected;}
        const int n=max(0,search_budget::integer("HIST_SAMPLES",42));
        uint64_t random=original_catalog[0]|1;int previous=-1;
        for(int q=0;q<n*7+32;q++) {
            if(expired()){summary.timeout=true;return selected;}
            int product;do{random^=random<<7;random^=random>>9;product=int(random%P);}while(product==previous);
            auto moves=sampler.query(product);previous=product;
            if(moves.size()>2*K || sampler.sim.current_display()!=sampler.A[product])return selected;
            if(q>=32 && (q-32)%7==0){history.emplace_back();capture_heads(sampler,history.back());}
        }
        sampled_bank_swaps(proposal,history,deadline);
        if(expired()){summary.timeout=true;return selected;}
        if(search_budget::integer("HIST_REDESIGN",1))history_holes(proposal,history,deadline);
        else {
            prefix_holes::Options options;options.max_total=16;options.max_per_paper=1;
            options.rank_mode=4;options.rank_weights=&rank_hist[id];
            options.objective_mode=1;options.own_hole_cost=.5;
            options.deadline=min(deadline,clock()+clock_t(search_budget::seconds(.06)*CLOCKS_PER_SEC));
            prefix_holes::optimize(proposal.S_init,proposal.internal_catalog,proposal.prod_to_paper,
                proposal.paper_bucket,proposal.tot_B,proposal.T,proposal.C,options);
        }
        if(expired()){summary.timeout=true;return selected;}
        pool.push_back({std::move(proposal),std::move(history),id});
    }
    summary.finalists=int(pool.size());
    if(pool.size()<2)return selected;
    for(auto& entry:pool) {
        cerr<<"FINAL_RACE_READY index="<<entry.source<<" hash="<<fingerprint(entry.design)
            <<" history="<<entry.history.size()<<"\n";
        if(search_budget::integer("FINAL_RACE_DUMP",0))for(int k=0;k<K;k++) {
            string paper(N,'.');for(int j=0;j<N;j++)paper[entry.design.column_order[j]]=entry.design.S_init[k][j];
            cerr<<"FINAL_RACE_PAPER index="<<entry.source<<" paper="<<k<<" pattern="<<paper<<"\n";
        }
        entry.router=make_unique<OnlineRouter>(entry.design,entry.design.internal_catalog,deadline);
        empirical_utility(*entry.router,entry.history);
        if(expired()){summary.timeout=true;return selected;}
        if(use_mdp)entry.router->prepare_bucket_mdp(deadline);
        if(expired()){summary.timeout=true;return selected;}
        entry.router->route_branching=use_branch;entry.router->route_deadline=deadline;
        entry.raw_counts=entry.router->hole_request_count;
        entry.feedback=make_unique<BankFeedback>(*entry.router);
    }
    uint64_t random=catalog_hash^0x01c94a6d7318bef5ULL;if(!random)random=1;
    int previous=-1;
    vector<double> one(pool.size());
    for(int q=0;q<warmup+requested;q++) {
        if(expired()){summary.timeout=true;return selected;}
        int product;do{random^=random<<7;random^=random>>9;product=int(random%P);}while(product==previous);
        for(int i=0;i<int(pool.size());i++) {
            auto& entry=pool[i];auto& router=*entry.router;
            // The routing feedback changes these counters. The control variate
            // instead needs the literal catalogue counts to stay conditionally
            // unbiased under the sampled non-repeating uniform distribution.
            router.hole_request_count.swap(entry.raw_counts);
            const double correction=control*order_adjustment(router,product,previous);
            router.hole_request_count.swap(entry.raw_counts);
            const auto start=router.sim.order;
            const uint64_t background=entry.feedback->background(router);
            auto moves=router.query(product);
            if(use_trace) {
                auto saved=nov5_relaxed_trace::stats();nov5_relaxed_trace::stats()=entry.trace;
                moves=nov5_relaxed_trace::optimize(router,product,start,std::move(moves),deadline);
                entry.trace=nov5_relaxed_trace::stats();nov5_relaxed_trace::stats()=saved;
                moves=trace_shortener::optimize(start,std::move(moves),router.opaque,deadline);
            }
            if(moves.size()>2*K || router.sim.current_display()!=router.A[product])return selected;
            if(use_feedback)entry.feedback->observe(router,product,background,q);
            if(expired()){summary.timeout=true;return selected;}
            one[i]=moves.size()+correction;
            if(q>=warmup){entry.score+=one[i];entry.moves+=moves.size();}
        }
        previous=product;
        if(q>=warmup) {
            summary.queries++;
            for(int i=1;i<int(pool.size());i++){double d=one[i]-one[0];pool[i].squared_delta+=d*d;}
        }
    }
    summary.complete=true;
    int best=0;
    for(int i=0;i<int(pool.size());i++) {
        const double delta=pool[i].score-pool[0].score;
        const double noise=sqrt(max(0.0,pool[i].squared_delta-delta*delta/requested));
        cerr<<"FINAL_RACE_SCORE index="<<pool[i].source<<" raw_moves="<<pool[i].moves
            <<" adjusted="<<pool[i].score<<" delta="<<delta<<" noise="<<noise
            <<" hash="<<fingerprint(pool[i].design)<<"\n";
        if(i && delta < -confidence*noise && pool[i].score<pool[best].score)best=i;
    }
    if(best) {
        summary.chosen=pool[best].source;summary.gain=(pool[0].score-pool[best].score)/requested;
        chosen=std::move(pool[best].design);heads=std::move(pool[best].history);
    }
    return summary.chosen;
}
} // namespace finalist_selection

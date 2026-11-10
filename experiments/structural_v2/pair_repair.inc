// Include after Design, ExactBackingPermutations, and search_budget.
// Endpoint exchanges keep every physical slot's owner count and anchor color.
// The core may already contain arbitrary literal patterns; displayed[s] must
// describe those core papers in ExactBackingPermutations::permutations[s].
namespace structural_pair_repair {
struct Result {
    bool feasible=false,timed_out=false;
    double cost=std::numeric_limits<double>::infinity();
    double initial_cost=std::numeric_limits<double>::infinity();
    int invalid_slots=0,initial_invalid_slots=0,accepted=0,changed_pairs=0,changed_products=0;
    int iterations=0,metric_evaluations=0;
};

struct DistanceSample {
    int states=0,samples=0;
    std::vector<uint8_t> distance;
    explicit DistanceSample(int B,int sample_count) {
        const auto& table=ExactBackingPermutations::get(B);
        states=table.states;samples=std::min(states,std::max(1,sample_count));
        std::vector<int> starts(states);for(int i=0;i<states;i++)starts[i]=i;
        // Fixed for each B: costs are comparable across mutation/re-pair calls.
        uint64_t seed=0x75b932aa1843ec6dULL^uint64_t(B);
        auto random=[&](){seed^=seed<<7;seed^=seed>>9;return uint32_t(seed);};
        if(samples<states)for(int i=states-1;i>0;i--)std::swap(starts[i],starts[random()%(i+1)]);
        distance.resize(size_t(samples)*states);
        for(int h=0;h<samples;h++) {
            std::array<uint8_t,9> position{},relative{};
            for(int i=0;i<B;i++)position[table.permutations[starts[h]][i]]=uint8_t(i);
            for(int s=0;s<states;s++) {
                for(int i=0;i<B;i++)relative[i]=position[table.permutations[s][i]];
                distance[size_t(h)*states+s]=table.distance[table.rank(relative)];
            }
        }
    }
};

inline const DistanceSample& distance_sample(int B) {
    const int states=ExactBackingPermutations::get(B).states;
    const int count=std::clamp(search_budget::integer("PAIR_REPAIR_SAMPLES",B==5?states:48),4,std::min(128,states));
    static std::map<std::pair<int,int>,std::unique_ptr<DistanceSample>> cache;
    auto key=std::make_pair(B,count);auto& ptr=cache[key];
    if(!ptr)ptr=std::make_unique<DistanceSample>(B,count);
    return *ptr;
}

struct PairMetric {
    float routing=0;
    uint8_t invalid=0;
    bool ready=false;
};

static Result optimize(Design& d,const std::vector<uint64_t>& displayed,
                       clock_t deadline,int iterations=20000,uint64_t seed=1) {
    Result answer;
    const clock_t started=clock();
    auto expired=[&](){return deadline>0 && clock()>=deadline;};
    const int B=d.tot_B,products=int(d.internal_catalog.size()),papers=int(d.S_init.size());
    if(B<3 || B>6 || d.C!=0 || d.T<1 || d.T>=64 || expired())return answer;
    const auto& table=ExactBackingPermutations::get(B);
    if(int(displayed.size())!=table.states || products!=P || papers!=K)return answer;
    const auto& sampler=distance_sample(B);
    if(expired()){answer.timed_out=true;return answer;}
    const auto& A=d.internal_catalog;
    const uint64_t prefix=(1ULL<<d.T)-1,suffix=~prefix;
    const double prefix_weight=std::max(0.0,search_budget::real("PAIR_REPAIR_PREFIX",.65));
    const double core_weight=std::max(0.0,search_budget::real("PAIR_REPAIR_CORE",1.0));
    const double initial_temperature=std::max(.01,search_budget::real("PAIR_REPAIR_TEMP",1.8));
    const double terminal_temperature=std::max(.001,search_budget::real("PAIR_REPAIR_END_TEMP",.035));
    const double invalid_penalty=std::max(1.0,search_budget::real("PAIR_REPAIR_INVALID",10.0));
    const int slot_count=papers-B;
    std::vector<std::array<int,2>> owners(slot_count,{-1,-1});
    std::vector<int> sizes(slot_count),anchors(slot_count),colors(slot_count),single_slots;
    for(int a=0;a<products;a++) {
        int slot=d.prod_to_paper[a]-B;
        if(slot<0 || slot>=slot_count || sizes[slot]>=2)return answer;
        owners[slot][sizes[slot]++]=a;
    }
    for(int s=0;s<slot_count;s++) {
        const int k=B+s,w=d.paper_bucket[k];
        if(w<0 || w>=d.T || sizes[s]!=(d.paper_is_paired[k]?2:1))return answer;
        char color=d.S_init[k][w];if(color!='0' && color!='1')return answer;
        anchors[s]=w;colors[s]=color-'0';
        for(int i=0;i<sizes[s];i++)if(int(A[owners[s][i]]>>w&1)!=colors[s])return answer;
        if(sizes[s]==1)single_slots.push_back(s);
    }
    const auto original_owners=owners;
    // Each endpoint's holes strictly above its anchor depend only on the slot.
    // At a differing cell below a shared anchor, the two endpoint mismatch
    // probabilities add to one, independent of the estimated background color.
    std::array<double,64> one_probability{};
    for(uint64_t a:A)for(int j=0;j<d.T;j++)one_probability[j]+=double(a>>j&1)/products;
    std::vector<std::array<double,64>> tail_cost(products);
    for(int a=0;a<products;a++) {
        double sum=0;
        for(int w=d.T-1;w>=0;w--) {
            tail_cost[a][w]=sum;
            sum+=(A[a]>>w&1)?1-one_probability[w]:one_probability[w];
        }
    }
    // The owner count and color of each slot remain fixed, so this retrieval
    // estimate is constant during a repair, but keeps the returned cost in
    // summed expected-move units when comparing different complete designs.
    std::vector<int> depth(d.T);
    double retrieval=0;
    for(int s=0;s<slot_count;s++)retrieval+=sizes[s]*(depth[anchors[s]]++ + 1.0);

    std::vector<PairMetric> metrics(size_t(products)*products);
    auto metric=[&](int u,int v)->const PairMetric& {
        if(u>v)std::swap(u,v);
        auto& m=metrics[size_t(u)*products+v];if(m.ready)return m;
        const uint64_t needed=(A[u]^A[v])&suffix;
        std::array<std::array<uint8_t,128>,2> best{};
        best[0].fill(uint8_t(B));best[1].fill(uint8_t(B));
        int goals[2]={},minimum_bad[2]={64,64};
        for(int s=0;s<table.states;s++) {
            const uint64_t bad=(displayed[s]^A[u])&needed;
            const uint64_t inverse=needed^bad;
            minimum_bad[0]=std::min(minimum_bad[0],__builtin_popcountll(bad));
            minimum_bad[1]=std::min(minimum_bad[1],__builtin_popcountll(inverse));
            for(int side=0;side<2;side++)if(!(side?inverse:bad)) {
                goals[side]++;
                for(int h=0;h<sampler.samples;h++)best[side][h]=std::min(best[side][h],sampler.distance[size_t(h)*table.states+s]);
            }
        }
        double total=0;
        for(int side=0;side<2;side++) {
            if(!goals[side]){m.invalid++;total+=B+minimum_bad[side];}
            else for(int h=0;h<sampler.samples;h++)total+=double(best[side][h])/sampler.samples;
        }
        m.routing=float(total);m.ready=true;answer.metric_evaluations++;return m;
    };
    struct SlotValue {double cost=0;int invalid=0;};
    auto evaluate_slot=[&](int s)->SlotValue {
        const int u=owners[s][0],w=anchors[s];
        SlotValue value;value.cost=prefix_weight*tail_cost[u][w];
        if(sizes[s]==2) {
            const int v=owners[s][1];const auto& m=metric(u,v);
            const uint64_t printable=(1ULL<<(w+1))-1;
            value.cost+=prefix_weight*(tail_cost[v][w]+__builtin_popcountll((A[u]^A[v])&printable));
            value.cost+=core_weight*m.routing;value.invalid=m.invalid;
        }
        return value;
    };
    std::vector<SlotValue> values(slot_count);
    double current_cost=retrieval;int current_invalid=0,current_invalid_slots=0;
    for(int s=0;s<slot_count;s++) {
        if(expired()){answer.timed_out=true;return answer;}
        values[s]=evaluate_slot(s);current_cost+=values[s].cost;
        current_invalid+=values[s].invalid;current_invalid_slots+=values[s].invalid!=0;
    }
    answer.initial_cost=current_cost;answer.initial_invalid_slots=current_invalid_slots;
    std::vector<std::array<int,2>> best_owners;
    double best_cost=std::numeric_limits<double>::infinity();
    if(!current_invalid){best_owners=owners;best_cost=current_cost;}
    if(!seed)seed=0x27c3698372efa51dULL;
    auto random=[&](){seed^=seed<<7;seed^=seed>>9;return uint32_t(seed);};
    const int requested=std::max(0,iterations);
    double progress=0,temperature=initial_temperature,penalty=invalid_penalty;
    for(int it=0;it<requested;it++) {
        answer.iterations=it+1;
        if((it&63)==0) {
            if(expired()){answer.timed_out=true;break;}
            progress=double(it)/std::max(1,requested);
            if(deadline>started)progress=std::max(progress,std::min(1.0,double(clock()-started)/(deadline-started)));
            temperature=initial_temperature*std::pow(terminal_temperature/initial_temperature,progress);
            penalty=invalid_penalty*(1+2*progress);
        }
        int s=int(random()%slot_count);
        if(current_invalid && random()%4!=0) {
            int start=s;
            while(!values[s].invalid){if(++s==slot_count)s=0;if(s==start)break;}
        }
        int t;
        if(values[s].invalid && !single_slots.empty() && random()%4!=0)t=single_slots[random()%single_slots.size()];
        else t=int(random()%slot_count);
        if(t==s)continue;
        int touched[3]={s,t,-1},count=2;
        std::array<std::array<int,2>,3> old={owners[s],owners[t],std::array<int,2>{-1,-1}};
        const int kind=int(random()%8);
        if(kind==0 && sizes[s]==2 && sizes[t]==2) {
            bool compatible=true;
            for(int i=0;i<2;i++)compatible&=int(A[owners[s][i]]>>anchors[t]&1)==colors[t]
                && int(A[owners[t][i]]>>anchors[s]&1)==colors[s];
            if(!compatible)continue;
            std::swap(owners[s],owners[t]);
        }else{
            int members[3]={int(random()%sizes[s]),int(random()%sizes[t]),0};
            if(kind==1) {
                int q=int(random()%slot_count);if(q==s || q==t)continue;
                touched[2]=q;old[2]=owners[q];members[2]=int(random()%sizes[q]);count=3;
            }
            bool compatible=true;
            for(int i=0;i<count;i++) {
                int a=old[i][members[i]],dest=touched[(i+1)%count];
                compatible&=int(A[a]>>anchors[dest]&1)==colors[dest];
            }
            if(!compatible)continue;
            for(int i=0;i<count;i++)owners[touched[(i+1)%count]][members[(i+1)%count]]=old[i][members[i]];
        }
        double delta=0;int invalid_delta=0;std::array<SlotValue,3> proposed;
        for(int i=0;i<count;i++) {
            int q=touched[i];proposed[i]=evaluate_slot(q);
            delta+=proposed[i].cost-values[q].cost;
            invalid_delta+=proposed[i].invalid-values[q].invalid;
        }
        const double energy=delta+penalty*invalid_delta;
        if(energy<=0 || (random()+.5)/4294967296.0<std::exp(-energy/temperature)) {
            answer.accepted++;current_cost+=delta;current_invalid+=invalid_delta;
            for(int i=0;i<count;i++)values[touched[i]]=proposed[i];
            if(!current_invalid && current_cost<best_cost-1e-9){best_cost=current_cost;best_owners=owners;}
        }else for(int i=0;i<count;i++)owners[touched[i]]=old[i];
    }
    if(best_owners.empty()) {
        for(const auto& v:values)answer.invalid_slots+=v.invalid!=0;
        return answer;
    }
    // Commit only a fully feasible assignment, rebuilding every primary and
    // all ownership metadata together. Core rows and slot attributes stay put.
    std::vector<std::string> rows=d.S_init;
    std::vector<PairInfo> pairs;
    std::vector<int> paper_of(products,-1),pair_of(products,-1);
    for(int s=0;s<slot_count;s++) {
        const int k=B+s,w=anchors[s],u=best_owners[s][0];
        const int v=sizes[s]==2?best_owners[s][1]:u;
        uint64_t common=~(A[u]^A[v]);
        const uint64_t domain=((1ULL<<(w+1))-1)|suffix;
        rows[k].assign(64,'.');
        for(uint64_t bits=common&domain;bits;bits&=bits-1) {
            int j=__builtin_ctzll(bits);rows[k][j]=char('0'+int(A[u]>>j&1));
        }
        paper_of[u]=k;
        if(sizes[s]==2) {
            paper_of[v]=k;int index=int(pairs.size());pair_of[u]=pair_of[v]=index;
            pairs.push_back({u,v,0,-1,1,-1,w,k});
            auto was=original_owners[s],now=best_owners[s];
            if(was[0]>was[1])std::swap(was[0],was[1]);if(now[0]>now[1])std::swap(now[0],now[1]);
            answer.changed_pairs+=was!=now;
        }
        for(int i=0;i<sizes[s];i++)if(d.prod_to_paper[best_owners[s][i]]!=k)answer.changed_products++;
    }
    d.S_init=std::move(rows);d.pairs=std::move(pairs);
    d.prod_to_paper=std::move(paper_of);d.prod_to_pair_idx=std::move(pair_of);
    answer.feasible=true;answer.cost=best_cost;answer.invalid_slots=0;
    return answer;
}
} // namespace structural_pair_repair

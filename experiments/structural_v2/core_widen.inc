#include <numeric>
// Transfer an anchor column into the general core, relocate its primary slots,
// then rebuild the full product matching. This changes the prefix/core split.
namespace structural_widen {
static std::vector<uint64_t> display_core(const Design& d) {
    const auto& table=ExactBackingPermutations::get(d.tot_B);
    std::array<uint64_t,8> ink{},black{};
    for(int k=0;k<d.tot_B;k++)for(int j=d.T;j<64;j++){
        if(d.S_init[k][j]!='.')ink[k]|=1ULL<<j;
        if(d.S_init[k][j]=='1')black[k]|=1ULL<<j;
    }
    std::vector<uint64_t> result(table.states);
    for(int id=0;id<table.states;id++){
        uint64_t seen=0;
        for(int p=0;p<d.tot_B;p++){
            int k=table.permutations[id][p];result[id]|=black[k]&~seen;seen|=ink[k];
        }
    }
    return result;
}
static bool transfer(Design& d,int column) {
    const int oldT=d.T,B=d.tot_B;
    if(d.C!=0 || oldT<=8 || column<0 || column>=oldT)return false;
    std::vector<std::vector<int>> owned(K);
    for(int a=0;a<P;a++)owned[d.prod_to_paper[a]].push_back(a);
    std::vector<int> map(64);std::iota(map.begin(),map.end(),0);
    map.erase(map.begin()+column);map.insert(map.begin()+oldT-1,column);
    auto old_columns=d.column_order;
    for(int j=0;j<64;j++)d.column_order[j]=old_columns[map[j]];
    d.internal_catalog=permute_catalog_columns(d.internal_catalog,map);
    for(auto& s:d.S_init){auto old=s;for(int j=0;j<64;j++)s[j]=old[map[j]];}
    --d.T;++d.W;
    std::array<int,64> count{},frequency{};
    std::vector<int> displaced;
    for(int k=B;k<K;k++){
        int& w=d.paper_bucket[k];
        if(w==column){displaced.push_back(k);continue;}
        if(w>column)--w;
        count[w]++;frequency[w]+=owned[k].size();
    }
    std::stable_sort(displaced.begin(),displaced.end(),[&](int a,int b){return owned[a].size()>owned[b].size();});
    for(int k:displaced){
        if(owned[k].empty())return false;
        uint64_t difference=0;for(int a:owned[k])difference|=d.internal_catalog[a]^d.internal_catalog[owned[k][0]];
        int best=-1;double price=1.e30;
        for(int w=0;w<d.T;w++)if(!(difference>>w&1)){
            double value=.5*(frequency[w]+owned[k].size()*(count[w]+2))
                +.325*owned[k].size()*(d.T-1-w)
                +.65*__builtin_popcountll(difference&((1ULL<<(w+1))-1));
            if(value<price){price=value;best=w;}
        }
        if(best<0)return false;
        d.paper_bucket[k]=best;count[best]++;frequency[best]+=owned[k].size();
    }
    // Restore staircase printability and sort the physical slots by anchor.
    for(int k=B;k<K;k++){
        if(owned[k].empty())return false;
        uint64_t bits=d.internal_catalog[owned[k][0]],difference=0;
        for(int a:owned[k])difference|=bits^d.internal_catalog[a];
        const uint64_t mask=(((1ULL<<(d.paper_bucket[k]+1))-1)|(~0ULL<<d.T))&~difference;
        d.S_init[k].assign(64,'.');
        for(int j=0;j<64;j++)if(mask>>j&1)d.S_init[k][j]=char('0'+int(bits>>j&1));
    }
    std::vector<int> order(K);std::iota(order.begin(),order.end(),0);
    std::stable_sort(order.begin()+B,order.end(),[&](int a,int b){return d.paper_bucket[a]<d.paper_bucket[b];});
    const Design old=d;
    for(int k=B;k<K;k++){
        int from=order[k];d.S_init[k]=old.S_init[from];d.paper_bucket[k]=old.paper_bucket[from];
        d.paper_is_paired[k]=old.paper_is_paired[from];
        for(int a:owned[from])d.prod_to_paper[a]=k;
    }
    for(auto& p:d.pairs){p.paper_id=d.prod_to_paper[p.u];p.bucket_w=d.paper_bucket[p.paper_id];}
    return true;
}
static void optimize(Design& d,clock_t absolute_deadline) {
    if(!search_budget::integer("CORE_WIDEN",0))return;
    const clock_t started=clock();
    clock_t deadline=started+clock_t(search_budget::seconds(search_budget::real("CORE_WIDEN_CPU",.6))*CLOCKS_PER_SEC);
    if(absolute_deadline>0)deadline=std::min(deadline,absolute_deadline);
    auto expired=[&](){return clock()>=deadline;};
    if(expired() || d.tot_B>6 || d.C!=0)return;
    uint64_t state=d.internal_catalog[0]^0xefedafceb29f3f3dULL;if(!state)state=1;
    auto rnd=[&](){state^=state<<7;state^=state>>9;return uint32_t(state);};
    const int queries=std::max(64,search_budget::integer("CORE_WIDEN_QUERIES",384));
    std::vector<int> sequence;int previous=-1;
    for(int q=0;q<queries+32;q++){
        int a;do{a=rnd()%P;}while(a==previous);sequence.push_back(a);previous=a;
    }
    auto measure=[&](const Design& candidate){
        if(expired())return 1.e20;
        OnlineRouter router(candidate,candidate.internal_catalog,clock());
        router.route_deadline=deadline;router.route_branching=search_budget::integer("CORE_WIDEN_BRANCH",0);
        double value=0;
        for(int q=0;q<int(sequence.size());q++){
            if((q&7)==0 && expired())return 1.e20;
            auto moves=router.query(sequence[q]);
            if(router.sim.current_display()!=candidate.internal_catalog[sequence[q]])return 1.e20;
            if(q>=32)value+=moves.size();
        }
        return value/queries;
    };
    const Design original=d;
    double initial=measure(original),best=initial,best_changed=1.e20;
    Design winner=original,changed=original;
    int calls=0,feasible=0,improved=0;
    const int maximum=std::max(1,search_budget::integer("CORE_WIDEN_TRIES",64));
    const int depth=std::max(1,search_budget::integer("CORE_WIDEN_DEPTH",1));
    for(int it=0;it<maximum && !expired();it++){
        Design candidate=(depth>1 && winner.T<original.T && rnd()%3==0)?winner:original;
        if(original.T-candidate.T>=depth)candidate=original;
        const int B=candidate.tot_B;
        int column=it<original.T?original.T-1-it:rnd()%candidate.T;
        if(column>=candidate.T)column=candidate.T-1;
        if(!transfer(candidate,column))continue;
        std::array<int,8> p{};for(int k=0;k<B;k++)p[k]=k;
        for(int k=B-1;k>0;k--)std::swap(p[k],p[rnd()%(k+1)]);
        for(int k=0;k<B;k++)candidate.S_init[k][candidate.T]='.';
        const int opaque=(it%3==0)?4:2;
        for(int k=0;k<opaque;k++)candidate.S_init[p[k]][candidate.T]=char('0'+(k&1));
        auto display=display_core(candidate);
        clock_t repair_end=std::min(deadline,clock()+clock_t(search_budget::seconds(search_budget::real("CORE_WIDEN_REPAIR_CPU",.2))*CLOCKS_PER_SEC));
        auto result=structural_global_pair_repair::optimize(candidate,display,repair_end,state);
        calls++;
        if(!result.feasible)continue;
        feasible++;
        double cost=measure(candidate);
        if(cost<best_changed){best_changed=cost;changed=candidate;}
        if(cost<best){best=cost;winner=candidate;improved++;}
    }
    bool force=search_budget::integer("CORE_WIDEN_FORCE",0);
    if(force && best_changed<1.e19)d=std::move(changed);
    else if(best+search_budget::real("CORE_WIDEN_MIN_GAIN",.03)<initial)d=std::move(winner);
    std::cerr<<"CORE_WIDEN before="<<initial<<" after="<<best<<" best_changed="<<best_changed
        <<" calls="<<calls<<" feasible="<<feasible<<" improved="<<improved<<" W_before="<<original.W
        <<" W_after="<<d.W<<" forced="<<force<<" cpu="<<double(clock()-started)/CLOCKS_PER_SEC<<"\n";
}
}
static void optimize_wider_core(Design& d,clock_t deadline){structural_widen::optimize(d,deadline);}

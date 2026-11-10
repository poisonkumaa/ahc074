// Include after Design, ExactBackingPermutations, and search_budget are defined.
// Call once, after all comparison-only design refinement and before constructing
// the actual OnlineRouter. Requires the generic goal-set backing router.
namespace ternary_core_optimizer {
static void optimize(Design& d,clock_t absolute_deadline) {
    if(!search_budget::integer("TERNARY_CORE",0))return;
    const clock_t started=clock();
    const double cap=search_budget::seconds(std::max(0.0,search_budget::real("TERNARY_CORE_CPU",.12)));
    clock_t deadline=started+clock_t(cap*CLOCKS_PER_SEC);
    if(absolute_deadline>0)deadline=std::min(deadline,absolute_deadline);
    auto expired=[&](){return clock()>=deadline;};
    const int B=d.tot_B,products=int(d.internal_catalog.size());
    if(B<3 || B>std::min(6,search_budget::integer("TERNARY_CORE_MAX_B",6)) ||
       d.C!=0 || d.T<=0 || d.T>=64 || expired())return;
    const auto& table=ExactBackingPermutations::get(B);
    const int states=table.states,words=(states+63)/64;
    const uint64_t suffix=~uint64_t(0)<<d.T;
    std::vector<uint64_t> paper_ink(B),paper_black(B),holes(products);
    std::vector<std::vector<int>> affected(64);
    for(int k=0;k<B;k++)for(int j=d.T;j<64;j++){
        if(d.S_init[k][j]!='.')paper_ink[k]|=uint64_t(1)<<j;
        if(d.S_init[k][j]=='1')paper_black[k]|=uint64_t(1)<<j;
    }
    for(int a=0;a<products;a++){
        const int main=d.prod_to_paper[a];
        for(int j=d.T;j<64;j++)if(d.S_init[main][j]=='.'){
            holes[a]|=uint64_t(1)<<j;affected[j].push_back(a);
        }
        holes[a]&=suffix;
    }
    // Reserve an unchanged two-paper witness for every pair. Prefer reserving
    // a heavily used column, leaving less constrained duplicates editable.
    std::array<int,64> endpoint0{},endpoint1{},extra{},extra_color{};
    endpoint0.fill(-1);endpoint1.fill(-1);extra.fill(-1);
    std::array<std::array<int,6>,6> witness{};
    for(auto& row:witness)row.fill(-1);
    for(int j=d.T;j<64;j++){
        int count=0;
        for(int k=0;k<B;k++)if(d.S_init[k][j]!='.'){
            count++;
            if(d.S_init[k][j]=='0')endpoint0[j]=k;
            else endpoint1[j]=k;
        }
        if(count!=2 || endpoint0[j]<0 || endpoint1[j]<0)return;
        int u=std::min(endpoint0[j],endpoint1[j]),v=std::max(endpoint0[j],endpoint1[j]);
        int& old=witness[u][v];
        if(old<0 || affected[j].size()>affected[old].size())old=j;
    }
    std::array<bool,64> reserved{};
    for(int u=0;u<B;u++)for(int v=u+1;v<B;v++){
        if(witness[u][v]<0)return;
        reserved[witness[u][v]]=true;
    }
    std::vector<int> columns;
    for(int j=d.T;j<64;j++)if(!reserved[j] && !affected[j].empty())columns.push_back(j);
    // Try the easiest feasibility constraints first when the deadline permits
    // only part of a complete best-improvement sweep.
    std::stable_sort(columns.begin(),columns.end(),[&](int a,int b){return affected[a].size()<affected[b].size();});
    std::vector<std::array<uint8_t,6>> positions(states);
    std::vector<uint64_t> displayed(states);
    for(int s=0;s<states;s++){
        uint64_t seen=0;
        for(int i=0;i<B;i++){
            int k=table.permutations[s][i];positions[s][k]=uint8_t(i);
            displayed[s]|=paper_black[k]&~seen;seen|=paper_ink[k];
        }
    }
    std::vector<uint8_t> wrong(size_t(products)*states);
    std::vector<uint64_t> goals(size_t(products)*words);
    for(int a=0;a<products;a++){
        if(expired())return;
        bool any=false;
        for(int s=0;s<states;s++){
            const int bad=__builtin_popcountll((displayed[s]^d.internal_catalog[a])&holes[a]);
            wrong[size_t(a)*states+s]=uint8_t(bad);
            if(!bad){goals[size_t(a)*words+s/64]|=uint64_t(1)<<(s%64);any=true;}
        }
        if(!any){std::cerr<<"TERNARY_CORE invalid_original="<<a<<"\n";return;}
    }
    // Cost is the exact sum, over all current permutations, of the shortest
    // physically legal path to this goal set. A goal-count proxy is not used.
    std::unordered_map<std::string,int> distance_cache;
    const int cache_limit=std::max(0,search_budget::integer("TERNARY_CORE_CACHE",12000));
    distance_cache.reserve(std::min(cache_limit,2048));
    std::vector<uint8_t> distance(states);
    std::vector<int> queue;queue.reserve(states);
    long long bfs_runs=0,cache_hits=0;
    auto cost=[&](const uint64_t* bits,int& answer){
        std::string key(reinterpret_cast<const char*>(bits),size_t(words)*sizeof(uint64_t));
        auto found=distance_cache.find(key);
        if(found!=distance_cache.end()){answer=found->second;cache_hits++;return true;}
        if(expired())return false;
        std::fill(distance.begin(),distance.end(),uint8_t(255));queue.clear();
        for(int w=0;w<words;w++)for(uint64_t b=bits[w];b;b&=b-1){
            int s=64*w+__builtin_ctzll(b);distance[s]=0;queue.push_back(s);
        }
        if(queue.empty())return false;
        answer=0;
        for(size_t head=0;head<queue.size();head++){
            if((head&127)==0 && expired())return false;
            const int s=queue[head],next=distance[s]+1;
            for(int previous:table.reverse_edges[s])if(distance[previous]==255){
                distance[previous]=uint8_t(next);answer+=next;queue.push_back(previous);
            }
        }
        if(int(queue.size())!=states)return false;
        bfs_runs++;
        if(int(distance_cache.size())<cache_limit)distance_cache.emplace(std::move(key),answer);
        return true;
    };
    std::vector<int> product_cost(products);
    long long total=0;
    for(int a=0;a<products;a++){
        if(!cost(goals.data()+size_t(a)*words,product_cost[a])){
            std::cerr<<"TERNARY_CORE timeout_stage=initial cpu="<<double(clock()-started)/CLOCKS_PER_SEC<<"\n";return;
        }
        total+=product_cost[a];
    }
    const long long original_total=total;
    const int max_edits=std::max(0,search_budget::integer("TERNARY_CORE_EDITS",8));
    const int max_steps=std::max(0,search_budget::integer("TERNARY_CORE_STEPS",2*max_edits));
    const double minimum_gain=std::max(0.0,search_budget::real("TERNARY_CORE_GAIN",0.0));
    int modified=0,accepted=0;long long trials=0,feasible=0,improving=0;
    bool timed_out=false;
    for(int step=0;step<max_steps && !expired();step++){
        struct Best {
            int column=-1,paper=-1,color=0;
            long long total=0;
            std::vector<int> costs;
            std::vector<uint64_t> ones;
        } best;
        best.total=total;
        for(int j:columns){
            if(expired()){timed_out=true;break;}
            const auto& list=affected[j];const uint64_t bit=uint64_t(1)<<j;
            std::vector<uint64_t> other(size_t(list.size())*words),trial(other.size()),ones(words);
            std::vector<int> trial_cost(list.size());
            for(size_t ai=0;ai<list.size();ai++){
                const int a=list[ai];
                for(int s=0;s<states;s++){
                    const int bad=wrong[size_t(a)*states+s];
                    if(!bad || (bad==1 && ((displayed[s]^d.internal_catalog[a])&bit)))
                        other[ai*words+s/64]|=uint64_t(1)<<(s%64);
                }
            }
            std::vector<std::pair<int,int>> options;
            if(extra[j]>=0)options.push_back({-1,0});
            if(extra[j]>=0 || modified<max_edits)
                for(int k=0;k<B;k++)if(k!=endpoint0[j] && k!=endpoint1[j])
                    for(int c=0;c<2;c++)if(k!=extra[j] || c!=extra_color[j])options.push_back({k,c});
            for(auto [k,c]:options){
                if(expired()){timed_out=true;break;}
                trials++;std::fill(ones.begin(),ones.end(),0);
                for(int s=0;s<states;s++){
                    bool color=positions[s][endpoint1[j]]<positions[s][endpoint0[j]];
                    if(k>=0 && positions[s][k]<std::min(positions[s][endpoint0[j]],positions[s][endpoint1[j]]))color=c;
                    if(color)ones[s/64]|=uint64_t(1)<<(s%64);
                }
                bool possible=true;
                for(size_t ai=0;ai<list.size();ai++){
                    const bool want=bool(d.internal_catalog[list[ai]]&bit);uint64_t any=0;
                    for(int w=0;w<words;w++){
                        uint64_t b=other[ai*words+w]&(want?ones[w]:~ones[w]);
                        trial[ai*words+w]=b;any|=b;
                    }
                    if(!any){possible=false;break;}
                }
                if(!possible)continue;
                feasible++;long long candidate=total;
                for(size_t ai=0;ai<list.size();ai++){
                    int a=list[ai];
                    if(std::equal(trial.data()+ai*words,trial.data()+(ai+1)*words,goals.data()+size_t(a)*words))trial_cost[ai]=product_cost[a];
                    else if(!cost(trial.data()+ai*words,trial_cost[ai])){timed_out=true;possible=false;break;}
                    candidate+=trial_cost[ai]-product_cost[a];
                }
                if(!possible)break;
                if(candidate<total)improving++;
                if(candidate<best.total && double(total-candidate)>minimum_gain*products*states){
                    best.column=j;best.paper=k;best.color=c;best.total=candidate;
                    best.costs=trial_cost;best.ones=ones;
                }
            }
            if(timed_out)break;
        }
        if(best.column<0)break;
        const int j=best.column;const uint64_t bit=uint64_t(1)<<j;
        const auto& list=affected[j];
        for(size_t ai=0;ai<list.size();ai++){
            const int a=list[ai];
            std::fill(goals.begin()+size_t(a)*words,goals.begin()+size_t(a+1)*words,0);
            for(int s=0;s<states;s++){
                bool old_color=bool(displayed[s]&bit),new_color=bool(best.ones[s/64]>>(s%64)&1);
                auto& bad=wrong[size_t(a)*states+s];
                if(old_color!=new_color)bad+=((displayed[s]^d.internal_catalog[a])&bit)?-1:1;
                if(!bad)goals[size_t(a)*words+s/64]|=uint64_t(1)<<(s%64);
            }
            product_cost[a]=best.costs[ai];
        }
        for(int s=0;s<states;s++)displayed[s]=(displayed[s]&~bit)|((best.ones[s/64]>>(s%64)&1)?bit:0);
        if(extra[j]>=0){d.S_init[extra[j]][j]='.';modified--;}
        extra[j]=best.paper;extra_color[j]=best.color;
        if(extra[j]>=0){d.S_init[extra[j]][j]=char('0'+best.color);modified++;}
        total=best.total;accepted++;
        if(timed_out)break;
    }
    std::cerr<<"TERNARY_CORE B="<<B<<" states="<<states<<" trials="<<trials<<" feasible="<<feasible
        <<" improving="<<improving<<" accepted="<<accepted<<" edited_columns="<<modified
        <<" mean_before="<<double(original_total)/(products*states)<<" mean_after="<<double(total)/(products*states)
        <<" bfs="<<bfs_runs<<" cache_hits="<<cache_hits<<" timeout="<<(timed_out||expired())
        <<" cpu="<<double(clock()-started)/CLOCKS_PER_SEC<<"\n";
}
} // namespace ternary_core_optimizer

static void optimize_ternary_core(Design& chosen,clock_t deadline) {
    ternary_core_optimizer::optimize(chosen,deadline);
}

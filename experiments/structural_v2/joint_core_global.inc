// Include after pair_repair.inc, global_pair_repair.inc, and the generic core router. Run after the
// comparison-only synthesis is complete, before constructing the live router.
namespace structural_joint_core {
static std::vector<uint64_t> render(const Design& d,const ExactBackingPermutations& table) {
    std::array<uint64_t,6> ink{},black{};
    for(int k=0;k<d.tot_B;k++)for(int j=d.T;j<64;j++){
        if(d.S_init[k][j]!='.')ink[k]|=uint64_t(1)<<j;
        if(d.S_init[k][j]=='1')black[k]|=uint64_t(1)<<j;
    }
    std::vector<uint64_t> result(table.states);
    for(int s=0;s<table.states;s++){
        uint64_t seen=0;
        for(int i=0;i<d.tot_B;i++){
            int k=table.permutations[s][i];result[s]|=black[k]&~seen;seen|=ink[k];
        }
    }
    return result;
}

static void optimize(Design& d,clock_t absolute_deadline) {
    if(!search_budget::integer("CORE_JOINT",0))return;
    const int B=d.tot_B,P0=int(d.internal_catalog.size());
    if(B<3 || B>std::min(6,search_budget::integer("CORE_JOINT_MAX_B",6)) || d.C!=0 || d.T<=0 || d.T>=64)return;
    const clock_t started=clock();
    const double cap=search_budget::seconds(std::max(0.0,search_budget::real("CORE_JOINT_CPU",.60)));
    clock_t deadline=started+clock_t(cap*CLOCKS_PER_SEC);
    if(absolute_deadline>0)deadline=std::min(deadline,absolute_deadline);
    auto expired=[&](){return clock()>=deadline;};
    if(expired())return;
    const auto& table=ExactBackingPermutations::get(B);
    const uint64_t suffix=~uint64_t(0)<<d.T;
    const Design original=d;
    std::array<std::array<int,6>,6> witness{};
    for(auto& row:witness)row.fill(-1);
    std::array<int,64> demand{};
    for(int a=0;a<P0;a++)for(int j=d.T;j<64;j++)demand[j]+=d.S_init[d.prod_to_paper[a]][j]=='.';
    for(int j=d.T;j<64;j++){
        int zero=-1,one=-1,count=0;
        for(int k=0;k<B;k++)if(d.S_init[k][j]!='.'){
            count++;if(d.S_init[k][j]=='0')zero=k;else one=k;
        }
        if(count==2 && zero>=0 && one>=0){
            int u=std::min(zero,one),v=std::max(zero,one),&old=witness[u][v];
            if(old<0 || demand[j]>demand[old])old=j;
        }
    }
    std::array<bool,64> reserved{};
    for(int u=0;u<B;u++)for(int v=u+1;v<B;v++){
        if(witness[u][v]<0)return;
        reserved[witness[u][v]]=true;
    }
    std::vector<int> columns;
    for(int j=d.T;j<64;j++)if(!reserved[j])columns.push_back(j);
    if(columns.empty())return;
    uint64_t random_state=0x7dcabe24149a563bULL;
    for(uint64_t x:d.internal_catalog)random_state^=x+0x9e3779b97f4a7c15ULL+(random_state<<6)+(random_state>>2);
    if(!random_state)random_state=1;
    auto random32=[&](){random_state^=random_state<<7;random_state^=random_state>>9;return uint32_t(random_state);};
    auto random01=[&](){return (double(random32())+.5)/4294967296.0;};
    const double repair_slice=search_budget::seconds(std::max(0.0,search_budget::real("CORE_JOINT_REPAIR_CPU",.12)));
    auto repair_deadline=[&](){return std::min(deadline,clock()+clock_t(repair_slice*CLOCKS_PER_SEC));};
    const int repair_iterations=std::max(0,search_budget::integer("CORE_JOINT_REPAIR_ITERS",4000));
    const int maximum_calls=std::max(0,search_budget::integer("CORE_JOINT_CALLS",128));
    const int maximum_chain=std::max(1,search_budget::integer("CORE_JOINT_CHAIN",3));
    const int proposal_count=std::max(1,std::min(16,search_budget::integer("CORE_JOINT_PROPOSALS",4)));
    const bool mutate_core=search_budget::integer("CORE_JOINT_MUTATE",1);
    const bool global_repair=search_budget::integer("CORE_JOINT_GLOBAL",0);
    const bool return_mutated=search_budget::integer("CORE_JOINT_RETURN_MUTATED",0);
    const bool require_ternary=search_budget::integer("CORE_JOINT_REQUIRE_TERNARY",0);
    const double mutated_slack=std::max(0.0,search_budget::real("CORE_JOINT_MUTATED_SLACK",1.0))*P0;
    const int maximum_edits=std::max(1,search_budget::integer("CORE_JOINT_EDITS",12));
    const double temperature=std::max(0.0,search_budget::real("CORE_JOINT_TEMP",8.0));
    const int pattern_mode=search_budget::integer("CORE_JOINT_PATTERN",0);
    if(pattern_mode==4 && B<4)return;
    Design current=d;
    auto current_display=render(current,table);
    // Zero iterations measures the original pairing under the same objective
    // used for every repaired candidate. The repair engine may normalize its
    // main stencils; do not commit that normalization without an improvement.
    auto baseline=structural_pair_repair::optimize(current,current_display,repair_deadline(),0,random_state);
    if(!baseline.feasible || expired()){
        std::cerr<<"CORE_JOINT initial_feasible="<<baseline.feasible<<" timeout="<<expired()
                 <<" cpu="<<double(clock()-started)/CLOCKS_PER_SEC<<"\n";
        return;
    }
    double current_cost=baseline.cost,best_cost=baseline.cost;
    Design best=current,pending=current;
    Design best_mutated;
    double best_mutated_cost=std::numeric_limits<double>::infinity();
    auto pending_display=current_display;
    int calls=0,failed=0,feasible=0,accepted=0,improved=0,chain=0,proposals=0;
    std::vector<double> log_goals(table.states+1);
    for(int i=1;i<=table.states;i++)log_goals[i]=std::log1p(double(i));
    // This inexpensive score only selects which mutation to send to repair.
    // Admission and best-design tracking always use the repair engine's
    // actual movement/prefix/retrieval objective and full hard feasibility.
    auto proposal_quality=[&](const Design& source,const std::vector<uint64_t>& display){
        double score=0;
        for(int a=0;a<P0;a++){
            if((a&31)==0 && expired())return 1e100;
            uint64_t holes=0;
            for(int j=source.T;j<64;j++)if(source.S_init[source.prod_to_paper[a]][j]=='.')holes|=uint64_t(1)<<j;
            holes&=suffix;if(!holes)continue;
            int goals=0,nearest=65;
            for(uint64_t out:display){
                uint64_t bad=(out^source.internal_catalog[a])&holes;
                if(!bad)goals++;
                else if(!goals)nearest=std::min(nearest,__builtin_popcountll(bad));
            }
            if(goals)score-=log_goals[goals];
            else score+=16+4*nearest;
        }
        return score;
    };
    auto changed_column=[&](const Design& source,int j){
        for(int k=0;k<B;k++)if(source.S_init[k][j]!=original.S_init[k][j])return true;
        return false;
    };
    struct PairProbe {int u,v;uint64_t difference;};
    std::vector<PairProbe> probes;
    int paired_slots=0;
    for(int k=B;k<int(d.S_init.size());k++)paired_slots+=d.paper_is_paired[k];
    if(global_repair){
        const int nearest=std::max(0,std::min(12,search_budget::integer("CORE_JOINT_PROBE_NEAR",4)));
        const int random_count=std::max(0,std::min(12,search_budget::integer("CORE_JOINT_PROBE_RANDOM",4)));
        std::vector<bool> used(size_t(P0)*P0);
        auto add=[&](int u,int v){
            if(u==v)return;if(u>v)std::swap(u,v);
            if(used[size_t(u)*P0+v])return;used[size_t(u)*P0+v]=true;
            probes.push_back({u,v,(d.internal_catalog[u]^d.internal_catalog[v])&suffix});
        };
        for(int u=0;u<P0;u++){
            std::vector<std::pair<int,int>> neighbors;
            for(int v=0;v<P0;v++)if(v!=u)neighbors.push_back({__builtin_popcountll(d.internal_catalog[u]^d.internal_catalog[v]),v});
            std::partial_sort(neighbors.begin(),neighbors.begin()+std::min(nearest,int(neighbors.size())),neighbors.end());
            for(int i=0;i<std::min(nearest,int(neighbors.size()));i++)add(u,neighbors[i].second);
            for(int i=0;i<random_count;i++)add(u,int(random32()%P0));
        }
    }
    int global_probe_evaluations=0,global_probe_edges=0,global_probe_match=0;
    auto global_quality=[&](const std::vector<uint64_t>& display){
        Blossom matching(P0);int edges=0;
        for(size_t i=0;i<probes.size();i++){
            if((i&63)==0 && expired())return 1e100;
            const auto& pair=probes[i];bool first=false,second=false;
            for(uint64_t out:display){
                uint64_t bad=(out^d.internal_catalog[pair.u])&pair.difference;
                first|=bad==0;second|=bad==pair.difference;
                if(first && second)break;
            }
            if(first && second){matching.g[pair.u].push_back(pair.v);matching.g[pair.v].push_back(pair.u);edges++;}
        }
        if(expired())return 1e100;
        int matched=matching.solve();
        global_probe_evaluations++;global_probe_edges=std::max(global_probe_edges,edges);
        global_probe_match=std::max(global_probe_match,matched);
        // A sampled graph is only a proposal ranking. A low sampled matching
        // never rejects a core: the full global repair gets the final say.
        return -1000.0*std::min(matched,paired_slots+8)-2.0*matched-.04*edges;
    };
    auto make_mutation=[&](const Design& source,int j,std::array<char,6>& pattern){
        std::array<char,6> old{};for(int k=0;k<B;k++)old[k]=source.S_init[k][j];
        for(int attempt=0;attempt<40;attempt++){
            pattern=old;const unsigned kind=random32()%10;
            if(pattern_mode==4 || (pattern_mode==0 && B>=4 && kind>=7)){
                // Four opaque papers with two of each color partition the
                // uniform permutation space exactly in half, like a binary
                // comparison, while providing a different goal-set shape.
                pattern.fill('.');std::array<int,6> papers{};for(int k=0;k<B;k++)papers[k]=k;
                for(int k=B-1;k>0;k--)std::swap(papers[k],papers[random32()%(k+1)]);
                pattern[papers[0]]=pattern[papers[1]]='0';
                pattern[papers[2]]=pattern[papers[3]]='1';
            }else if(kind<5){
                int k=random32()%B;
                if(pattern[k]=='.')pattern[k]=char('0'+(random32()&1));
                else if(random32()%3==0)pattern[k]='.';
                else pattern[k]=char('0'+'1'-pattern[k]);
            }else if(kind<8){
                pattern.fill('.');std::array<int,6> papers{};for(int k=0;k<B;k++)papers[k]=k;
                for(int k=B-1;k>0;k--)std::swap(papers[k],papers[random32()%(k+1)]);
                int opaque=2+random32()%(std::min(B,4)-1);
                pattern[papers[0]]='0';pattern[papers[1]]='1';
                for(int k=2;k<opaque;k++)pattern[papers[k]]=char('0'+(random32()&1));
            }else if(kind==8){
                int x=random32()%B,y=random32()%B;std::swap(pattern[x],pattern[y]);
            }else{
                for(int k=0;k<B;k++)if(pattern[k]!='.')pattern[k]=char('0'+'1'-pattern[k]);
            }
            bool zero=false,one=false,different=false;
            for(int k=0;k<B;k++){zero|=pattern[k]=='0';one|=pattern[k]=='1';different|=pattern[k]!=old[k];}
            if(zero && one && different)return true;
        }
        return false;
    };
    while(calls<maximum_calls && !expired()){
        Design candidate=pending;auto candidate_display=pending_display;
        if(mutate_core){
        int selected_column=-1;double selected_quality=1e100;
        std::array<char,6> selected_pattern{};
        std::vector<int> modified;
        for(int j:columns)if(changed_column(pending,j))modified.push_back(j);
        for(int proposal=0;proposal<proposal_count && !expired();proposal++){
            int j;
            if(!modified.empty() && (int(modified.size())>=maximum_edits || random32()%3==0))j=modified[random32()%modified.size()];
            else j=columns[random32()%columns.size()];
            if(int(modified.size())>=maximum_edits && !changed_column(pending,j))continue;
            std::array<char,6> pattern{};if(!make_mutation(pending,j,pattern))continue;
            auto display=pending_display;const uint64_t bit=uint64_t(1)<<j;
            for(int s=0;s<table.states;s++){
                char color='0';
                for(int p=0;p<B;p++)if(pattern[table.permutations[s][p]]!='.'){
                    color=pattern[table.permutations[s][p]];break;
                }
                display[s]=(display[s]&~bit)|(color=='1'?bit:0);
            }
            proposals++;
            double score=(global_repair?global_quality(display):proposal_quality(pending,display))
                +search_budget::real("CORE_JOINT_NOISE",1.5)*(random01()-.5);
            if(score<selected_quality){
                selected_quality=score;selected_column=j;selected_pattern=pattern;candidate_display=std::move(display);
            }
        }
        if(selected_column<0 || expired())break;
        for(int k=0;k<B;k++)candidate.S_init[k][selected_column]=selected_pattern[k];
        }
        // Existing products are allowed to have zero goals at this point.
        // Repair may select different partners and assign templates to slots.
        const uint64_t seed_high=random32(),seed_low=random32();
        const uint64_t repair_seed=(seed_high<<32)|seed_low;
        auto result=global_repair?
            structural_global_pair_repair::optimize(candidate,candidate_display,repair_deadline(),repair_seed):
            structural_pair_repair::optimize(candidate,candidate_display,repair_deadline(),repair_iterations,repair_seed);
        calls++;
        if(!result.feasible){
            failed++;chain++;
            if(chain<maximum_chain){pending=std::move(candidate);pending_display=std::move(candidate_display);}
            else {pending=current;pending_display=current_display;chain=0;}
            continue;
        }
        feasible++;
        if(result.cost<best_cost-1e-9){best_cost=result.cost;best=candidate;improved++;}
        bool different_core=false;
        for(int j:columns)different_core|=changed_column(candidate,j);
        bool has_ternary=false;
        for(int j:columns){int count=0;for(int k=0;k<B;k++)count+=candidate.S_init[k][j]!='.';has_ternary|=count>=3;}
        if(different_core && (!require_ternary || has_ternary) && result.cost<best_mutated_cost){best_mutated_cost=result.cost;best_mutated=candidate;}
        const double elapsed=double(clock()-started)/std::max(1.0,double(deadline-started));
        const double heat=temperature*std::pow(.05,std::min(1.0,elapsed));
        const bool take=result.cost<=current_cost || (heat>0 && random01()<std::exp((current_cost-result.cost)/heat));
        if(take){current=std::move(candidate);current_display=std::move(candidate_display);current_cost=result.cost;accepted++;}
        pending=current;pending_display=current_display;chain=0;
    }
    bool returned_mutated=false;
    if(return_mutated && std::isfinite(best_mutated_cost) && best_mutated_cost<=baseline.cost+mutated_slack){
        d=std::move(best_mutated);returned_mutated=true;
    }else if(best_cost<baseline.cost-1e-9)d=std::move(best);
    int changed_columns=0;for(int j:columns)changed_columns+=changed_column(d,j);
    int generalized_columns=0;
    for(int j:columns){int count=0;for(int k=0;k<B;k++)count+=d.S_init[k][j]!='.';generalized_columns+=count>=3;}
    std::vector<std::vector<int>> old_owners(original.S_init.size()),new_owners(d.S_init.size());
    for(int a=0;a<P0;a++){old_owners[original.prod_to_paper[a]].push_back(a);new_owners[d.prod_to_paper[a]].push_back(a);}
    int changed_pairs=0;
    for(int k=B;k<int(d.S_init.size());k++)if(d.paper_is_paired[k])changed_pairs+=old_owners[k]!=new_owners[k];
    std::cerr<<"CORE_JOINT B="<<B<<" mutate="<<mutate_core<<" global="<<global_repair<<" pattern="<<pattern_mode
             <<" calls="<<calls<<" failed="<<failed<<" feasible="<<feasible
             <<" accepted="<<accepted<<" improved="<<improved<<" proposals="<<proposals
             <<" cost_before="<<baseline.cost/P0<<" cost_after="<<(returned_mutated?best_mutated_cost:best_cost)/P0
             <<" best_proxy="<<best_cost/P0<<" best_mutated="<<best_mutated_cost/P0<<" returned_mutated="<<returned_mutated
             <<" probe_evaluations="<<global_probe_evaluations<<" probe_edges="<<global_probe_edges<<" probe_matching="<<global_probe_match
             <<" changed_columns="<<changed_columns<<" generalized_columns="<<generalized_columns<<" changed_pairs="<<changed_pairs
             <<" timeout="<<expired()<<" cpu="<<double(clock()-started)/CLOCKS_PER_SEC<<"\n";
}
} // namespace structural_joint_core

static void optimize_joint_core(Design& chosen,clock_t deadline) {
    structural_joint_core::optimize(chosen,deadline);
}

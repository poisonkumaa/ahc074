// Include after Design, ExactBackingPermutations, and search_budget.
// Only paired primary suffix cells are erased. Their owners already use the
// core router, and every accepted edit preserves a core completion for each.
namespace suffix_hole_optimizer {
struct Result {
    int edits=0,candidates=0,feasible=0,bfs_runs=0;
    double foreign_gain=0,owner_loss=0,cpu=0;
    bool timed_out=false;
};

static Result optimize(Design& d,clock_t absolute_deadline=0) {
    Result answer;
    if(!search_budget::integer("SUFFIX_HOLES",0))return answer;
    const clock_t started=clock();
    clock_t deadline=started+clock_t(search_budget::seconds(std::max(0.0,
        search_budget::real("SUFFIX_HOLE_CPU",.25)))*CLOCKS_PER_SEC);
    if(absolute_deadline>0)deadline=std::min(deadline,absolute_deadline);
    auto expired=[&](){return clock()>=deadline;};
    const int B=d.tot_B,products=int(d.internal_catalog.size()),papers=int(d.S_init.size());
    if(B<3 || B>6 || d.C!=0 || d.T<1 || d.T>=64 || expired())return answer;
    const auto& table=ExactBackingPermutations::get(B);
    const int states=table.states;
    const auto& A=d.internal_catalog;
    const uint64_t prefix=(1ULL<<d.T)-1,suffix=~prefix;
    const int samples=std::clamp(search_budget::integer("SUFFIX_HOLE_SAMPLES",16),1,std::min(32,states));
    const int max_edits=std::max(0,search_budget::integer("SUFFIX_HOLE_EDITS",10));
    const int max_per_paper=std::max(1,search_budget::integer("SUFFIX_HOLE_PER_PAPER",3));
    const double move_cost=std::max(0.0,search_budget::real("SUFFIX_HOLE_DONOR_COST",2.0));
    const double owner_weight=std::max(0.0,search_budget::real("SUFFIX_HOLE_OWN",1.0));
    const double minimum_gain=std::max(0.0,search_budget::real("SUFFIX_HOLE_GAIN",.01));
    std::vector<uint64_t> ink(papers),black(papers),holes(products),displayed(states);
    std::vector<std::array<uint64_t,9>> cover(states);
    std::vector<std::vector<int>> owners(papers);
    std::vector<int> bucket_size(d.T),used(papers);
    std::array<double,64> one_probability{};
    for(int k=0;k<papers;k++)for(int j=0;j<64;j++) {
        if(d.S_init[k][j]!='.')ink[k]|=1ULL<<j;
        if(d.S_init[k][j]=='1')black[k]|=1ULL<<j;
    }
    for(int a=0;a<products;a++) {
        int main=d.prod_to_paper[a];owners[main].push_back(a);holes[a]=suffix&~ink[main];
        for(int j=0;j<d.T;j++)one_probability[j]+=double(A[a]>>j&1)/products;
    }
    for(int k=B;k<papers;k++)bucket_size[d.paper_bucket[k]]++;
    for(int s=0;s<states;s++) {
        cover[s][0]=0;
        for(int i=0;i<B;i++) {
            int k=table.permutations[s][i];
            displayed[s]|=black[k]&~cover[s][i];cover[s][i+1]=cover[s][i]|ink[k];
        }
    }
    // A deterministic core-state sample makes before/after donor costs paired.
    std::vector<int> starts(states);for(int s=0;s<states;s++)starts[s]=s;
    uint64_t random_state=0x8b6af129e534c07dULL^uint64_t(B);
    auto random=[&](){random_state^=random_state<<7;random_state^=random_state>>9;return uint32_t(random_state);};
    for(int i=states-1;i>0;i--)std::swap(starts[i],starts[random()%(i+1)]);
    std::vector<uint8_t> distance_sample(size_t(samples)*states);
    for(int h=0;h<samples;h++) {
        std::array<uint8_t,9> position{},relative{};
        for(int i=0;i<B;i++)position[table.permutations[starts[h]][i]]=uint8_t(i);
        for(int s=0;s<states;s++) {
            for(int i=0;i<B;i++)relative[i]=position[table.permutations[s][i]];
            distance_sample[size_t(h)*states+s]=table.distance[table.rank(relative)];
        }
    }
    struct OwnCost {double mean=0;std::array<uint8_t,32> sampled{};};
    std::unordered_map<std::string,OwnCost> cost_cache;
    std::vector<uint8_t> distance(states);
    std::vector<int> queue;queue.reserve(states);
    auto own_cost=[&](const std::vector<int>& goals,OwnCost& result)->bool {
        if(goals.empty() || expired())return false;
        std::string key((states+7)/8,char(0));
        for(int s:goals)key[s/8]=char(static_cast<unsigned char>(key[s/8])|(1u<<(s%8)));
        auto found=cost_cache.find(key);if(found!=cost_cache.end()){result=found->second;return true;}
        std::fill(distance.begin(),distance.end(),uint8_t(255));queue.clear();
        for(int s:goals){distance[s]=0;queue.push_back(s);}
        int sum=0;
        for(size_t i=0;i<queue.size();i++) {
            if((i&255)==0 && expired())return false;
            int s=queue[i],next=distance[s]+1;
            for(int prev:table.reverse_edges[s])if(distance[prev]==255){distance[prev]=uint8_t(next);sum+=next;queue.push_back(prev);}
        }
        if(int(queue.size())!=states)return false;
        result.mean=double(sum)/states;
        for(int h=0;h<samples;h++)result.sampled[h]=distance[starts[h]];
        cost_cache.emplace(std::move(key),result);answer.bfs_runs++;return true;
    };
    std::vector<std::vector<int>> goals(products);
    std::vector<OwnCost> owned(products);
    for(int a=0;a<products;a++) {
        for(int s=0;s<states;s++)if(!((displayed[s]^A[a])&holes[a]))goals[a].push_back(s);
        if(!own_cost(goals[a],owned[a]))return answer;
    }
    struct Donor {int target,paper;double credit,access;};
    std::vector<Donor> donors;
    std::vector<std::vector<int>> donor_of_paper(papers),donor_of_target(products);
    for(int a=0;a<products;a++)if(holes[a]) {
        int main=d.prod_to_paper[a],home=d.paper_bucket[main];
        uint64_t prefix_holes=prefix&~ink[main];
        for(int k=B;k<papers;k++)if(k!=main) {
            int w=d.paper_bucket[k];if(w==home)continue;
            if((ink[k]>>home&1) && (ink[main]>>w&1))continue;
            if((black[k]^A[a])&ink[k]&prefix_holes)continue;
            double gain=0;
            for(uint64_t bits=ink[k]&prefix_holes;bits;bits&=bits-1) {
                int j=__builtin_ctzll(bits);gain+=(A[a]>>j&1)?1-one_probability[j]:one_probability[j];
            }
            // A random bank head exposes this donor within the first three
            // positions with this probability. The credit includes estimated
            // extraction/return work; goal distances below are exact per sample.
            double access=std::min(1.0,3.0/std::max(1,bucket_size[w]));
            int id=int(donors.size());donors.push_back({a,k,gain-move_cost,access});
            donor_of_paper[k].push_back(id);donor_of_target[a].push_back(id);
        }
    }
    auto utility=[&](const Donor& donor,uint64_t opaque)->std::array<double,32> {
        const int a=donor.target,k=donor.paper;
        const uint64_t mask=opaque&holes[a],bad=(black[k]^A[a])&mask;
        std::array<uint8_t,32> best;best.fill(uint8_t(B));
        bool feasible=false;
        for(int s=0;s<states;s++) {
            const uint64_t wrong=(displayed[s]^A[a])&holes[a];
            if(wrong&~mask)continue;
            int depth=B;while(wrong&cover[s][depth])--depth;
            if(bad&~cover[s][depth])continue;
            feasible=true;
            for(int h=0;h<samples;h++)best[h]=std::min(best[h],distance_sample[size_t(h)*states+s]);
        }
        std::array<double,32> result{};
        if(!feasible)return result;
        for(int h=0;h<samples;h++)result[h]=donor.access*std::max(0.0,double(owned[a].sampled[h])-best[h]+donor.credit);
        return result;
    };
    struct Top {double first=0,second=0;int paper=-1;};
    std::vector<std::array<Top,32>> top(products);
    for(int step=0;step<max_edits;step++) {
        if(expired()){answer.timed_out=true;break;}
        for(auto& row:top)row={};
        bool complete=true;
        for(const auto& donor:donors) {
            if(expired()){complete=false;break;}
            auto u=utility(donor,ink[donor.paper]);
            for(int h=0;h<samples;h++) {
                auto& t=top[donor.target][h];
                if(u[h]>t.first){t.second=t.first;t.first=u[h];t.paper=donor.paper;}
                else if(u[h]>t.second)t.second=u[h];
            }
        }
        if(!complete){answer.timed_out=true;break;}
        int best_paper=-1,best_column=-1;
        double best_net=minimum_gain,best_gain=0,best_loss=0;
        std::array<OwnCost,2> best_owned;
        std::array<std::vector<int>,2> best_goals;
        for(int k=B;k<papers;k++)if(owners[k].size()==2 && d.paper_is_paired[k] && used[k]<max_per_paper) {
            for(uint64_t bits=ink[k]&suffix;bits;bits&=bits-1) {
                if(expired()){answer.timed_out=true;complete=false;break;}
                int j=__builtin_ctzll(bits);uint64_t bit=1ULL<<j;answer.candidates++;
                std::array<std::vector<int>,2> new_goals;
                bool valid=true;
                for(int side=0;side<2;side++) {
                    int a=owners[k][side];
                    for(int s:goals[a])if(!((displayed[s]^A[a])&bit))new_goals[side].push_back(s);
                    if(new_goals[side].empty()){valid=false;break;}
                }
                if(!valid)continue;answer.feasible++;
                double gain=0;
                for(int id:donor_of_paper[k]) {
                    const auto& donor=donors[id];auto changed=utility(donor,ink[k]&~bit);
                    for(int h=0;h<samples;h++) {
                        const auto& t=top[donor.target][h];
                        double without=t.paper==k?t.second:t.first;
                        gain+=(std::max(without,changed[h])-t.first)/samples;
                    }
                }
                // Erasure can only increase an owner's core distance, so this
                // foreign gain is an upper bound before the two exact BFSs.
                if(gain<=best_net)continue;
                double loss=0;std::array<OwnCost,2> changed_owned;
                for(int side=0;side<2;side++) {
                    int a=owners[k][side];
                    if(!own_cost(new_goals[side],changed_owned[side])){complete=false;break;}
                    loss+=changed_owned[side].mean-owned[a].mean;
                }
                if(!complete){answer.timed_out=true;break;}
                double net=gain-owner_weight*loss;
                if(net>best_net){best_net=net;best_paper=k;best_column=j;best_gain=gain;best_loss=loss;
                    best_owned=changed_owned;best_goals=std::move(new_goals);}
            }
            if(!complete)break;
        }
        if(best_paper<0)break;
        // Each edit is a complete valid design even when the next sweep stops.
        const uint64_t bit=1ULL<<best_column;
        d.S_init[best_paper][best_column]='.';ink[best_paper]&=~bit;used[best_paper]++;
        for(int side=0;side<2;side++) {
            int a=owners[best_paper][side];holes[a]|=bit;
            owned[a]=best_owned[side];goals[a]=std::move(best_goals[side]);
        }
        answer.edits++;answer.foreign_gain+=best_gain;answer.owner_loss+=best_loss;
        if(!complete)break;
    }
    answer.cpu=double(clock()-started)/CLOCKS_PER_SEC;
    std::cerr<<"SUFFIX_HOLES edits="<<answer.edits<<" candidates="<<answer.candidates
        <<" feasible="<<answer.feasible<<" foreign="<<answer.foreign_gain
        <<" own="<<answer.owner_loss<<" bfs="<<answer.bfs_runs
        <<" timeout="<<answer.timed_out<<" cpu="<<answer.cpu<<"\n";
    return answer;
}
} // namespace suffix_hole_optimizer

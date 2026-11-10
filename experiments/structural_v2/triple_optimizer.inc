// Post-design structural experiment: two pairs become a triple and a singleton.
// Run after all two-owner template refinements and before creating a fresh router.
namespace triple_group_optimizer {
struct Proposal {
    int triple_paper=-1,single_paper=-1,single_product=-1;
    std::array<int,3> triple{};
    double hint=0,delta=0,prefix_delta=0,retrieval_delta=0;
};

static void optimize(Design& d,clock_t absolute_deadline) {
    if(!search_budget::integer("TRIPLE_GROUPS",0))return;
    const clock_t started=clock();
    clock_t deadline=started+clock_t(search_budget::seconds(std::max(0.0,
        search_budget::real("TRIPLE_GROUP_CPU",.20)))*CLOCKS_PER_SEC);
    if(absolute_deadline>0)deadline=std::min(deadline,absolute_deadline);
    auto expired=[&](){return clock()>=deadline;};
    const int B=d.tot_B,T=d.T;
    if((B!=5 && B!=6) || d.C!=0 || T<=0 || T>=64 || expired())return;
    const auto& A=d.internal_catalog;
    const auto& table=ExactBackingPermutations::get(B);
    const int states=table.states;
    const uint64_t prefix=(uint64_t(1)<<T)-1,suffix=~prefix;
    const double prefix_weight=search_budget::real("TRIPLE_PREFIX_WEIGHT",.5);
    const double retrieval_weight=search_budget::real("TRIPLE_RETRIEVAL_WEIGHT",1.0);
    const int shortlist=std::max(1,search_budget::integer("TRIPLE_SHORTLIST",96));
    const int max_changes=std::max(0,search_budget::integer("TRIPLE_MAX",8));
    std::vector<std::vector<int>> owned(K);
    std::array<int,64> bucket_size{};
    std::vector<int> paired;
    std::array<uint64_t,K> ink{};
    for(int a=0;a<int(A.size());a++)owned[d.prod_to_paper[a]].push_back(a);
    for(int k=0;k<K;k++){
        for(int j=0;j<64;j++)if(d.S_init[k][j]!='.')ink[k]|=uint64_t(1)<<j;
        if(k>=B){++bucket_size[d.paper_bucket[k]];if(owned[k].size()==2)paired.push_back(k);}
    }
    std::array<uint64_t,8> core_black{};
    for(int k=0;k<B;k++)for(int j=T;j<64;j++)
        if(d.S_init[k][j]=='1')core_black[k]|=uint64_t(1)<<j;
    std::vector<uint64_t> displayed(states);
    for(int id=0;id<states;id++){
        uint64_t seen=0;
        for(int j=0;j<B;j++){
            const int k=table.permutations[id][j];
            displayed[id]|=core_black[k]&~seen;seen|=ink[k];
        }
    }
    auto goals=[&](int a,uint64_t holes,std::vector<int>* out=nullptr){
        int count=0;if(out)out->clear();
        for(int id=0;id<states;id++)if(!((displayed[id]^A[a])&holes)){
            ++count;if(out)out->push_back(id);
        }
        return count;
    };
    std::vector<int> old_count(A.size());
    for(int a=0;a<int(A.size());a++)old_count[a]=goals(a,suffix&~ink[d.prod_to_paper[a]]);
    for(int count:old_count)if(!count)return;
    auto proxy=[&](int count){return std::log(double(states)/count)/std::log(double(B));};
    std::vector<Proposal> candidates;
    long long pair_trials=0,anchor_valid=0,feasible=0,scored=0,bfs_runs=0;
    for(int x=0;x<int(paired.size()) && !expired();x++)
        for(int y=x+1;y<int(paired.size()) && !expired();y++){
            ++pair_trials;
            const int p=paired[x],q=paired[y];
            std::array<int,4> products{owned[p][0],owned[p][1],owned[q][0],owned[q][1]};
            const double old_prefix=prefix_weight*(2*__builtin_popcountll(prefix&~ink[p])+
                                                    2*__builtin_popcountll(prefix&~ink[q]));
            double old_proxy=0;for(int a:products)old_proxy+=proxy(old_count[a]);
            for(int leave=0;leave<4 && !expired();leave++){
                const int single=products[leave];
                std::array<int,3> group{};int at=0;
                for(int i=0;i<4;i++)if(i!=leave)group[at++]=products[i];
                const uint64_t difference=(A[group[0]]^A[group[1]])|(A[group[0]]^A[group[2]]);
                std::array<int,3> counts{};bool checked=false,possible=true;
                for(int orientation=0;orientation<2;orientation++){
                    const int t=orientation?p:q,s=orientation?q:p;
                    const int wt=d.paper_bucket[t],ws=d.paper_bucket[s];
                    const int ct=d.S_init[t][wt]-'0',cs=d.S_init[s][ws]-'0';
                    if((difference>>wt&1) || int(A[group[0]]>>wt&1)!=ct || int(A[single]>>ws&1)!=cs)continue;
                    ++anchor_valid;
                    if(!checked){
                        for(int j=0;j<3;j++){counts[j]=goals(group[j],difference&suffix);possible&=counts[j]>0;}
                        checked=true;
                    }
                    if(!possible)continue;
                    ++feasible;
                    const uint64_t low_t=(uint64_t(1)<<(wt+1))-1,low_s=(uint64_t(1)<<(ws+1))-1;
                    const uint64_t triple_ink=(low_t|suffix)&~difference;
                    const uint64_t singleton_ink=low_s|suffix;
                    const double prefix_delta=prefix_weight*(3*__builtin_popcountll(prefix&~triple_ink)+
                        __builtin_popcountll(prefix&~singleton_ink))-old_prefix;
                    // The bucket populations do not change. This is the change
                    // in uniformly distributed retrieval rank, weighted by each
                    // paper's actual number of products (3/1 instead of 2/2).
                    const double retrieval_delta=retrieval_weight*.5*(bucket_size[wt]-bucket_size[ws]);
                    double hint=prefix_delta+retrieval_delta-old_proxy;
                    for(int count:counts)hint+=proxy(count);
                    candidates.push_back({t,s,single,group,hint,0,prefix_delta,retrieval_delta});
                }
            }
        }
    std::stable_sort(candidates.begin(),candidates.end(),[](const auto& a,const auto& b){return a.hint<b.hint;});
    if(int(candidates.size())>shortlist)candidates.resize(shortlist);
    std::unordered_map<std::string,double> mean_cache;
    auto mean_distance=[&](int a,uint64_t holes)->double {
        std::vector<int> target;goals(a,holes,&target);
        if(target.empty())return 1.e30;
        if(int(target.size())==states)return 0;
        std::string key((states+7)/8,char(0));
        for(int id:target)key[id/8]=char(static_cast<unsigned char>(key[id/8])|(1u<<(id%8)));
        auto saved=mean_cache.find(key);if(saved!=mean_cache.end())return saved->second;
        if(expired())return 1.e30;
        std::vector<uint8_t> distance(states,255);
        std::vector<int> queue;queue.reserve(states);
        for(int id:target){distance[id]=0;queue.push_back(id);}
        int sum=0;
        for(size_t head=0;head<queue.size();head++){
            if((head&127)==0 && expired())return 1.e30;
            const int id=queue[head];
            for(int next:table.reverse_edges[id])if(distance[next]==255){
                distance[next]=distance[id]+1;sum+=distance[next];queue.push_back(next);
            }
        }
        if(int(queue.size())!=states)return 1.e30;
        ++bfs_runs;double result=double(sum)/states;mean_cache.emplace(std::move(key),result);return result;
    };
    std::vector<Proposal> improvements;
    for(auto candidate:candidates){
        if(expired())break;
        double old_core=0,new_core=0;
        for(int paper:{candidate.triple_paper,candidate.single_paper})
            for(int a:owned[paper])old_core+=mean_distance(a,suffix&~ink[paper]);
        const auto& group=candidate.triple;
        const uint64_t difference=(A[group[0]]^A[group[1]])|(A[group[0]]^A[group[2]]);
        for(int a:group)new_core+=mean_distance(a,difference&suffix);
        if(old_core>1.e20 || new_core>1.e20)break;
        candidate.delta=candidate.prefix_delta+candidate.retrieval_delta+new_core-old_core;
        ++scored;
        if(candidate.delta<-1.e-9)improvements.push_back(candidate);
    }
    std::stable_sort(improvements.begin(),improvements.end(),[](const auto& a,const auto& b){return a.delta<b.delta;});
    std::array<bool,K> used{};
    const Design original=d;
    int accepted=0;double gain=0;
    auto paint=[&](int paper,const std::vector<int>& products){
        const int w=d.paper_bucket[paper];
        std::string s(64,'.');
        for(int j=0;j<64;j++)if(j<=w || j>=T){
            int bit=int(A[products[0]]>>j&1);bool same=true;
            for(int a:products)same&=int(A[a]>>j&1)==bit;
            if(same)s[j]=char('0'+bit);
        }
        d.S_init[paper]=std::move(s);
    };
    for(const auto& candidate:improvements){
        if(accepted>=max_changes || used[candidate.triple_paper] || used[candidate.single_paper])continue;
        const int t=candidate.triple_paper,s=candidate.single_paper;
        const int marker=d.prod_to_pair_idx[owned[t][0]];
        if(marker<0)continue;
        std::vector<int> group(candidate.triple.begin(),candidate.triple.end());
        paint(t,group);paint(s,{candidate.single_product});
        for(int a:group){d.prod_to_paper[a]=t;d.prod_to_pair_idx[a]=marker;}
        d.prod_to_paper[candidate.single_product]=s;d.prod_to_pair_idx[candidate.single_product]=-1;
        d.paper_is_paired[t]=true;d.paper_is_paired[s]=false;
        // Runtime routing uses this only as a shared-primary marker. No
        // two-owner construction or BankTemplates pass may follow this step.
        d.pairs[marker].u=group[0];d.pairs[marker].v=group[1];
        d.pairs[marker].paper_id=t;d.pairs[marker].bucket_w=d.paper_bucket[t];
        used[t]=used[s]=true;++accepted;gain-=candidate.delta;
    }
    bool valid=true;
    for(int a=0;a<int(A.size());a++){
        const int k=d.prod_to_paper[a],w=d.paper_bucket[k];uint64_t need=0;
        valid&=d.S_init[k][w]==char('0'+int(A[a]>>w&1));
        for(int j=0;j<64;j++)if(d.S_init[k][j]!='.')
            valid&=d.S_init[k][j]==char('0'+int(A[a]>>j&1));
        for(int j=T;j<64;j++)if(d.S_init[k][j]=='.')need|=uint64_t(1)<<j;
        valid&=goals(a,need)>0;
    }
    if(!valid){d=original;accepted=0;gain=0;}
    std::cerr<<"TRIPLE_GROUP B="<<B<<" pair_trials="<<pair_trials<<" anchor_valid="<<anchor_valid
        <<" feasible="<<feasible<<" shortlisted="<<candidates.size()<<" scored="<<scored
        <<" improving="<<improvements.size()<<" accepted="<<accepted<<" rollback="<<!valid
        <<" modeled_gain="<<gain<<" modeled_saved_per_case="<<gain*R/P<<" bfs="<<bfs_runs
        <<" timeout="<<expired()<<" cpu="<<double(clock()-started)/CLOCKS_PER_SEC<<"\n";
}
} // namespace triple_group_optimizer

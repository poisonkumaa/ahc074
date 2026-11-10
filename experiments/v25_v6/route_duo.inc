// Included inside nov5_relaxed_trace, immediately after rebuild_state.
// Two complementary donors reuse their last paid operations. No new move is
// assumed free: every resulting trace is replayed from the original deck.
namespace paid_duo {
struct Statistics {
    long long offered=0,kept=0,searched=0,pairs=0,replays=0,legal=0;
    long long projection_failures=0,wins=0,saved=0,verified=0;
    double cpu=0;
};
inline Statistics& stats(){static Statistics x;return x;}
struct Config {
    bool enabled=search_budget::integer("NOV5_DUO",0)!=0;
    bool verify=search_budget::integer("NOV5_DUO_VERIFY",0)!=0;
    double budget=std::max(0.,search_budget::real("NOV5_DUO_CPU",.03));
    int choices=std::clamp(search_budget::integer("NOV5_DUO_CHOICES",6),2,12);
};
inline const Config& config(){static Config x;return x;}
struct Charge {
    clock_t began=clock();
    ~Charge(){stats().cpu+=double(clock()-began)/CLOCKS_PER_SEC;}
};
struct Seed {
    std::array<uint8_t,K> order;
    std::vector<std::pair<int,int>> moves;
    std::array<int,K> last;
    uint64_t bad=0;
    std::pair<int,int> priority;
};
struct Pool {
    std::array<Seed,2> seeds;
    int size=0;
    template<class Order,class Ops>
    void offer(const OnlineRouter& router,int product,const Order& order,
               const Ops& moves,int count,uint64_t bad) {
        const auto& cfg=config();
        if(!cfg.enabled || stats().cpu>=cfg.budget || __builtin_popcountll(bad)<2)return;
        const std::pair<int,int> priority={count,__builtin_popcountll(bad)};
        int slot=size;
        if(size==2){slot=seeds[0].priority<seeds[1].priority?1:0;if(priority>seeds[slot].priority)return;}
        Charge charge;stats().offered++;
        for(int i=0;i<size;i++)if(seeds[i].priority==priority && seeds[i].order==order)return;
        std::array<int,K> last;last.fill(-1);
        for(int i=0;i<count;i++)last[moves[i].first]=i;
        const int main=router.prod_to_paper[product],home=router.paper_bucket[main];
        uint64_t possible=0;int useful=0;
        for(int k=router.tot_B;k<K;k++)if(last[k]>=0 && k!=main && router.paper_bucket[k]!=home){
            uint64_t cover=bad&router.opaque[k]&~(router.black[k]^router.A[product]);
            if(cover){possible|=cover;useful++;}
        }
        if(useful<2 || (possible&bad)!=bad)return;
        auto& seed=seeds[slot];seed.order=order;seed.moves.assign(moves.begin(),moves.begin()+count);
        seed.last=last;seed.bad=bad;seed.priority=priority;
        if(size<2)size++;stats().kept++;
    }

    static uint64_t display(const OnlineRouter& router,const std::array<uint8_t,K>& order){
        uint64_t seen=0,value=0;
        for(int p:order){value|=router.black[p]&~seen;seen|=router.opaque[p];if(seen==~0ULL)break;}
        return value;
    }
    static void relocate(std::array<uint8_t,K>& order,int from,int to){
        int paper=order[from];
        if(from<to)std::memmove(order.data()+from,order.data()+from+1,to-from);
        else if(to<from)std::memmove(order.data()+to+1,order.data()+to,from-to);
        order[to]=paper;
    }
    static void verify(const OnlineRouter& router,int product,const std::vector<int>& start,
                       const std::vector<std::pair<int,int>>& moves,
                       const std::array<uint8_t,K>& wanted){
        std::vector<int> order=start;
        for(auto [p,d]:moves){
            auto it=std::find(order.begin(),order.end(),p);
            if(it==order.end() || d<0 || d>=K)std::abort();
            uint64_t free=router.opaque[p];
            for(auto j=order.begin();j!=it;j++)free&=~router.opaque[*j];
            if(!free)std::abort();
            order.erase(it);order.insert(order.begin()+d,p);
        }
        if(!std::equal(order.begin(),order.end(),wanted.begin()))std::abort();
        uint64_t seen=0,value=0;
        for(int p:order){value|=router.black[p]&~seen;seen|=router.opaque[p];}
        if(value!=router.A[product])std::abort();
        stats().verified++;
    }

    // Ordinary papers retain the exact projection of the already legal seed
    // trace. The two donor positions are the only new degrees of freedom.
    static bool replay(const OnlineRouter& router,const std::vector<int>& start,
                       const Seed& seed,const std::array<uint8_t,K>& desired,
                       int a,int b,std::vector<std::pair<int,int>>& output){
        std::array<uint8_t,K> current,reference;
        std::array<int,K> goal_position;
        for(int i=0;i<K;i++){current[i]=reference[i]=start[i];goal_position[desired[i]]=i;}
        auto donor=[&](int p){return p==a || p==b;};
        output.clear();output.reserve(seed.moves.size());
        for(int t=0;t<int(seed.moves.size());t++){
            auto [p,original_depth]=seed.moves[t];
            int original_from=std::find(reference.begin(),reference.end(),p)-reference.begin();
            relocate(reference,original_from,original_depth);
            int from=std::find(current.begin(),current.end(),p)-current.begin(),depth=-1;
            if(donor(p) && seed.last[p]==t){
                // The nearest final successor with no remaining operation is
                // a stable anchor. Later ordinary moves are placed on the
                // required side of this donor within their projected gap.
                int right=-1;
                for(int j=goal_position[p]+1;j<K;j++)if(seed.last[desired[j]]<=t){right=desired[j];break;}
                depth=right<0?K-1:int(std::find(current.begin(),current.end(),right)-current.begin());
                if(right>=0)depth-=from<depth;
            }else{
                int preceding=0;
                for(int q:reference){if(q==p)break;if(!donor(q))preceding++;}
                auto remaining=current;
                std::memmove(remaining.data()+from,remaining.data()+from+1,K-1-from);
                int at=0,ordinary=0;
                while(at<K-1 && ordinary<preceding){ordinary+=!donor(remaining[at]);at++;}
                if(ordinary!=preceding)return false;
                while(at<K-1 && donor(remaining[at]) && goal_position[remaining[at]]<goal_position[p])at++;
                depth=at;
            }
            if(depth<0 || depth>=K)return false;
            if(from==depth)continue;
            uint64_t free=router.opaque[p];
            for(int i=0;i<from && free;i++)free&=~router.opaque[current[i]];
            if(!free)return false;
            relocate(current,from,depth);output.emplace_back(p,depth);
        }
        if(current!=desired){stats().projection_failures++;return false;}
        return true;
    }

    void run(OnlineRouter& router,int product,const std::vector<int>& start,
             clock_t outer_deadline,int max_supports,
             const OnlineRouter::RouteState& restore_to,double& best_score,
             OnlineRouter::RouteState& best_state,std::vector<std::pair<int,int>>& best_moves){
        const auto& cfg=config();
        if(!cfg.enabled || !size || stats().cpu>=cfg.budget)return;
        Charge charge;
        const clock_t deadline=std::min(outer_deadline,charge.began+clock_t((cfg.budget-stats().cpu)*CLOCKS_PER_SEC));
        if(clock()>=deadline)return;
        if(size==2 && seeds[1].priority<seeds[0].priority)std::swap(seeds[0],seeds[1]);
        const int main=router.prod_to_paper[product],home=router.paper_bucket[main];
        const uint64_t target=router.A[product];
        struct Placement {int paper,anchor,depth;uint64_t cover;};
        for(int s=0;s<size;s++){
            if(clock()>=deadline)return;
            const auto& seed=seeds[s];
            if(seed.moves.size()>=best_moves.size())continue;
            stats().searched++;
            std::array<uint64_t,K+1> prefix{};
            std::array<int,K> position;
            for(int i=0;i<K;i++){prefix[i+1]=prefix[i]|router.opaque[seed.order[i]];position[seed.order[i]]=i;}
            std::vector<Placement> choices;
            for(int p=router.tot_B;p<K;p++)if(seed.last[p]>=0 && p!=main && router.paper_bucket[p]!=home){
                Placement best{p,-1,-1,0};
                const uint64_t wrong=router.opaque[p]&(router.black[p]^target);
                for(int d=position[p]-1;d>=1;d--){
                    const int upper=seed.order[d-1],lower=seed.order[d];
                    if(upper>=router.tot_B && lower>=router.tot_B && router.paper_bucket[upper]==router.paper_bucket[lower])continue;
                    if(wrong&~prefix[d])continue;
                    const uint64_t cover=seed.bad&router.opaque[p]&~prefix[d];
                    if(cover==seed.bad)continue; // Existing one-donor repair handles this case.
                    if(__builtin_popcountll(cover)>__builtin_popcountll(best.cover))best={p,lower,d,cover};
                }
                if(best.cover)choices.push_back(best);
            }
            std::stable_sort(choices.begin(),choices.end(),[](const auto& a,const auto& b){
                return __builtin_popcountll(a.cover)>__builtin_popcountll(b.cover);
            });
            if(int(choices.size())>cfg.choices)choices.resize(cfg.choices);
            for(int i=0;i<int(choices.size());i++)for(int j=i+1;j<int(choices.size());j++){
                if(clock()>=deadline)return;
                const auto a=choices[i],b=choices[j];
                if((a.cover|b.cover)!=seed.bad || router.paper_bucket[a.paper]==router.paper_bucket[b.paper] ||
                   a.anchor==b.paper || b.anchor==a.paper)continue;
                for(int direction=0;direction<(a.anchor==b.anchor?2:1);direction++){
                    if(clock()>=deadline)return;
                    stats().pairs++;
                    auto desired=seed.order;
                    Placement steps[2]={direction?b:a,direction?a:b};
                    for(auto placement:steps){
                        int from=std::find(desired.begin(),desired.end(),placement.paper)-desired.begin();
                        int to=std::find(desired.begin(),desired.end(),placement.anchor)-desired.begin();
                        to-=from<to;relocate(desired,from,to);
                    }
                    if(display(router,desired)!=target)continue;
                    OnlineRouter::RouteState candidate;
                    if(!rebuild_state(router,desired,product,candidate,max_supports))continue;
                    stats().replays++;
                    std::vector<std::pair<int,int>> output;
                    if(!replay(router,start,seed,desired,a.paper,b.paper,output))continue;
                    stats().legal++;
                    if(cfg.verify)verify(router,product,start,output,desired);
                    if(output.size()>=best_moves.size())continue;
                    router.restore_route_state(candidate);
                    const double value=output.size()+router.route_state_value();
                    router.restore_route_state(restore_to);
                    if(value<best_score-1e-9){
                        stats().wins++;stats().saved+=int(best_moves.size())-int(output.size());
                        best_score=value;best_state=std::move(candidate);best_moves=std::move(output);
                    }
                }
            }
        }
    }
};
} // namespace paid_duo

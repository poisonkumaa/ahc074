// Dynamic, within-order suffix beam. Include after nov5_relaxed_trace.
// The fixed prefix has already promoted the requested main and corrected the
// backing windows. Search then changes the ordinary suffix one legal move at
// a time; intermediate decks may need several repairs or contain extra loans.
// Only an exact target display with a reusable terminal deck can be committed.
namespace query_trace_beam {
struct Statistics {
    long long queries=0,calls=0,pivots=0,nodes=0,children=0,neutral=0;
    long long display_ok=0,state_ok=0,accepted=0,saved=0,timeout=0;
    long long structural_nodes=0,invalid_goals=0,home_returns=0;
    double cpu=0;
};
inline Statistics& stats(){static Statistics s;return s;}
constexpr int max_depth=8;
struct Node {
    std::array<uint8_t,K> order{},position{};
    std::array<std::pair<uint8_t,uint8_t>,max_depth> path{};
    uint64_t bad=0,hash=0;
    int rank=0,repairs=0;
    bool exploratory=false;
};
inline void move_paper(Node& n,int k,int d) {
    const int from=n.position[k];
    if(from<d)std::memmove(n.order.data()+from,n.order.data()+from+1,d-from);
    else if(d<from)std::memmove(n.order.data()+d+1,n.order.data()+d,from-d);
    n.order[d]=uint8_t(k);
    for(int i=std::min(from,d);i<=std::max(from,d);i++)n.position[n.order[i]]=uint8_t(i);
}
inline uint64_t rendered(const OnlineRouter& r,const Node& n) {
    uint64_t covered=0,black=0;
    for(int k:n.order){black|=r.black[k]&~covered;covered|=r.opaque[k];}
    return black;
}
inline uint64_t deck_hash(const Node& n) {
    uint64_t h=0x9e3779b97f4a7c15ULL;
    for(int i=0;i<K;i+=8) {
        uint64_t part=0;
        std::memcpy(&part,n.order.data()+i,std::min(8,K-i));
        h^=part+0x9e3779b97f4a7c15ULL+(h<<6)+(h>>2);
    }
    return h;
}
inline int difficulty(const OnlineRouter& r,const Node& n,uint64_t target) {
    uint64_t unresolved=n.bad;
    int wrong_layers=0;
    for(int k:n.order) {
        unresolved&=~(r.opaque[k]&~(r.black[k]^target));
        if(!unresolved)break;
        wrong_layers+=__builtin_popcountll(unresolved&r.opaque[k]);
    }
    return wrong_layers;
}
struct Structure {
    std::array<uint8_t,K> support{},late{},needs_return{};
    int repairs=0;
};
// The unmoved, non-loan sheets retain the pivot's sorted bank skeleton.
// Recognize sheets inserted too late separately: counting them as ordinary
// would incorrectly classify every intervening bank as an outstanding loan.
// This is a search priority, not a feasibility bound; rebuild_state remains
// the sole authority for accepting a terminal deck.
inline Structure structure(const OnlineRouter& r,const Node& n,int depth,
                           int border,int main,int max_supports,
                           const std::array<uint8_t,K>& pivot_loans) {
    Structure s;
    auto exceptional=pivot_loans;
    for(int i=0;i<depth;i++)exceptional[n.path[i].first]=1;
    int previous=r.C-1;
    for(int d=border;d<K;d++) {
        const int k=n.order[d],w=r.paper_bucket[k];
        if(exceptional[k]) {
            if(w<previous){s.late[k]=s.needs_return[k]=1;s.repairs++;}
        }else previous=w;
    }
    for(int d=1;d<border;d++)if(n.order[d]>=r.tot_B)s.support[n.order[d]]=1;
    int next=r.T;
    for(int d=K-1;d>=border;d--) {
        const int k=n.order[d],w=r.paper_bucket[k];
        if(s.late[k])continue;
        if(w>next)s.support[k]=1;
        else next=w;
    }
    std::array<int,K+1> next_regular{};next_regular[K]=r.T;
    for(int d=K-1;d>=0;d--) {
        const int k=n.order[d];next_regular[d]=next_regular[d+1];
        if(k!=main && k>=r.tot_B && !s.support[k] && !s.late[k])next_regular[d]=r.paper_bucket[k];
    }
    std::array<int,64> by_home{};
    previous=r.C-1;
    for(int d=1;d<K;d++) {
        const int k=n.order[d];if(k<r.tot_B || s.late[k])continue;
        const int w=r.paper_bucket[k];
        if(s.support[k]) {
            if(w==r.paper_bucket[main] || w<=previous || next_regular[d+1]<=previous) {
                s.needs_return[k]=1;s.repairs++;
            }else by_home[w]++;
        }else previous=w;
    }
    int distinct=0;
    for(int w=r.C;w<r.T;w++)if(by_home[w]) {
        distinct++;s.repairs+=by_home[w]-1;
    }
    s.repairs+=std::max(0,distinct-max_supports);
    for(int k=0;k<K;k++)if(s.support[k] &&
        (by_home[r.paper_bucket[k]]>1 || distinct>max_supports))s.needs_return[k]=1;
    return s;
}
inline std::vector<std::pair<int,int>> optimize(
        OnlineRouter& router,int product,const std::vector<int>& start,
        std::vector<std::pair<int,int>> moves,clock_t deadline) {
    static const bool enabled=search_budget::integer("TRACE_BEAM",0)!=0;
    if(!enabled)return moves;
    auto& log=stats();log.queries++;
    static const double budget=search_budget::seconds(search_budget::real("TRACE_BEAM_BUDGET",.25));
    static const double call_limit=search_budget::seconds(search_budget::real("TRACE_BEAM_CALL",.0008));
    static const int width=std::clamp(search_budget::integer("TRACE_BEAM_WIDTH",8),1,32);
    static const int depth_limit=std::clamp(search_budget::integer("TRACE_BEAM_DEPTH",5),1,max_depth);
    static const int donor_limit=std::clamp(search_budget::integer("TRACE_BEAM_DONORS",6),0,32);
    static const int explore_slots=std::clamp(search_budget::integer("TRACE_BEAM_EXPLORE",2),0,width-1);
    static const int worsen=std::clamp(search_budget::integer("TRACE_BEAM_WORSEN",2),0,8);
    static const int min_suffix=std::max(2,search_budget::integer("TRACE_BEAM_MIN_SUFFIX",3));
    static const int max_supports=std::clamp(search_budget::integer("TRACE_BEAM_SUPPORTS",4),0,12);
    static const int structure_weight=std::max(0,search_budget::integer("TRACE_BEAM_STRUCT_WEIGHT",96));
    static const int return_options=std::clamp(search_budget::integer("TRACE_BEAM_HOME_OPTIONS",2),1,8);
    if(int(moves.size())<min_suffix || log.cpu>=budget ||
       (deadline>0 && clock()>=deadline))return moves;
    const clock_t begun=clock();
    struct Charge {
        Statistics& log;clock_t start;
        ~Charge(){log.cpu+=double(clock()-start)/CLOCKS_PER_SEC;}
    } charge{log,begun};
    log.calls++;
    // Distribute optional work across actual orders, including cheap skipped
    // orders. This beam never reads or samples a subsequent order.
    const double left=std::max(0.0,budget-log.cpu);
    const double fair=2*left/std::max<long long>(1,R-log.queries+1);
    const double allowance=std::min({left,call_limit,std::max(search_budget::seconds(.00004),fair)});
    clock_t local_deadline=begun+std::max<clock_t>(1,clock_t(allowance*CLOCKS_PER_SEC));
    if(deadline>0)local_deadline=std::min(local_deadline,deadline);
    unsigned ticks=0;bool timed_out=false;
    auto expired=[&](bool force=false){
        if(force || !(ticks++&15))timed_out=clock()>=local_deadline;
        return timed_out;
    };
    const int main=router.prod_to_paper[product];
    const uint64_t target=router.A[product];
    const auto original_state=router.save_route_state();
    auto best_state=original_state;
    auto best_moves=moves;
    double best_score=moves.size()+router.route_state_value();
    Node initial;
    for(int d=0;d<K;d++){initial.order[d]=uint8_t(start[d]);initial.position[start[d]]=uint8_t(d);}
    int prefix=-1,border=K;
    std::array<uint8_t,K> pivot_loans{};
    for(int done=0;done+min_suffix<=int(moves.size());done++) {
        if(expired())break;
        if(initial.order[0]==main) {
            initial.bad=rendered(router,initial)^target;
            OnlineRouter::RouteState rebuilt;
            if(!(initial.bad&~router.index_main_mask) &&
               nov5_relaxed_trace::rebuild_state(router,initial.order,product,rebuilt,max_supports)) {
                border=1;
                for(int k=0;k<router.tot_B;k++)border=std::max(border,int(initial.position[k])+1);
                uint64_t fixed=0;
                for(int d=0;d<border;d++)fixed|=router.opaque[initial.order[d]];
                if(!(initial.bad&fixed)){
                    prefix=done;
                    for(int k:rebuilt.supports)pivot_loans[k]=1;
                    break;
                }
            }
        }
        const auto [k,d]=moves[done];move_paper(initial,k,d);
    }
    if(prefix<0){if(timed_out)log.timeout++;return moves;}
    log.pivots++;
    auto accept=[&](const Node& n,int depth)->bool {
        if(n.bad)return false;
        if(prefix+depth>=int(best_moves.size()))return true;
        log.display_ok++;
        OnlineRouter::RouteState candidate;
        if(!nov5_relaxed_trace::rebuild_state(router,n.order,product,candidate,max_supports)){
            log.invalid_goals++;return false;
        }
        log.state_ok++;
        router.restore_route_state(candidate);
        const double score=prefix+depth+router.route_state_value();
        router.restore_route_state(original_state);
        if(score+1e-9>=best_score)return true;
        best_score=score;best_state=std::move(candidate);
        best_moves.assign(moves.begin(),moves.begin()+prefix);
        for(int i=0;i<depth;i++)best_moves.emplace_back(n.path[i].first,n.path[i].second);
        return true;
    };
    accept(initial,0);
    initial.hash=deck_hash(initial);
    initial.rank=64*__builtin_popcountll(initial.bad)+difficulty(router,initial,target);
    std::vector<Node> beam(1,initial),pool;
    pool.reserve(width*48);
    struct Candidate {int k,d,gain;bool donor;};
    for(int depth=1;depth<=depth_limit && prefix+depth<int(best_moves.size()) && !expired(true);depth++) {
        pool.clear();
        for(const Node& parent:beam) {
            if(expired())break;
            log.nodes++;
            const auto structural=structure(router,parent,depth-1,border,main,max_supports,pivot_loans);
            log.structural_nodes+=structural.repairs>0;
            std::array<uint64_t,K+1> above{};
            std::array<uint64_t,K> visible{};
            std::array<int,64> last_home{};last_home.fill(-1);
            for(int d=0;d<K;d++) {
                const int k=parent.order[d];
                visible[k]=router.opaque[k]&~above[d];
                above[d+1]=above[d]|router.opaque[k];
                if(d>=border)last_home[router.paper_bucket[k]]=d;
            }
            std::vector<Candidate> candidates,donors;
            candidates.reserve(40);donors.reserve(128);
            auto add=[&](int k,int d,int gain,bool donor) {
                if(d<border || d>=K || parent.position[k]<border || parent.position[k]==d || !visible[k])return;
                for(const auto& c:candidates)if(c.k==k && c.d==d)return;
                candidates.push_back({k,d,gain,donor});
            };
            // A correct display can still contain a late sheet, duplicate
            // home loans, a split bank, or excess supports. Return one of
            // those visible sheets to its actual home, retaining the best
            // few insertion positions. Subsequent beam moves may repair any
            // colors this necessary structural correction uncovers.
            for(int from=border;from<K;from++) {
                const int k=parent.order[from];
                if(!structural.needs_return[k] || !visible[k])continue;
                const int home=router.paper_bucket[k];
                std::vector<std::pair<int,int>> returns;
                auto home_option=[&](int before) {
                    const int d=before-(from<before);
                    if(d<border || d>=K || d==from)return;
                    for(auto [rank,old]:returns)if(old==d)return;
                    uint64_t bad;
                    if(d<from){
                        const uint64_t paint=router.opaque[k]&~above[d];
                        bad=(parent.bad&~paint)|((router.black[k]^target)&paint);
                    }else{
                        Node trial=parent;move_paper(trial,k,d);
                        bad=rendered(router,trial)^target;
                    }
                    returns.emplace_back(64*__builtin_popcountll(bad),d);
                };
                int after_home=K;
                for(int d=border;d<K;d++) {
                    const int other=parent.order[d];
                    if(other==k || structural.support[other] || structural.late[other])continue;
                    const int w=router.paper_bucket[other];
                    if(w==home)home_option(d);
                    if(w>home){after_home=d;break;}
                }
                home_option(after_home);
                std::sort(returns.begin(),returns.end(),[](auto a,auto b){
                    if(a.first!=b.first)return a.first<b.first;
                    return a.second>b.second;
                });
                for(int i=0;i<std::min(return_options,int(returns.size()));i++) {
                    add(k,returns[i].second,0,false);log.home_returns++;
                }
            }
            // Push a currently wrong visible owner below its next correct
            // replacement. Also allow a full return to its home queue's end.
            for(int from=border;from<K;from++) {
                const int k=parent.order[from];
                uint64_t wrong=visible[k]&parent.bad;
                if(!wrong)continue;
                const int home=router.paper_bucket[k];
                add(k,last_home[home],0,false);
                uint64_t unresolved=wrong;
                for(int d=from+1;d<K && unresolved;d++) {
                    const int under=parent.order[d];
                    uint64_t fixed=unresolved&router.opaque[under]&~(router.black[under]^target);
                    if(fixed){add(k,d,0,false);unresolved&=~fixed;}
                }
                // White is also supplied by the empty bottom background.
                if(unresolved&~target)add(k,K-1,0,false);
            }
            // Any visible ordinary sheet may be a donor. Keep useful moves
            // and neutral/slightly worsening ones; the latter can combine
            // with another donor at the next beam depth.
            if(donor_limit || explore_slots)for(int from=border+1;from<K;from++) {
                const int k=parent.order[from];
                if(!visible[k])continue;
                for(int d=border;d<from;d++) {
                    if(d>border && router.paper_bucket[parent.order[d]]==router.paper_bucket[parent.order[d-1]])continue;
                    const uint64_t paint=router.opaque[k]&~above[d];
                    if(!(paint&parent.bad))continue;
                    const uint64_t after=(parent.bad&~paint)|((router.black[k]^target)&paint);
                    const int gain=__builtin_popcountll(parent.bad)-__builtin_popcountll(after);
                    if(gain>=-worsen)donors.push_back({k,d,gain,true});
                }
            }
            auto donor_better=[](const Candidate& a,const Candidate& b){
                if(a.gain!=b.gain)return a.gain>b.gain;
                if(a.d!=b.d)return a.d>b.d;
                return a.k<b.k;
            };
            std::sort(donors.begin(),donors.end(),donor_better);
            int kept=0,neutral_kept=0;
            std::array<uint8_t,K> kept_per_paper{};
            for(const auto& c:donors) {
                if(kept_per_paper[c.k]>=2)continue;
                if(kept<donor_limit){add(c.k,c.d,c.gain,true);kept++;kept_per_paper[c.k]++;}
                else if(c.gain<=0 && neutral_kept<explore_slots){add(c.k,c.d,c.gain,true);neutral_kept++;kept_per_paper[c.k]++;}
                if(kept>=donor_limit && neutral_kept>=explore_slots)break;
            }
            for(const auto& c:candidates) {
                if(expired())break;
                Node child=parent;
                child.path[depth-1]={uint8_t(c.k),uint8_t(c.d)};
                move_paper(child,c.k,c.d);
                if(c.d<int(parent.position[c.k])) {
                    const uint64_t paint=router.opaque[c.k]&~above[c.d];
                    child.bad=(parent.bad&~paint)|((router.black[c.k]^target)&paint);
                }else child.bad=rendered(router,child)^target;
                log.children++;
                child.exploratory=c.donor && __builtin_popcountll(child.bad)>=__builtin_popcountll(parent.bad);
                log.neutral+=child.exploratory;
                if(!child.bad && accept(child,depth))continue;
                child.hash=deck_hash(child);
                child.repairs=structure(router,child,depth,border,main,max_supports,pivot_loans).repairs;
                child.rank=64*__builtin_popcountll(child.bad)+difficulty(router,child,target)+structure_weight*child.repairs;
                pool.push_back(std::move(child));
            }
        }
        if(pool.empty() || prefix+depth+1>=int(best_moves.size()))break;
        std::sort(pool.begin(),pool.end(),[](const Node&a,const Node&b){
            if(a.rank!=b.rank)return a.rank<b.rank;
            return a.hash<b.hash;
        });
        beam.clear();
        auto retain=[&](const Node& n) {
            for(const Node& old:beam)if(old.hash==n.hash && old.order==n.order)return false;
            beam.push_back(n);return true;
        };
        const int normal_width=std::max(1,width-explore_slots);
        for(const Node& n:pool){retain(n);if(int(beam.size())>=normal_width)break;}
        for(const Node& n:pool)if(n.exploratory){if(int(beam.size())>=width)break;retain(n);}
        for(const Node& n:pool){if(int(beam.size())>=width)break;retain(n);}
    }
    router.restore_route_state(best_state);
    if(best_moves.size()<moves.size()){log.accepted++;log.saved+=moves.size()-best_moves.size();}
    if(timed_out)log.timeout++;
    return best_moves;
}
} // namespace query_trace_beam

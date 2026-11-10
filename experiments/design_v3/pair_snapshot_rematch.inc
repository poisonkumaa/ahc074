// Reprice an already constructed weight-4 core map and rerun the original
// matching algorithm. The snapshot and its original four designs are immutable.
namespace pair_snapshot_rematch {
template<class Snapshot>
static bool valid_pair(const Snapshot& source,int a,int b) {
    const int B=source.B,T=N-source.W;
    const uint64_t difference=source.a[a]^source.a[b];
    if(!(~difference&((1ULL<<T)-1)))return false;
    std::array<unsigned,16> before{};
    for(int j=T;j<N;j++)if(difference>>j&1) {
        int top=-1,bottom=-1,count=0;
        for(int k=0;k<B;k++)if(source.domains[k]>>j&1) {
            count++;
            const int color=(source.masks[k]>>j&1)?int(source.bits[k]>>j&1):0;
            if(color==int(source.a[a]>>j&1))top=k;else bottom=k;
        }
        if(count!=2 || top<0 || bottom<0)return false;
        before[bottom]|=1u<<top;
    }
    unsigned alive=(1u<<B)-1;
    for(int step=0;step<B;step++) {
        int take=-1;
        for(int k=0;k<B;k++)if((alive>>k&1) && !(before[k]&alive)){take=k;break;}
        if(take<0)return false;
        alive^=1u<<take;
    }
    return true; // Reversing every edge also remains acyclic for endpoint b.
}
template<class Snapshot>
static std::optional<Snapshot> remake(const Snapshot& source,clock_t deadline,double target_weight) {
    const clock_t started=clock();
    const int inf=1000000,scale=32,B=source.B,T=N-source.W,needed=64+B;
    if(B<2 || B>9 || T<1 || T>=64 || int(source.a.size())!=P ||
       int(source.route.size())!=P*P || int(source.pairs.size())!=needed ||
       int(source.domains.size())!=B || int(source.masks.size())!=B || int(source.bits.size())!=B)
        return std::nullopt;
    if(!std::isfinite(target_weight))target_weight=2;
    target_weight=std::clamp(target_weight,0.0,32.0);
    const double ratio=target_weight/4.0;
    auto expired=[&](){return deadline>0 && clock()>=deadline;};
    if(expired())return std::nullopt;
    std::vector<int> cost(P*P,inf),route(P*P,inf);
    std::vector<std::tuple<int,int,int>> edges;
    Blossom blossom(P);
    for(int u=0;u<P;u++) {
        if(!(u&31) && expired())return std::nullopt;
        for(int v=u+1;v<P;v++)if(source.route[u*P+v]<inf) {
            const int hd=__builtin_popcountll((source.a[u]^source.a[v])&((1ULL<<T)-1));
            const int core=int(std::llround(source.route[u*P+v]*ratio));
            const int value=scale*hd+core;
            route[u*P+v]=route[v*P+u]=core;
            cost[u*P+v]=cost[v*P+u]=value;
            edges.emplace_back(value,u,v);blossom.g[u].push_back(v);blossom.g[v].push_back(u);
        }
    }
    int original_score=0;
    std::array<int,P> original_mate{};original_mate.fill(-1);
    for(const auto& p:source.pairs) {
        if(p.u<0 || p.u>=P || p.v<0 || p.v>=P || p.u==p.v ||
           original_mate[p.u]>=0 || original_mate[p.v]>=0 || cost[p.u*P+p.v]>=inf)return std::nullopt;
        original_mate[p.u]=p.v;original_mate[p.v]=p.u;
        original_score+=cost[p.u*P+p.v];
    }
    // Same cheapest-edge initialization and Blossom completion as try_pack.
    std::sort(edges.begin(),edges.end());int count=0;
    for(auto [price,u,v]:edges)if(blossom.match[u]<0 && blossom.match[v]<0) {
        blossom.match[u]=v;blossom.match[v]=u;if(++count==needed)break;
    }
    if(count<needed)count=blossom.solve();
    if(count<needed)return std::nullopt;
    auto& mate=blossom.match;
    if(count>needed) {
        std::vector<std::pair<int,int>> selected;
        for(int u=0;u<P;u++)if(mate[u]>u)selected.push_back({cost[u*P+mate[u]],u});
        std::sort(selected.rbegin(),selected.rend());
        for(int i=0;i<count-needed;i++){int u=selected[i].second,v=mate[u];mate[u]=mate[v]=-1;}
    }
    int score=0;for(int u=0;u<P;u++)if(mate[u]>u)score+=cost[u*P+mate[u]];
    auto greedy=[&]() {
        while(true) {
            if(expired())return;
            int delta_best=0,bu=-1,bv=-1,bx=-1,by=-1;
            for(int u=0;u<P;u++)if(mate[u]>u) {
                int v=mate[u],old=cost[u*P+v];
                for(int x=0;x<P;x++)if(mate[x]<0) {
                    int delta=cost[u*P+x]-old;
                    if(delta<delta_best)delta_best=delta,bu=u,bv=v,bx=x,by=-1;
                    delta=cost[v*P+x]-old;
                    if(delta<delta_best)delta_best=delta,bu=v,bv=u,bx=x,by=-1;
                }else if(mate[x]>x && x>u) {
                    int y=mate[x],old2=old+cost[x*P+y];
                    int delta=cost[u*P+x]+cost[v*P+y]-old2;
                    if(delta<delta_best)delta_best=delta,bu=u,bv=v,bx=x,by=y;
                    delta=cost[u*P+y]+cost[v*P+x]-old2;
                    if(delta<delta_best)delta_best=delta,bu=u,bv=v,bx=y,by=x;
                }
            }
            if(bu<0)return;
            mate[bu]=bx;mate[bx]=bu;mate[bv]=by;if(by>=0)mate[by]=bv;score+=delta_best;
        }
    };
    greedy();
    uint64_t random_state=123456789ULL^source.a[0];
    auto random32=[&](){random_state^=random_state<<7;random_state^=random_state>>9;return uint32_t(random_state);};
    auto best_mate=mate;int best_score=score,done=0;
    const int iterations=std::max(1,search_budget::integer("PAIR_SNAPSHOT_ITERS",
        search_budget::integer("NOV8_MATCH_ITERS",200000)));
    for(int it=0;it<iterations;it++) {
        if(!(it&1023) && expired())break;
        done=it+1;
        int u=random32()%P;if(mate[u]<0)continue;
        int v=mate[u],x=random32()%P;if(x==u || x==v)continue;
        int y=mate[x];if(cost[u*P+x]>=inf)continue;
        int delta=cost[u*P+x]-cost[u*P+v];
        if(y>=0){if(cost[v*P+y]>=inf)continue;delta+=cost[v*P+y]-cost[x*P+y];}
        const double temperature=scale*(2*(1-double(it)/iterations)+.025);
        if(delta<=0 || (random32()+.5)/4294967296.0<std::exp(-delta/temperature)) {
            mate[u]=x;mate[x]=u;mate[v]=y;if(y>=0)mate[y]=v;score+=delta;
            if(score<best_score){best_score=score;best_mate=mate;}
        }
    }
    mate=std::move(best_mate);score=best_score;greedy();
    Snapshot result=source;result.pairs.clear();result.route=std::move(route);result.price=0;
    int changed=0,checked_score=0;
    for(int u=0;u<P;u++)if(mate[u]>u) {
        const int v=mate[u];
        if(v>=P || mate[v]!=u || cost[u*P+v]>=inf || !valid_pair(source,u,v))return std::nullopt;
        changed+=original_mate[u]!=v;checked_score+=cost[u*P+v];
        result.pairs.push_back({u,v,0,-1,1,-1,-1,-1});
        result.price+=.7*__builtin_popcountll((source.a[u]^source.a[v])&((1ULL<<T)-1))
            +result.route[u*P+v]/32.;
    }
    if(int(result.pairs.size())!=needed || checked_score!=score)return std::nullopt;
    std::cerr<<"PAIR_SNAPSHOT B="<<B<<" W="<<source.W<<" weight="<<target_weight
        <<" pairs="<<result.pairs.size()<<" changed="<<changed<<" edges="<<edges.size()
        <<" before="<<double(original_score)/scale<<" after="<<double(score)/scale
        <<" iterations="<<done<<" timeout="<<expired()<<" cpu="<<double(clock()-started)/CLOCKS_PER_SEC<<"\n";
    return result;
}
} // namespace pair_snapshot_rematch

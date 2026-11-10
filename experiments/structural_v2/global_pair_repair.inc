// Include after pair_repair.inc. Rebuild the matching globally, then assign its
// pair and singleton items to the existing physical slots by Hungarian search.
namespace structural_global_pair_repair {
using Result = structural_pair_repair::Result;
static constexpr double impossible_cost=1.e12;

static bool hungarian(const std::vector<double>& cost,int n,std::vector<int>& column,
                      clock_t deadline) {
    column.assign(n,-1);
    if(n==0)return true;
    std::vector<double> u(n+1),v(n+1),minimum(n+1);
    std::vector<int> owner(n+1),previous(n+1);
    std::vector<uint8_t> used(n+1);
    for(int row=1;row<=n;row++) {
        if((row&7)==0 && deadline>0 && clock()>=deadline)return false;
        owner[0]=row;int at=0;
        std::fill(minimum.begin(),minimum.end(),impossible_cost);
        std::fill(used.begin(),used.end(),0);
        do {
            used[at]=1;const int r=owner[at];int next=-1;double delta=impossible_cost;
            for(int c=1;c<=n;c++)if(!used[c]) {
                const double reduced=cost[size_t(r-1)*n+c-1]-u[r]-v[c];
                if(reduced<minimum[c]){minimum[c]=reduced;previous[c]=at;}
                if(minimum[c]<delta){delta=minimum[c];next=c;}
            }
            if(next<0 || delta>=impossible_cost*.25)return false;
            for(int c=0;c<=n;c++) {
                if(used[c]){u[owner[c]]+=delta;v[c]-=delta;}
                else minimum[c]-=delta;
            }
            at=next;
        }while(owner[at]!=0);
        do {const int next=previous[at];owner[at]=owner[next];at=next;}while(at!=0);
    }
    for(int c=1;c<=n;c++) {
        if(owner[c]<1 || cost[size_t(owner[c]-1)*n+c-1]>=impossible_cost*.5)return false;
        column[owner[c]-1]=c-1;
    }
    return true;
}

static Result optimize(Design& d,const std::vector<uint64_t>& displayed,
                       clock_t deadline,uint64_t seed=1) {
    Result result;
    auto expired=[&](){return deadline>0 && clock()>=deadline;};
    const int B=d.tot_B,products=int(d.internal_catalog.size()),papers=int(d.S_init.size());
    if(B<3 || B>6 || d.C!=0 || d.T<1 || d.T>=64 || products!=P || papers!=K || expired())return result;
    const auto& table=ExactBackingPermutations::get(B);
    if(int(displayed.size())!=table.states)return result;
    const auto& sample=structural_pair_repair::distance_sample(B);
    if(expired()){result.timed_out=true;return result;}
    const auto& A=d.internal_catalog;
    const uint64_t prefix=(uint64_t(1)<<d.T)-1,suffix=~prefix;
    const double prefix_weight=std::max(0.0,search_budget::real("PAIR_REPAIR_PREFIX",.65));
    const double core_weight=std::max(0.0,search_budget::real("PAIR_REPAIR_CORE",1.0));
    const int attempts=std::max(1,search_budget::integer("GLOBAL_PAIR_ATTEMPTS",2));
    const int swaps=std::max(0,search_budget::integer("GLOBAL_PAIR_SWAPS",3000));
    const int slots=papers-B;
    std::vector<std::array<int,2>> original(slots,{-1,-1});
    std::vector<int> size(slots),anchor(slots),color(slots),pair_slots,single_slots;
    for(int a=0;a<products;a++) {
        const int s=d.prod_to_paper[a]-B;
        if(s<0 || s>=slots || size[s]>=2)return result;
        original[s][size[s]++]=a;
    }
    for(int s=0;s<slots;s++) {
        const int paper=B+s,w=d.paper_bucket[paper];
        if(w<0 || w>=d.T || size[s]!=(d.paper_is_paired[paper]?2:1))return result;
        const char c=d.S_init[paper][w];if(c!='0' && c!='1')return result;
        anchor[s]=w;color[s]=c-'0';
        for(int i=0;i<size[s];i++)if(int(A[original[s][i]]>>w&1)!=color[s])return result;
        (size[s]==2?pair_slots:single_slots).push_back(s);
    }
    const int required=int(pair_slots.size()),singles=int(single_slots.size());
    if(2*required+singles!=products || required!=products-slots)return result;
    std::array<double,64> one_probability{};
    for(uint64_t a:A)for(int j=0;j<d.T;j++)one_probability[j]+=double(a>>j&1)/products;
    std::vector<std::array<double,64>> tail(products);
    for(int a=0;a<products;a++) {
        double total=0;
        for(int w=d.T-1;w>=0;w--){tail[a][w]=total;total+=(A[a]>>w&1)?1-one_probability[w]:one_probability[w];}
    }
    std::vector<int> depth(d.T);double retrieval=0;
    for(int s=0;s<slots;s++)retrieval+=size[s]*(depth[anchor[s]]++ + 1.0);
    auto compatible=[&](int a,int s){return int(A[a]>>anchor[s]&1)==color[s];};
    auto single_cost=[&](int a,int s){return prefix_weight*tail[a][anchor[s]];};
    std::vector<double> cheapest_single(products,impossible_cost);
    for(int a=0;a<products;a++)for(int s:single_slots)if(compatible(a,s))
        cheapest_single[a]=std::min(cheapest_single[a],single_cost(a,s));
    // Products that cannot occupy any singleton slot must be covered by the
    // matching. A large marginal credit encourages that without weakening the
    // exact feasibility check in the final singleton assignment.
    for(double& value:cheapest_single)if(value>=impossible_cost*.5)value=1000;
    std::vector<float> routing(size_t(products)*products,-1);
    std::vector<double> weight(size_t(products)*products,impossible_cost);
    auto pair_cost=[&](int u,int v,int s){
        const int w=anchor[s];const uint64_t low=(uint64_t(1)<<(w+1))-1;
        return prefix_weight*(tail[u][w]+tail[v][w]+__builtin_popcountll((A[u]^A[v])&low))
            +core_weight*routing[size_t(u)*products+v];
    };
    struct Edge {int u,v;double weight;};
    std::vector<Edge> edges;
    std::array<std::array<int,720>,2> target{};
    for(int u=0;u<products;u++)for(int v=u+1;v<products;v++) {
        if((result.metric_evaluations&63)==0 && expired()){result.timed_out=true;return result;}
        ++result.metric_evaluations;
        bool slot_possible=false;
        for(int s:pair_slots)if(compatible(u,s)&&compatible(v,s)){slot_possible=true;break;}
        if(!slot_possible)continue;
        const uint64_t needed=(A[u]^A[v])&suffix;
        int count[2]={};
        for(int state=0;state<table.states;state++) {
            const uint64_t bad=(displayed[state]^A[u])&needed;
            if(!bad)target[0][count[0]++]=state;
            if(bad==needed)target[1][count[1]++]=state;
        }
        if(!count[0] || !count[1])continue;
        double total=0;
        if(count[0]!=table.states || count[1]!=table.states) {
            for(int side=0;side<2;side++)for(int h=0;h<sample.samples;h++) {
                const uint8_t* row=sample.distance.data()+size_t(h)*table.states;
                int best=B;
                for(int i=0;i<count[side] && best;i++)best=std::min(best,int(row[target[side][i]]));
                total+=double(best)/sample.samples;
            }
        }
        routing[size_t(u)*products+v]=routing[size_t(v)*products+u]=float(total);
        double best=impossible_cost;
        for(int s:pair_slots)if(compatible(u,s)&&compatible(v,s))best=std::min(best,pair_cost(u,v,s));
        best-=cheapest_single[u]+cheapest_single[v];
        weight[size_t(u)*products+v]=weight[size_t(v)*products+u]=best;
        edges.push_back({u,v,best});
    }
    // The original assignment is a useful incumbent if the changed core still
    // supports it. Its cost uses the same normalized stencils as local repair.
    double best_cost=retrieval,initial_cost=retrieval;
    int initial_invalid=0;
    for(int s=0;s<slots;s++) {
        const int u=original[s][0],v=original[s][1];
        if(size[s]==1){initial_cost+=single_cost(u,s);continue;}
        if(routing[size_t(u)*products+v]<0)initial_invalid++;
        else initial_cost+=pair_cost(u,v,s);
    }
    result.initial_invalid_slots=initial_invalid;
    result.initial_cost=initial_invalid?std::numeric_limits<double>::infinity():initial_cost;
    std::vector<std::array<int,2>> best_owner;
    if(initial_invalid==0){best_cost=initial_cost;best_owner=original;}
    else best_cost=impossible_cost;
    std::stable_sort(edges.begin(),edges.end(),[](const Edge& a,const Edge& b){return a.weight<b.weight;});
    if(!seed)seed=0xd682ab93e648ULL;
    auto random=[&](){seed^=seed<<7;seed^=seed>>9;return uint32_t(seed);};
    auto assign=[&](const std::vector<int>& mate)->bool {
        if(expired())return false;
        std::vector<std::pair<int,int>> pairs;std::vector<int> alone;
        for(int a=0;a<products;a++){
            if(mate[a]<0)alone.push_back(a);
            else if(mate[a]>a)pairs.emplace_back(a,mate[a]);
        }
        if(int(pairs.size())!=required || int(alone.size())!=singles)return false;
        std::vector<double> matrix(size_t(required)*required,impossible_cost);
        for(int i=0;i<required;i++)for(int j=0;j<required;j++){
            auto [u,v]=pairs[i];const int s=pair_slots[j];
            if(compatible(u,s)&&compatible(v,s))matrix[size_t(i)*required+j]=pair_cost(u,v,s);
        }
        std::vector<int> pair_at,single_at;
        if(!hungarian(matrix,required,pair_at,deadline))return false;
        std::vector<std::array<int,2>> owners(slots,{-1,-1});double cost=retrieval;
        for(int i=0;i<required;i++){
            const int j=pair_at[i],s=pair_slots[j];
            owners[s]={pairs[i].first,pairs[i].second};cost+=matrix[size_t(i)*required+j];
        }
        matrix.assign(size_t(singles)*singles,impossible_cost);
        for(int i=0;i<singles;i++)for(int j=0;j<singles;j++){
            const int s=single_slots[j];
            if(compatible(alone[i],s))matrix[size_t(i)*singles+j]=single_cost(alone[i],s);
        }
        if(!hungarian(matrix,singles,single_at,deadline))return false;
        for(int i=0;i<singles;i++){
            const int j=single_at[i],s=single_slots[j];
            owners[s]={alone[i],-1};cost+=matrix[size_t(i)*singles+j];
        }
        if(cost<best_cost-1e-9){best_cost=cost;best_owner=std::move(owners);}
        return true;
    };
    for(int attempt=0;attempt<attempts && !expired();attempt++) {
        std::vector<int> order(edges.size());
        for(int i=0;i<int(edges.size());i++)order[i]=i;
        if(attempt) {
            std::vector<double> noisy(edges.size());
            for(int i=0;i<int(edges.size());i++)noisy[i]=edges[i].weight+.8*((random()+.5)/4294967296.0-.5);
            std::stable_sort(order.begin(),order.end(),[&](int x,int y){return noisy[x]<noisy[y];});
        }
        Blossom blossom(products);
        for(int id:order){const auto& e=edges[id];blossom.g[e.u].push_back(e.v);blossom.g[e.v].push_back(e.u);}
        int matched=0;
        for(int id:order){const auto& e=edges[id];if(blossom.match[e.u]<0&&blossom.match[e.v]<0){
            blossom.match[e.u]=e.v;blossom.match[e.v]=e.u;if(++matched==required)break;
        }}
        for(int a=0;a<products && matched<required && !expired();a++)
            if(blossom.match[a]<0 && blossom.augment(a))matched++;
        if(matched<required)continue;
        auto mate=blossom.match;
        assign(mate);
        // Endpoint exchanges optimize the marginal edge estimate while keeping
        // the number of pairs fixed. Hungarian admission remains authoritative.
        double current=0;for(int u=0;u<products;u++)if(mate[u]>u)current+=weight[size_t(u)*products+mate[u]];
        double saved_cost=current;auto saved_mate=mate;
        for(int it=0;it<swaps && !edges.empty();it++) {
            result.iterations++;
            if((it&63)==0 && expired())break;
            const auto& e=edges[random()%edges.size()];const int u=e.u,v=e.v,x=mate[u],y=mate[v];
            if(x==v || (x<0 && y<0))continue;
            double delta=e.weight;
            if(x>=0)delta-=weight[size_t(u)*products+x];
            if(y>=0)delta-=weight[size_t(v)*products+y];
            if(x>=0 && y>=0){if(routing[size_t(x)*products+y]<0)continue;delta+=weight[size_t(x)*products+y];}
            const double temperature=.25*(1-double(it)/std::max(1,swaps))+.01;
            if(delta>0 && (random()+.5)/4294967296.0>=std::exp(-delta/temperature))continue;
            mate[u]=v;mate[v]=u;if(x>=0)mate[x]=y;if(y>=0)mate[y]=x;
            current+=delta;result.accepted++;
            if(current<saved_cost-1e-9){saved_cost=current;saved_mate=mate;}
        }
        if(saved_mate!=blossom.match && !expired())assign(saved_mate);
    }
    result.timed_out=expired();
    if(best_owner.empty()){result.invalid_slots=initial_invalid;return result;}
    // Validate the complete product permutation and every slot before committing.
    std::vector<int> seen(products);
    for(int s=0;s<slots;s++) {
        for(int i=0;i<size[s];i++){
            const int a=best_owner[s][i];
            if(a<0 || a>=products || !compatible(a,s) || seen[a]++)return result;
        }
        if(size[s]==2 && routing[size_t(best_owner[s][0])*products+best_owner[s][1]]<0)return result;
    }
    if(std::any_of(seen.begin(),seen.end(),[](int n){return n!=1;}))return result;
    std::vector<std::string> rows=d.S_init;
    std::vector<PairInfo> pairs;
    std::vector<int> paper_of(products,-1),pair_of(products,-1);
    for(int s=0;s<slots;s++) {
        const int k=B+s,w=anchor[s],u=best_owner[s][0],v=size[s]==2?best_owner[s][1]:u;
        const uint64_t common=~(A[u]^A[v]),domain=((uint64_t(1)<<(w+1))-1)|suffix;
        rows[k].assign(64,'.');
        for(uint64_t bits=common&domain;bits;bits&=bits-1){
            const int j=__builtin_ctzll(bits);rows[k][j]=char('0'+int(A[u]>>j&1));
        }
        paper_of[u]=k;
        if(size[s]==2){
            paper_of[v]=k;const int id=int(pairs.size());pair_of[u]=pair_of[v]=id;
            pairs.push_back({u,v,0,-1,1,-1,w,k});
            auto was=original[s],now=best_owner[s];
            if(was[0]>was[1])std::swap(was[0],was[1]);if(now[0]>now[1])std::swap(now[0],now[1]);
            result.changed_pairs+=was!=now;
        }
        for(int i=0;i<size[s];i++)result.changed_products+=d.prod_to_paper[best_owner[s][i]]!=k;
    }
    d.S_init=std::move(rows);d.pairs=std::move(pairs);
    d.prod_to_paper=std::move(paper_of);d.prod_to_pair_idx=std::move(pair_of);
    result.feasible=true;result.invalid_slots=0;result.cost=best_cost;
    return result;
}
} // namespace structural_global_pair_repair

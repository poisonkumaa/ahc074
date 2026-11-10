// Reprice all feasible pairs using both complete parent layouts, then optimize
// a fixed-cardinality matching. This changes only the optional eighth design.
namespace design_matching {
static constexpr double infinity=1e30;

struct LayoutPrice {
    const Design& d;
    std::array<int,64> load{},paired{};
    std::array<std::array<double,64>,P> mismatch{};
    explicit LayoutPrice(const Design& d):d(d) {
        std::array<int,64> zeros{},ones{};
        for(int k=d.tot_B;k<K;k++) {
            const int w=d.paper_bucket[k];load[w]++;paired[w]+=d.paper_is_paired[k];
        }
        for(int a=0;a<P;a++)for(int j=0;j<d.T;j++)
            if(d.S_init[d.prod_to_paper[a]][j]=='.')
                ((d.internal_catalog[a]>>j&1)?ones[j]:zeros[j])++;
        for(int a=0;a<P;a++)for(int j=0;j<d.T;j++) {
            const double black=(3.+ones[j])/(6+ones[j]+zeros[j]);
            const double wrong=(d.internal_catalog[a]>>j&1)?1-black:black;
            mismatch[a][j]=2*wrong*wrong;
        }
    }
    double item(int a,int b)const {
        const bool pair=b>=0;
        const uint64_t diff=pair?d.internal_catalog[a]^d.internal_catalog[b]:0;
        double uncovered=0,best=infinity;
        for(int j=0;j<d.T;j++)uncovered+=mismatch[a][j]+(pair?mismatch[b][j]:0);
        for(int w=0;w<d.T;w++)if(!(diff>>w&1)) {
            uncovered-=mismatch[a][w]+(pair?mismatch[b][w]:0);
            best=std::min(best,uncovered+load[w]+1+(pair?paired[w]+1:0));
        }
        return best;
    }
};

static bool literal_pair(const Design& d,int a,int b) {
    if(a<0 || b<0 || a>=P || b>=P || a==b)return false;
    const uint64_t diff=d.internal_catalog[a]^d.internal_catalog[b];
    if(!(~diff&((1ULL<<d.T)-1)))return false;
    std::array<unsigned,9> before{};
    for(int j=d.T;j<N;j++)if(diff>>j&1) {
        int winner=-1,loser=-1,opaque=0;
        for(int k=0;k<d.tot_B;k++)if(d.S_init[k][j]!='.') {
            opaque++;
            if(int(d.S_init[k][j]-'0')==int(d.internal_catalog[a]>>j&1))winner=k;
            else loser=k;
        }
        if(opaque!=2 || winner<0 || loser<0)return false;
        before[loser]|=1u<<winner;
    }
    unsigned alive=(1u<<d.tot_B)-1;
    while(alive) {
        int k=0;while(k<d.tot_B && (!(alive>>k&1) || (before[k]&alive)))k++;
        if(k==d.tot_B)return false;
        alive^=1u<<k;
    }
    return true;
}

static std::vector<PairInfo> offspring(const Design& first,const Design& second,
        const std::vector<int>& route,clock_t absolute_end) {
    auto original=merge_matching(first,second,route);
    if(!search_budget::integer("DESIGN_MATCHING",0))return original;
    if(first.C!=0 || second.C!=0 || first.T!=second.T || first.tot_B!=second.tot_B ||
       first.pairs.size()!=second.pairs.size() || route.size()!=size_t(P*P))return original;
    const clock_t begun=clock();
    const double cap=search_budget::seconds(std::max(0.,search_budget::real("DESIGN_MATCHING_CPU",.04)));
    const clock_t deadline=std::min(absolute_end,begun+clock_t(cap*CLOCKS_PER_SEC));
    auto expired=[&](){return clock()>=deadline;};
    if(expired())return original;
    const int required=P-(K-first.tot_B);
    if(int(original.size())!=required)return original;
    const LayoutPrice p0(first),p1(second);
    std::array<double,P> single{};
    double singleton_total=0;
    for(int a=0;a<P;a++)singleton_total+=(single[a]=(p0.item(a,-1)+p1.item(a,-1))*.5);
    std::vector<double> edge(P*P,infinity);
    std::array<std::vector<int>,P> adj;
    int edge_count=0;
    for(int a=0;a<P;a++) {
        if(expired()) {
            std::cerr<<"DESIGN_MATCHING aborted=pricing cpu="<<double(clock()-begun)/CLOCKS_PER_SEC<<"\n";
            return original;
        }
        for(int b=a+1;b<P;b++)if(route[a*P+b]<1000000) {
            if(!literal_pair(first,a,b) || !literal_pair(second,a,b))continue;
            const double price=(p0.item(a,b)+p1.item(a,b))*.5+route[a*P+b]/32.-single[a]-single[b];
            if(price>=infinity/2)continue;
            edge[a*P+b]=edge[b*P+a]=price;adj[a].push_back(b);adj[b].push_back(a);edge_count++;
        }
    }
    using Matching=std::array<int,P>;
    auto decode=[&](const std::vector<PairInfo>& pairs,Matching& mate) {
        mate.fill(-1);int count=0;
        for(const auto& p:pairs) {
            const int a=p.u,b=p.v;
            if(a<0 || b<0 || a>=P || b>=P || a==b || mate[a]>=0 || mate[b]>=0 ||
               edge[a*P+b]>=infinity/2)return false;
            mate[a]=b;mate[b]=a;count++;
        }
        return count==required;
    };
    auto energy=[&](const Matching& mate) {
        double value=singleton_total;
        for(int a=0;a<P;a++)if(mate[a]>a)value+=edge[a*P+mate[a]];
        return value;
    };
    Matching base,mate,best_mate;
    if(!decode(original,base))return original;
    mate=best_mate=base;
    const double before=energy(base);double best=before;
    for(const auto* parent:{&first.pairs,&second.pairs}) {
        Matching start;if(!decode(*parent,start))continue;
        const double value=energy(start);
        if(value<best){best=value;mate=best_mate=start;}
    }
    double current=best;
    uint64_t rng=first.internal_catalog[0]^0x3ad8342d9b4c7195ULL;
    if(!rng)rng=1;
    auto random32=[&](){rng^=rng<<7;rng^=rng>>9;return uint32_t(rng);};
    const int iterations=std::max(0,search_budget::integer("DESIGN_MATCHING_ITERS",60000));
    std::array<double,256> cooling{};
    for(int i=0;i<256;i++)cooling[i]=3*std::pow(.002,double(i)/255);
    int done=0,accepted=0,three_exchanges=0;
    for(int it=0;it<iterations;it++) {
        if(!(it&255) && expired())break;
        done++;
        const int a=random32()%P,b=mate[a];
        if(b<0 || adj[a].empty())continue;
        const int c=adj[a][random32()%adj[a].size()],d=mate[c];
        if(c==b)continue;
        bool three=(random32()&3)==0 && d>=0;
        int e=-1,f=-1;
        double delta=edge[a*P+c]-edge[a*P+b];
        if(three) {
            if(adj[b].empty())continue;
            e=adj[b][random32()%adj[b].size()];
            if(e==a || e==b || e==c || e==d)continue;
            f=mate[e];
            if(f>=0 && edge[d*P+f]>=infinity/2)continue;
            delta+=edge[b*P+e]-edge[c*P+d];
            if(f>=0)delta+=edge[d*P+f]-edge[e*P+f];
        }else if(d>=0) {
            if(edge[b*P+d]>=infinity/2)continue;
            delta+=edge[b*P+d]-edge[c*P+d];
        }
        const double temperature=cooling[std::min(255,int(256LL*it/std::max(1,iterations)))];
        if(delta>0 && (random32()+.5)/4294967296.>=std::exp(-delta/temperature))continue;
        mate[a]=c;mate[c]=a;
        if(three) {
            mate[b]=e;mate[e]=b;mate[d]=f;if(f>=0)mate[f]=d;three_exchanges++;
        }else {mate[b]=d;if(d>=0)mate[d]=b;}
        current+=delta;accepted++;
        if(current<best-1e-9){best=current;best_mate=mate;}
    }
    // Cheap descent over every feasible two-edge/endpoint exchange.
    mate=best_mate;current=energy(mate);
    int descent=0;
    for(int pass=0;pass<64 && !expired();pass++) {
        double gain=-1e-9;int aa=-1,cc=-1;
        for(int a=0;a<P;a++)if(mate[a]>=0)for(int c:adj[a]) {
            const int b=mate[a],d=mate[c];if(c==b)continue;
            if(d>=0 && edge[b*P+d]>=infinity/2)continue;
            double delta=edge[a*P+c]-edge[a*P+b];
            if(d>=0)delta+=edge[b*P+d]-edge[c*P+d];
            if(delta<gain){gain=delta;aa=a;cc=c;}
        }
        if(aa<0)break;
        const int b=mate[aa],d=mate[cc];mate[aa]=cc;mate[cc]=aa;mate[b]=d;if(d>=0)mate[d]=b;
        current+=gain;descent++;
        if(current<best-1e-9){best=current;best_mate=mate;}
    }
    int changed=0,new_edges=0;
    std::vector<PairInfo> result;
    for(int a=0;a<P;a++)if(best_mate[a]>a) {
        const int b=best_mate[a];
        if(best_mate[b]!=a || !literal_pair(first,a,b) || !literal_pair(second,a,b))return original;
        changed+=base[a]!=b;
        new_edges+=first.prod_to_paper[a]!=first.prod_to_paper[b] &&
                   second.prod_to_paper[a]!=second.prod_to_paper[b];
        result.push_back({a,b,0,-1,1,-1,-1,-1});
    }
    Matching verified;
    if(int(result.size())!=required || !decode(result,verified) || verified!=best_mate ||
       energy(best_mate)>before+1e-7)return original;
    std::cerr<<"DESIGN_MATCHING edges="<<edge_count<<" pairs="<<result.size()<<" changed="<<changed
        <<" new_edges="<<new_edges<<" proxy="<<before<<"->"<<energy(best_mate)
        <<" iterations="<<done<<" accepted="<<accepted<<" three="<<three_exchanges<<" descent="<<descent
        <<" timeout="<<expired()<<" cpu="<<double(clock()-begun)/CLOCKS_PER_SEC<<"\n";
    return changed?result:original;
}
} // namespace design_matching

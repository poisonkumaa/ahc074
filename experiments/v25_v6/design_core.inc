// Refit only the chosen design's backing comparison columns to its final pairs.
// Main patterns, pairing, anchors, prefix columns, and metadata stay fixed.
namespace design_core {
static int mean_cost(int B,uint64_t graph) {
    for(int k=0;k<B;k++)for(int i=0;i<B;i++)if(graph>>(8*i+k)&1)
        graph|=((graph>>(8*k))&255ULL)<<(8*i);
    for(int k=0;k<B;k++)if(graph>>(9*k)&1)return -1;
    return B==5?OrderMetric<5>::get()(graph):OrderMetric<6>::get()(graph);
}

// Independently construct a legal core order for every product and verify the
// literal suffix display with its main on top. Prefix main colors must also fit.
static bool validate(const Design& d) {
    if(!has_complete_backing_graph(d))return false;
    for(int a=0;a<P;a++) {
        const int main=d.prod_to_paper[a],B=d.tot_B;
        std::array<unsigned,9> before{};
        for(int j=0;j<N;j++) {
            const char c=d.S_init[main][j];
            if(c!='.' && int(c-'0')!=int(d.internal_catalog[a]>>j&1))return false;
            if(j<d.T || c!='.')continue;
            int top=-1,bottom=-1,count=0;
            for(int k=0;k<B;k++)if(d.S_init[k][j]!='.') {
                count++;
                if(int(d.S_init[k][j]-'0')==int(d.internal_catalog[a]>>j&1))top=k;
                else bottom=k;
            }
            if(count!=2 || top<0 || bottom<0)return false;
            before[bottom]|=1u<<top;
        }
        std::array<int,9> order{};unsigned alive=(1u<<B)-1;
        for(int rank=0;rank<B;rank++) {
            int k=0;while(k<B && (!(alive>>k&1) || (before[k]&alive)))k++;
            if(k==B)return false;
            order[rank]=k;alive^=1u<<k;
        }
        for(int j=d.T;j<N;j++) {
            char color=d.S_init[main][j];
            if(color=='.')for(int rank=0;rank<B;rank++)if(d.S_init[order[rank]][j]!='.') {
                color=d.S_init[order[rank]][j];break;
            }
            if(color=='.' || int(color-'0')!=int(d.internal_catalog[a]>>j&1))return false;
        }
    }
    return true;
}

static bool optimize(Design& design,clock_t absolute_end) {
    if(!search_budget::integer("DESIGN_CORE",0))return false;
    const int B=design.tot_B,T=design.T,W=N-T;
    if(design.C!=0 || (B!=5 && B!=6) || W<=0 || design.pairs.empty())return false;
    const clock_t begun=clock();
    const double cap=search_budget::seconds(std::max(0.,search_budget::real("DESIGN_CORE_CPU",.06)));
    const clock_t deadline=std::min(absolute_end,begun+clock_t(cap*CLOCKS_PER_SEC));
    if(clock()>=deadline)return false;
    std::vector<std::pair<int,int>> arcs;
    std::array<int,64> arc_id;arc_id.fill(-1);
    for(int z=0;z<B;z++)for(int o=z+1;o<B;o++) {
        arc_id[8*z+o]=arcs.size();arcs.push_back({z,o});
        arc_id[8*o+z]=arcs.size();arcs.push_back({o,z});
    }
    auto unordered_edge=[&](int code){auto[z,o]=arcs[code];return 8*std::min(z,o)+std::max(z,o);};
    std::vector<int> codes(W);
    std::array<int,64> witnesses{};
    for(int j=0;j<T;j++)for(int k=0;k<B;k++)if(design.S_init[k][j]!='.')return false;
    for(int j=0;j<W;j++) {
        int z=-1,o=-1,count=0;
        for(int k=0;k<B;k++)if(design.S_init[k][T+j]!='.') {
            count++;if(design.S_init[k][T+j]=='0')z=k;else if(design.S_init[k][T+j]=='1')o=k;
        }
        if(count!=2 || z<0 || o<0)return false;
        codes[j]=arc_id[8*z+o];witnesses[unordered_edge(codes[j])]++;
    }
    auto complete=[&](){for(int z=0;z<B;z++)for(int o=z+1;o<B;o++)if(!witnesses[8*z+o])return false;return true;};
    if(!complete() || !validate(design))return false;
    const int n=design.pairs.size();
    std::vector<uint64_t> difference(n),first(n),graph(n);
    std::vector<std::array<uint8_t,64>> multiplicity(n);
    std::vector<int> costs(n),proposal_cost(n);
    auto relation=[&](int pair,int column,int code) {
        auto[z,o]=arcs[code];return (first[pair]>>column&1)?8*z+o:8*o+z;
    };
    int sum=0;
    for(int p=0;p<n;p++) {
        const auto& pair=design.pairs[p];
        first[p]=design.internal_catalog[pair.u]>>T;
        difference[p]=(design.internal_catalog[pair.u]^design.internal_catalog[pair.v])>>T;
        for(uint64_t todo=difference[p];todo;todo&=todo-1) {
            int j=__builtin_ctzll(todo),edge=relation(p,j,codes[j]);
            multiplicity[p][edge]++;graph[p]|=1ULL<<edge;
        }
        costs[p]=mean_cost(B,graph[p]);if(costs[p]<0)return false;sum+=costs[p];
    }
    const int initial=sum;
    int best=sum;
    auto best_codes=codes,initial_codes=codes;
    auto replace=[&](int column,int code) {
        const int old=codes[column];
        witnesses[unordered_edge(old)]--;witnesses[unordered_edge(code)]++;
        for(int p=0;p<n;p++)if(difference[p]>>column&1) {
            const int erased=relation(p,column,old),added=relation(p,column,code);
            if(--multiplicity[p][erased]==0)graph[p]&=~(1ULL<<erased);
            if(multiplicity[p][added]++==0)graph[p]|=1ULL<<added;
        }
        codes[column]=code;
    };
    const int iterations=std::max(0,search_budget::integer("DESIGN_CORE_ITERS",12000));
    const double start_temperature=std::max(0.,search_budget::real("DESIGN_CORE_TEMP",8.));
    std::array<double,256> cooling{};
    for(int i=0;i<256;i++)cooling[i]=start_temperature*std::pow(.005,double(i)/255);
    uint64_t state=design.internal_catalog[0]^0xf431896b0dd87147ULL;if(!state)state=1;
    auto random32=[&](){state^=state<<7;state^=state>>9;return uint32_t(state);};
    int tries=0,feasible=0,accepted=0,improvements=0,witness_rejects=0;
    const bool verify=search_budget::integer("DESIGN_CORE_VERIFY",0)!=0;
    for(int it=0;it<iterations;it++) {
        if(!(it&63) && clock()>=deadline)break;
        const int kind=random32()%3,x=random32()%W,oldx=codes[x];
        int y=-1,oldy=-1,newx,newy=-1;
        if(kind==0)newx=oldx^1;
        else if(kind==1)newx=random32()%arcs.size();
        else {
            y=random32()%W;if(y==x)continue;oldy=codes[y];
            newx=oldy^(random32()&1);newy=oldx^(random32()&1);
        }
        if(newx==oldx && (y<0 || newy==oldy))continue;
        tries++;
        replace(x,newx);if(y>=0)replace(y,newy);
        bool valid=complete();int delta=0;
        if(!valid)witness_rejects++;
        const uint64_t changed=(1ULL<<x)|(y>=0?1ULL<<y:0);
        if(valid)for(int p=0;p<n;p++)if(difference[p]&changed) {
            proposal_cost[p]=mean_cost(B,graph[p]);
            if(proposal_cost[p]<0){valid=false;break;}
            delta+=proposal_cost[p]-costs[p];
        }
        if(valid)feasible++;
        const double temperature=cooling[std::min(255,int(256LL*it/std::max(1,iterations)))];
        bool take=valid && (delta<=0 || (temperature>0 && (random32()+.5)/4294967296.<std::exp(-delta/temperature)));
        if(take) {
            accepted++;sum+=delta;
            for(int p=0;p<n;p++)if(difference[p]&changed)costs[p]=proposal_cost[p];
            if(sum<best){best=sum;best_codes=codes;improvements++;}
        }else {if(y>=0)replace(y,oldy);replace(x,oldx);}
        if(verify) {
            int reference=0;
            for(int p=0;p<n;p++) {
                uint64_t fresh=0;
                std::array<uint8_t,64> counts{};
                for(uint64_t todo=difference[p];todo;todo&=todo-1) {
                    int j=__builtin_ctzll(todo),e=relation(p,j,codes[j]);fresh|=1ULL<<e;counts[e]++;
                }
                if(fresh!=graph[p] || counts!=multiplicity[p] || mean_cost(B,fresh)!=costs[p])std::abort();
                reference+=mean_cost(B,fresh);
            }
            if(reference!=sum || !complete())std::abort();
        }
    }
    int changed=0;for(int j=0;j<W;j++)changed+=best_codes[j]!=initial_codes[j];
    bool committed=false;
    if(best<initial && changed) {
        Design candidate=design;
        for(int j=0;j<W;j++) {
            for(int k=0;k<B;k++)candidate.S_init[k][T+j]='.';
            auto[z,o]=arcs[best_codes[j]];
            candidate.S_init[z][T+j]='0';candidate.S_init[o][T+j]='1';
        }
        if(validate(candidate)) {design=std::move(candidate);committed=true;}
    }
    std::cerr<<"DESIGN_CORE B="<<B<<" W="<<W<<" pairs="<<n<<" tries="<<tries
        <<" feasible="<<feasible<<" witness_rejects="<<witness_rejects<<" accepted="<<accepted
        <<" improvements="<<improvements<<" changed="<<(committed?changed:0)
        <<" metric="<<initial<<"->"<<best<<" predicted_moves="<<(initial-best)/8.
        <<" committed="<<committed<<" verified="<<verify<<" cpu="<<double(clock()-begun)/CLOCKS_PER_SEC<<"\n";
    return committed;
}
} // namespace design_core

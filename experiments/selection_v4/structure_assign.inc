// Experimental global anchor reassignment. No future real orders are read.
// The objective freezes observed pairwise queue-order probabilities and donor
// competition. Its cost is exact for that frozen model, not for the full router.
namespace structure_assign {
static bool optimize(Design& design,vector<HeadSample>& history,clock_t absolute_end) {
    if(!search_budget::integer("STRUCTURE_ASSIGN",0) || design.C!=0 || history.empty())return false;
    const clock_t started=clock();
    const double allowance=search_budget::seconds(max(0.,search_budget::real("STRUCTURE_CPU",.25)));
    const clock_t deadline=min(absolute_end,started+clock_t(allowance*CLOCKS_PER_SEC));
    const int B=design.tot_B,T=design.T;
    if(T<=0 || T>=63 || deadline<=started)return false;
    const int H=min<int>(history.size(),max(1,search_budget::integer("STRUCTURE_SAMPLES",12)));
    vector<HeadSample> hist;for(int h=0;h<H;h++)hist.push_back(history[h*history.size()/H]);
    BankTemplates templ(design);const uint64_t prefix=(1ULL<<T)-1;
    const auto& A=design.internal_catalog;
    array<int,K> home{},weight{},at{},kind{};
    array<uint64_t,K> ink{};array<uint64_t,P> holes{};
    vector<array<int,4>> count(T),initial_count(T);
    vector<vector<int>> members(T);
    for(int k=B;k<K;k++) {
        home[k]=at[k]=design.paper_bucket[k];weight[k]=1+int(design.paper_is_paired[k]);
        if(home[k]<0 || home[k]>=T || templ.owner[k]<0)return false;
        kind[k]=2*(weight[k]-1)+int(templ.bits[k]>>home[k]&1);
        count[home[k]][kind[k]]++;members[home[k]].push_back(k);
        for(int j=0;j<T;j++)if(design.S_init[k][j]!='.')ink[k]|=1ULL<<j;
    }
    initial_count=count;
    for(int a=0;a<P;a++)holes[a]=prefix&~ink[design.prod_to_paper[a]];
    // Probability that a type-u card precedes a type-t card in this bank.
    using Matrix=array<array<double,4>,4>;
    vector<Matrix> ahead(T),seen(T);Matrix global{},global_seen{};
    vector<array<double,4>> mean_depth(T);
    for(int w=0;w<T;w++)for(const auto& h:hist) {
        for(int x:members[w])mean_depth[w][kind[x]]+=h.depth[x];
        for(int x:members[w])for(int y:members[w])if(x!=y) {
            const int t=kind[x],u=kind[y];
            double p=h.depth[y]<h.depth[x]?1.:h.depth[y]>h.depth[x]?0.:.5;
            ahead[w][t][u]+=p;seen[w][t][u]++;
            global[t][u]+=p;global_seen[t][u]++;
        }
    }
    for(int w=0;w<T;w++)for(int t=0;t<4;t++) {
        mean_depth[w][t]=count[w][t]?mean_depth[w][t]/(H*count[w][t]):max(0.,(double(members[w].size())-1)/2);
        for(int u=0;u<4;u++)ahead[w][t][u]=seen[w][t][u]?ahead[w][t][u]/seen[w][t][u]:
            global_seen[t][u]?global[t][u]/global_seen[t][u]:.5;
    }
    const bool empirical=search_budget::integer("STRUCTURE_EMPIRICAL",1)!=0;
    auto retrieval=[&](int w,const array<int,4>& q) {
        if(!empirical){int p=q[2]+q[3],s=q[0]+q[1];return p*(p+1.)+s*p+s*(s+1.)/2;}
        double value=0;
        for(int t=0;t<4;t++) {
            const int f=1+(t>>1);
            value+=f*q[t]*(q[t]+1.)/2;
            for(int u=t+1;u<4;u++)value+=q[t]*q[u]*
                (f*ahead[w][t][u]+(1+(u>>1))*ahead[w][u][t]);
        }
        return value;
    };
    auto coverage=[&](int a,int k,int w,uint64_t opaque,uint64_t missing) {
        if(!(missing>>w&1))return uint64_t(0);
        uint64_t covered=opaque&missing,bad=(templ.bits[k]^A[a])&covered;
        if(bad){int j=63-__builtin_clzll(bad);if(j>=w)return uint64_t(0);covered&=~0ULL<<(j+1);}
        return covered;
    };
    struct Best{float value=0,second=0;int id=-1;};
    vector<Best> competition(P*H);
    for(int a=0;a<P;a++)for(int k=B;k<K;k++)if(k!=design.prod_to_paper[a]) {
        uint64_t covered=coverage(a,k,home[k],ink[k],holes[a]);if(!covered)continue;
        for(int h=0;h<H;h++) {
            float value=__builtin_popcountll((hist[h].black^A[a])&covered)-hist[h].depth[k]-2;
            auto& best=competition[a*H+h];
            if(value>best.value){best.second=best.value;best.value=value;best.id=k;}
            else best.second=max(best.second,value);
        }
    }
    const double infinity=1e30;
    vector<vector<double>> price(K,vector<double>(T,infinity));
    bool aborted=false;
    for(int k=B;k<K;k++) {
        if(clock()>=deadline){aborted=true;break;}
        for(int w=0;w<T;w++)if(templ.printable[k]>>w&1) {
            uint64_t new_ink=templ.printable[k]&((1ULL<<(w+1))-1);
            // Preserve the original row exactly when it remains at its anchor.
            if(w==home[k])new_ink=ink[k];
            uint64_t missing=prefix&~new_ink;double own=0,utility=0;
            for(int a:{templ.owner[k],templ.other[k]})if(a>=0) {
                vector<int> best(H);
                for(int donor=B;donor<K;donor++)if(donor!=k) {
                    uint64_t covered=coverage(a,donor,home[donor],ink[donor],missing);
                    if(__builtin_popcountll(covered)<3)continue;
                    for(int h=0;h<H;h++)best[h]=max(best[h],
                        __builtin_popcountll((hist[h].black^A[a])&covered)-hist[h].depth[donor]-2);
                }
                for(int h=0;h<H;h++)own+=double(__builtin_popcountll((hist[h].black^A[a])&missing)-best[h])/H;
            }
            const int new_type=2*(weight[k]-1)+int(templ.bits[k]>>w&1);
            for(int a=0;a<P;a++)if(design.prod_to_paper[a]!=k) {
                uint64_t covered=coverage(a,k,w,new_ink,holes[a]);if(!covered)continue;
                for(int h=0;h<H;h++) {
                    const auto& best=competition[a*H+h];
                    double alternative=best.id==k?best.second:best.value;
                    double depth=w==home[k]?hist[h].depth[k]:mean_depth[w][new_type];
                    utility+=max(0.,__builtin_popcountll((hist[h].black^A[a])&covered)-depth-2-alternative)/H;
                }
            }
            price[k][w]=own-utility;
        }
    }
    if(aborted){cerr<<"STRUCTURE_ASSIGN aborted=pricing cpu="<<double(clock()-started)/CLOCKS_PER_SEC<<'\n';return false;}
    auto valid=[&](int w,const array<int,4>& q) {
        for(int t=0;t<4;t++)if(q[t]<0)return false;
        for(int c=0;c<2;c++)if(q[c]+q[c+2]<min(2,initial_count[w][c]+initial_count[w][c+2]))return false;
        return true;
    };
    auto type_at=[&](int k,int w){return 2*(weight[k]-1)+int(templ.bits[k]>>w&1);};
    auto objective=[&](){double v=0;for(int w=0;w<T;w++)v+=retrieval(w,count[w]);for(int k=B;k<K;k++)v+=price[k][at[k]];return v;};
    const double before=objective();int steps=0,relocations=0,swaps=0;
    const int limit=max(0,search_budget::integer("STRUCTURE_STEPS",24));
    for(;steps<limit && clock()<deadline;steps++) {
        int bx=-1,by=-1,bw=-1;double gain=-1e-8;
        auto consider=[&](int x,int y,int v) {
            int u=at[x];if(u==v || price[x][v]>=infinity/2)return;
            if(y>=0 && price[y][u]>=infinity/2)return;
            auto qu=count[u],qv=count[v];qu[type_at(x,u)]--;qv[type_at(x,v)]++;
            double delta=price[x][v]-price[x][u];
            if(y>=0){qv[type_at(y,v)]--;qu[type_at(y,u)]++;delta+=price[y][u]-price[y][v];}
            if(!valid(u,qu)||!valid(v,qv))return;
            delta+=retrieval(u,qu)+retrieval(v,qv)-retrieval(u,count[u])-retrieval(v,count[v]);
            if(delta<gain){gain=delta;bx=x;by=y;bw=v;}
        };
        for(int x=B;x<K;x++) {
            if((x&7)==0 && clock()>=deadline)break;
            for(int w=0;w<T;w++)consider(x,-1,w);
            for(int y=x+1;y<K;y++)consider(x,y,at[y]);
        }
        if(bx<0)break;
        int u=at[bx],v=bw;count[u][type_at(bx,u)]--;count[v][type_at(bx,v)]++;at[bx]=v;
        if(by>=0){count[v][type_at(by,v)]--;count[u][type_at(by,u)]++;at[by]=u;swaps++;}else relocations++;
    }
    int changed=0;for(int k=B;k<K;k++)changed+=at[k]!=home[k];
    double after=objective();
    if(changed==0 || clock()>=deadline){cerr<<"STRUCTURE_ASSIGN changed=0 proposed="<<changed<<" steps="<<steps<<" proxy="<<before<<"->"<<after<<" aborted="<<int(clock()>=deadline)<<"\n";return false;}
    Design candidate=design;array<int,K> destination{};int next=B;
    for(int k=0;k<B;k++)destination[k]=k;
    for(int w=0;w<T;w++)for(int k=B;k<K;k++)if(at[k]==w)destination[k]=next++;
    if(next!=K)return false;
    for(int k=B;k<K;k++) {
        int nk=destination[k],w=at[k];candidate.S_init[nk]=design.S_init[k];
        if(w!=home[k])for(int j=0;j<T;j++)candidate.S_init[nk][j]=
            j<=w&&(templ.printable[k]>>j&1)?char('0'+int(templ.bits[k]>>j&1)):'.';
        candidate.paper_bucket[nk]=w;candidate.paper_is_paired[nk]=design.paper_is_paired[k];
    }
    for(int a=0;a<P;a++)candidate.prod_to_paper[a]=destination[design.prod_to_paper[a]];
    for(auto& pair:candidate.pairs){pair.paper_id=candidate.prod_to_paper[pair.u];pair.bucket_w=candidate.paper_bucket[pair.paper_id];}
    // Refresh physical head observations after changing paper IDs and bank sizes.
    // An incomplete refresh rejects the entire design change.
    OnlineRouter sample(candidate,A,deadline);sample.route_deadline=deadline;sample.route_branching=false;
    vector<HeadSample> fresh;const int samples=history.size();uint64_t rng=A[0]|1;int previous=-1;
    for(int z=0;z<32+7*samples;z++) {
        if(clock()>=deadline){aborted=true;break;}
        int a;do{rng^=rng<<7;rng^=rng>>9;a=int(rng%P);}while(a==previous);
        auto moves=sample.query(a);previous=a;
        if(moves.size()>2*K || sample.sim.current_display()!=A[a]){aborted=true;break;}
        if(z>=32 && (z-32)%7==0){fresh.emplace_back();capture_heads(sample,fresh.back());}
    }
    bool commit=!aborted && int(fresh.size())==samples && clock()<deadline;
    cerr<<"STRUCTURE_ASSIGN changed="<<(commit?changed:0)<<" proposed="<<changed<<" relocations="<<relocations
        <<" swaps="<<swaps<<" steps="<<steps<<" empirical="<<empirical<<" proxy="<<before<<"->"<<after
        <<" refreshed="<<fresh.size()<<" aborted="<<!commit<<" cpu="<<double(clock()-started)/CLOCKS_PER_SEC<<'\n';
    if(commit){design=std::move(candidate);history=std::move(fresh);}
    return commit;
}
} // namespace structure_assign

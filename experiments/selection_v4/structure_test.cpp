// Standalone research-only invariant/replay test. Compile separately from main.
#define ROUTE_ASSERT
#define main structure_submission_main
#include "structure_candidate.cpp"
#undef main
#include <fstream>

static void require(bool condition,const char* message) {
    if(!condition){cerr<<"STRUCTURE_TEST_FAIL "<<message<<'\n';exit(1);}
}
static void put_env(const char* key,const char* value) {
#ifdef _WIN32
    _putenv_s(key,value);
#else
    setenv(key,value,1);
#endif
}
static pair<Design,vector<HeadSample>> read_layout(const char* path) {
    ifstream f(path);require(bool(f),"open fixture");string magic;f>>magic;
    require(magic=="AHC074_LAYOUT_V1","fixture version");Design d;
    f>>d.W>>d.T>>d.C>>d.tot_B;d.S_init.resize(K);for(auto& s:d.S_init)f>>s;
    d.prod_to_paper.resize(P);for(auto& x:d.prod_to_paper)f>>x;
    d.prod_to_pair_idx.resize(P);for(auto& x:d.prod_to_pair_idx)f>>x;
    d.paper_bucket.resize(K);for(auto& x:d.paper_bucket)f>>x;
    d.paper_is_paired.resize(K);for(int k=0,x;k<K;k++){f>>x;d.paper_is_paired[k]=bool(x);}
    size_t n;f>>n;d.pairs.resize(n);
    for(auto& p:d.pairs)f>>p.u>>p.v>>p.u_top>>p.u_bot>>p.v_top>>p.v_bot>>p.bucket_w>>p.paper_id;
    d.internal_catalog.resize(P);for(auto& x:d.internal_catalog)f>>x;
    d.column_order.resize(N);for(auto& x:d.column_order)f>>x;
    f>>n;vector<HeadSample> heads(n);
    for(auto& h:heads){f>>h.black;for(auto& z:h.depth){int x;f>>x;z=uint8_t(x);}}
    require(bool(f),"complete fixture");return {move(d),move(heads)};
}
int main(int argc,char** argv) {
    require(argc==2,"usage: structure_test fixture.txt");
    auto [d,heads]=read_layout(argv[1]);const Design before=d;const size_t old_heads=heads.size();
    put_env("STRUCTURE_ASSIGN","1");put_env("STRUCTURE_CPU","10");
    const bool changed=structure_assign::optimize(d,heads,clock()+10*CLOCKS_PER_SEC);
    require(changed,"optimizer committed a changed layout");
    require(d.W==before.W && d.T==before.T && d.C==before.C && d.tot_B==before.tot_B,"core dimensions unchanged");
    require(d.internal_catalog==before.internal_catalog && d.column_order==before.column_order,"catalog and column map unchanged");
    require(d.prod_to_pair_idx==before.prod_to_pair_idx && d.pairs.size()==before.pairs.size(),"product group metadata unchanged");
    for(int k=0;k<d.tot_B;k++)require(d.S_init[k]==before.S_init[k],"core paper unchanged");
    vector<vector<int>> own(K);array<array<int,2>,64> colors{};
    int previous=-1;
    for(int a=0;a<P;a++) {
        int k=d.prod_to_paper[a],old=before.prod_to_paper[a];
        require(k>=d.tot_B && k<K,"main ID range");own[k].push_back(a);
        require(d.S_init[k].substr(d.T)==before.S_init[old].substr(d.T),"owner suffix unchanged");
        for(int j=0;j<N;j++)require(d.S_init[k][j]=='.'||d.S_init[k][j]==char('0'+int(d.internal_catalog[a]>>j&1)),"main ink agrees with owner");
        for(int b=0;b<P;b++)require((d.prod_to_paper[a]==d.prod_to_paper[b])==(before.prod_to_paper[a]==before.prod_to_paper[b]),"owner groups unchanged");
    }
    for(int k=d.tot_B;k<K;k++) {
        int w=d.paper_bucket[k];require(w>=0 && w<d.T && w>=previous,"initial staircase order");previous=w;
        require(own[k].size()==size_t(1+int(d.paper_is_paired[k])),"paper owner count");
        require(d.S_init[k][w]=='0'||d.S_init[k][w]=='1',"anchor is printed");
        colors[w][d.S_init[k][w]-'0']++;
        for(int j=w+1;j<d.T;j++)require(d.S_init[k][j]=='.',"cells after anchor are holes");
    }
    for(int w=0;w<d.T;w++)require(colors[w][0]>0&&colors[w][1]>0,"both anchor colors available");
    for(const auto& p:d.pairs)require(p.paper_id==d.prod_to_paper[p.u]&&p.paper_id==d.prod_to_paper[p.v]&&p.bucket_w==d.paper_bucket[p.paper_id],"pair anchor metadata");
    require(heads.size()==old_heads,"history completely refreshed");
    OnlineRouter router(d,d.internal_catalog);router.route_branching=true;
    vector<int> order(K);iota(order.begin(),order.end(),0);
    vector<uint64_t> ink(K),black(K);
    for(int k=0;k<K;k++)for(int j=0;j<N;j++){if(d.S_init[k][j]!='.')ink[k]|=1ULL<<j;if(d.S_init[k][j]=='1')black[k]|=1ULL<<j;}
    uint64_t rng=d.internal_catalog[0]|1;int last=-1;long long moves_count=0;
    for(int q=0;q<1024;q++) {
        int a;if(q<512)a=(251*q+17)%P;else do{rng^=rng<<7;rng^=rng>>9;a=int(rng%P);}while(a==last);
        last=a;auto moves=router.query(a);require(moves.size()<=2*K,"query move cap");
        for(auto [k,to]:moves) {
            require(k>=0&&k<K&&to>=0&&to<K,"move ranges");
            auto it=find(order.begin(),order.end(),k);uint64_t covered=0;
            for(auto x=order.begin();x!=it;x++)covered|=ink[*x];
            require((ink[k]&~covered)!=0,"moved paper visible");
            order.erase(it);order.insert(order.begin()+to,k);moves_count++;
        }
        uint64_t covered=0,display=0;for(int k:order){display|=black[k]&~covered;covered|=ink[k];}
        require(display==d.internal_catalog[a],"exact final barcode");
        require(order==router.sim.order,"router physical state matches independent replay");
    }
    cout<<"STRUCTURE_TEST_PASS requests=1024 moves="<<moves_count<<" heads="<<heads.size()<<'\n';
}

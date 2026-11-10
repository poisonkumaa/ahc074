#include <fstream>
#include <sstream>
// Research-only layout fixtures. Contains catalogue-derived design and head
// samples, never future requests. Submitted source does not include this file.
static std::string fixture_name(const std::vector<uint64_t>& original) {
    uint64_t h=0xc4da37b43e21f511ULL;
    for(auto x:original)h^=x+0x9e3779b97f4a7c15ULL+(h<<6)+(h>>2);
    std::ostringstream s;s<<std::hex<<h;
    return std::string(std::getenv("BEAM_FIXTURE_DIR"))+"/"+s.str()+".txt";
}
static void save_fixture(const Design& d,const std::vector<HeadSample>& heads,
                         const std::vector<uint64_t>& original) {
    if(!std::getenv("BEAM_FIXTURE_DIR"))return;
    std::ofstream f(fixture_name(original));assert(f);
    f<<"AHC074_LAYOUT_V1\n"<<d.W<<' '<<d.T<<' '<<d.C<<' '<<d.tot_B<<'\n';
    for(auto& s:d.S_init)f<<s<<'\n';
    for(auto a:d.prod_to_paper)f<<a<<' ';f<<'\n';
    for(auto a:d.prod_to_pair_idx)f<<a<<' ';f<<'\n';
    for(auto a:d.paper_bucket)f<<a<<' ';f<<'\n';
    for(auto a:d.paper_is_paired)f<<int(a)<<' ';f<<'\n';
    f<<d.pairs.size()<<'\n';
    for(auto& p:d.pairs)f<<p.u<<' '<<p.v<<' '<<p.u_top<<' '<<p.u_bot<<' '<<p.v_top<<' '<<p.v_bot<<' '<<p.bucket_w<<' '<<p.paper_id<<'\n';
    for(auto a:d.internal_catalog)f<<a<<' ';f<<'\n';
    for(auto a:d.column_order)f<<a<<' ';f<<'\n';
    f<<heads.size()<<'\n';
    for(auto& h:heads){f<<h.black;for(auto z:h.depth)f<<' '<<int(z);f<<'\n';}
    assert(f);
}
static void load_fixture(Design& d,std::vector<HeadSample>& heads,
                         const std::vector<uint64_t>& original) {
    assert(std::getenv("BEAM_FIXTURE_DIR"));
    std::ifstream f(fixture_name(original));assert(f);
    std::string magic;f>>magic;assert(magic=="AHC074_LAYOUT_V1");
    f>>d.W>>d.T>>d.C>>d.tot_B;
    d.S_init.resize(K);for(auto& s:d.S_init)f>>s;
    d.prod_to_paper.resize(P);for(auto& x:d.prod_to_paper)f>>x;
    d.prod_to_pair_idx.resize(P);for(auto& x:d.prod_to_pair_idx)f>>x;
    d.paper_bucket.resize(K);for(auto& x:d.paper_bucket)f>>x;
    d.paper_is_paired.resize(K);for(int k=0,x;k<K;k++){f>>x;d.paper_is_paired[k]=bool(x);}
    size_t size;f>>size;d.pairs.resize(size);
    for(auto& p:d.pairs)f>>p.u>>p.v>>p.u_top>>p.u_bot>>p.v_top>>p.v_bot>>p.bucket_w>>p.paper_id;
    d.internal_catalog.resize(P);for(auto& x:d.internal_catalog)f>>x;
    d.column_order.resize(N);for(auto& x:d.column_order)f>>x;
    f>>size;heads.resize(size);
    for(auto& h:heads){f>>h.black;for(auto& z:h.depth){int x;f>>x;z=uint8_t(x);}}
    assert(f);
    for(int a=0;a<P;a++)for(int j=0;j<N;j++)assert(((d.internal_catalog[a]>>j)^(original[a]>>d.column_order[j]))%2==0);
}

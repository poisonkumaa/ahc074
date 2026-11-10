// Small isolated test: no design search, actual orders, or large solver include.
#include <bits/stdc++.h>
constexpr int P=8,K=9;
namespace search_budget { static double real(const char*,double x){return x;} }
struct TestDistance { std::vector<uint8_t> distance; };
struct TestRequirement { std::shared_ptr<TestDistance> cached_distance; };
struct TestGraph {
    struct Table { int rank(const std::array<uint8_t,9>&)const{return 0;} } table;
    std::vector<TestRequirement> req;
};
struct OnlineRouter {
    struct Sim {std::vector<int> order;}sim;
    int tot_B=3,active_main=7;
    uint64_t index_main_mask=255;
    std::vector<int> paper_bucket,prod_to_paper;
    std::vector<uint64_t> A,black,opaque;
    std::unique_ptr<TestGraph> graph;
};
#include "selection_controls.inc"

int main() {
    OnlineRouter r;
    r.sim.order={7,0,2,3,6,1,4,8,5};
    r.paper_bucket={-1,-1,-1,0,1,2,0,1,2};
    r.prod_to_paper={3,3,4,4,5,6,7,8};
    r.A={170,85,27,219,152,66,107,214};
    r.black={255,0,128,173,151,14,64,2,4};
    r.opaque={255,255,255,249,234,39,213,119,236};
    r.graph=std::make_unique<TestGraph>();r.graph->req.resize(P);
    for(int a=0;a<P;a++)if(a%3) {
        r.graph->req[a].cached_distance=std::make_shared<TestDistance>();
        r.graph->req[a].cached_distance->distance={uint8_t((a*3)%4)};
    }
    int supports=0;
    for(int mask=1;mask<(1<<P);mask++) {
        std::vector<int> support;
        for(int a=0;a<P;a++)if(mask>>a&1)support.push_back(a);
        selection_controls::Feature sum{};
        for(int a:support) {
            auto control=selection_controls::get(r,a,support);
            for(int j=0;j<3;j++)sum[j]+=control[j];
        }
        for(double x:sum)if(std::abs(x)>1e-9) {
            std::cerr<<"Nonzero conditional control: "<<mask<<" "<<x<<"\n";return 1;
        }
        supports++;
    }
    selection_controls::PairedFit fit;
    const selection_controls::Feature target{.45,1.25,.7};
    for(int q=0;q<2048;q++) {
        fit.prepare();const auto before=fit.beta;
        std::vector<selection_controls::Feature> controls(2);
        double price=0;
        for(int j=0;j<3;j++) {
            controls[0][j]=(q>>j&1)?8:-8;
            controls[1][j]=-controls[0][j];
            price+=target[j]*controls[0][j];
        }
        fit.observe(controls,{100-price,100+price},{true,true});
        if(fit.beta!=before) {std::cerr<<"Current query changed coefficients\n";return 2;}
    }
    fit.prepare();
    for(int j=0;j<3;j++)if(std::abs(fit.beta[j]-target[j])>1e-3) {
        std::cerr<<"Regression mismatch: "<<j<<" "<<fit.beta[j]<<"\n";return 3;
    }
    std::cout<<"PASS "<<supports<<" conditional supports; fitted beta="
        <<fit.beta[0]<<","<<fit.beta[1]<<","<<fit.beta[2]<<"\n";
}

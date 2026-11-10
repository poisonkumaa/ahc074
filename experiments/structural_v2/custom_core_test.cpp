// Standalone safety test. Compile this file only; it includes the research
// solver with its interactive main renamed. It does not run that solver.
#include <set>
#include <stdexcept>
#define main ahc074_interactive_main_unused
#include "arbitrary_router.cpp"
#undef main

namespace custom_core_test {
static void check(bool condition,const char* message) {
    if(!condition)throw std::runtime_error(message);
}

static std::string key(const std::vector<int>& order) {
    std::string result;for(int k:order)result.push_back(char(k));return result;
}

// These checks deliberately use the printed strings rather than the router's
// opacity masks, graph transitions, requirement bits, or cached distances.
static bool visible(const std::vector<std::string>& sheets,const std::vector<int>& order,int paper) {
    auto found=std::find(order.begin(),order.end(),paper);
    check(found!=order.end(),"paper missing from independent deck");
    for(int j=0;j<64;j++)if(sheets[paper][j]!='.'){
        bool covered=false;
        for(auto p=order.begin();p!=found;p++)if(sheets[*p][j]!='.'){covered=true;break;}
        if(!covered)return true;
    }
    return false;
}

static uint64_t display(const std::vector<std::string>& sheets,const std::vector<int>& order) {
    uint64_t result=0;
    for(int j=0;j<64;j++)for(int k:order)if(sheets[k][j]!='.'){
        if(sheets[k][j]=='1')result|=uint64_t(1)<<j;
        break;
    }
    return result;
}

static Design make_design(int B,int family,std::vector<uint64_t>& outputs) {
    Design d{};d.W=64;d.T=0;d.C=0;d.tot_B=B;
    d.S_init.assign(K,std::string(64,'.'));
    d.prod_to_paper.assign(P,B); // The designated main is entirely transparent.
    d.prod_to_pair_idx.assign(P,0);
    d.paper_bucket.assign(K,-1);d.paper_is_paired.assign(K,false);
    d.column_order.resize(64);for(int j=0;j<64;j++)d.column_order[j]=j;
    std::vector<std::array<int,4>> masks;
    if(family==0){
        for(int a=0;a<B;a++)for(int b=a+1;b<B;b++)for(int c=b+1;c<B;c++)for(int e=c+1;e<B;e++)
            masks.push_back({a,b,c,e});
    }else if(B==5)masks={{{0,1,2,3}},{{1,2,3,4}}};
    else masks={{{0,1,2,3}},{{0,1,4,5}},{{2,3,4,5}}};
    const std::array<std::array<int,2>,6> black_positions{{{{0,1}},{{0,2}},{{0,3}},{{1,2}},{{1,3}},{{2,3}}}};
    for(int j=0;j<32;j++){
        const auto& mask=masks[j%masks.size()];
        const auto& black=black_positions[(j/masks.size()+3*j+family)%6];
        for(int p=0;p<4;p++){
            const char c=(p==black[0] || p==black[1])?'1':'0';
            d.S_init[mask[p]][j]=c;d.S_init[mask[p]][j+32]=char('0'+'1'-c);
        }
    }
    for(int j=0;j<64;j++){
        int zeros=0,ones=0;
        for(int k=0;k<B;k++){zeros+=d.S_init[k][j]=='0';ones+=d.S_init[k][j]=='1';}
        check(zeros==2 && ones==2,"test core must have four balanced opaque entries per column");
    }
    const auto& table=ExactBackingPermutations::get(B);
    outputs.resize(table.states);
    std::vector<uint64_t> distinct;std::set<uint64_t> seen;
    for(int s=0;s<table.states;s++){
        std::vector<int> order(B);for(int i=0;i<B;i++)order[i]=table.permutations[s][i];
        outputs[s]=display(d.S_init,order);
        check(__builtin_popcountll(outputs[s])==32,"complementary columns did not produce 32 ones");
        if(seen.insert(outputs[s]).second)distinct.push_back(outputs[s]);
    }
    check(!distinct.empty() && distinct.size()<=P,"unexpected number of test targets");
    d.internal_catalog=distinct;
    uint64_t rng=0x192d8f34a9b7ULL^uint64_t(31*B+family);
    while(d.internal_catalog.size()<P){rng^=rng<<7;rng^=rng>>9;d.internal_catalog.push_back(outputs[rng%outputs.size()]);}
    return d;
}

struct IndependentGraph {
    std::vector<std::vector<int>> reverse;
    std::vector<std::set<std::tuple<int,int,int>>> edges;
    IndependentGraph(const Design& d) {
        const int B=d.tot_B;const auto& table=ExactBackingPermutations::get(B);
        reverse.resize(table.states);edges.resize(table.states);
        std::map<std::string,int> ids;
        for(int s=0;s<table.states;s++){
            std::vector<int> order(B);for(int i=0;i<B;i++)order[i]=table.permutations[s][i];ids.emplace(key(order),s);
        }
        for(int s=0;s<table.states;s++){
            std::vector<int> order(B);for(int i=0;i<B;i++)order[i]=table.permutations[s][i];
            for(int i=0;i<B;i++)if(visible(d.S_init,order,order[i]))for(int to=0;to<B;to++)if(to!=i){
                const int paper=order[i];auto next=order;
                next.erase(next.begin()+i);next.insert(next.begin()+to,paper);
                auto found=ids.find(key(next));check(found!=ids.end(),"independent transition lost a permutation");
                edges[s].insert({found->second,paper,to});reverse[found->second].push_back(s);
            }
        }
    }
    std::vector<int> distances(uint64_t target,const std::vector<uint64_t>& outputs) const {
        std::vector<int> distance(outputs.size(),-1),queue;
        for(int s=0;s<int(outputs.size());s++)if(outputs[s]==target){distance[s]=0;queue.push_back(s);}
        check(!queue.empty(),"test target has no literal goal");
        for(size_t h=0;h<queue.size();h++)for(int previous:reverse[queue[h]])if(distance[previous]<0){
            distance[previous]=distance[queue[h]]+1;queue.push_back(previous);
        }
        check(queue.size()==outputs.size(),"independent graph is not strongly connected to target");
        return distance;
    }
};

static long long run_case(int B,int family,bool lazy) {
    std::vector<uint64_t> outputs;Design d=make_design(B,family,outputs);
    const auto& table=ExactBackingPermutations::get(B);
    IndependentGraph independent(d);
    requirement_distance_cache::entries().clear();
    requirement_distance_cache::statistics()=requirement_distance_cache::Statistics{};
    if(lazy)check(clock()>1,"lazy case requires an already expired positive CPU deadline");
    ExactGraphBacking graph(d,d.internal_catalog,lazy?clock_t(1):clock_t(0));
    check(graph.custom && graph.generic && !graph.complete,"synthetic core did not activate custom routing");
    for(int s=0;s<table.states;s++){
        std::set<std::tuple<int,int,int>> actual;
        for(const auto& e:graph.custom_edges[s])actual.insert({e.to,e.operation>>4,e.operation&15});
        check(actual==independent.edges[s],"custom edges differ from string-based legal moves");
    }
    if(lazy)for(const auto& r:graph.req)check(!r.cached_distance,"expired case unexpectedly has precomputed requirement distances");
    std::map<uint64_t,int> representative;
    for(int a=0;a<P;a++)representative.emplace(d.internal_catalog[a],a);
    std::vector<int> products;
    for(const auto& entry:representative)products.push_back(entry.second);
    const int requested=std::max(1,search_budget::integer("CUSTOM_CORE_TEST_TARGETS",16));
    std::vector<int> chosen;
    const int count=std::min(requested,int(products.size()));
    for(int i=0;i<count;i++)chosen.push_back(products[size_t(i)*products.size()/count]);
    chosen.push_back(P-1);chosen.push_back(P-2); // Duplicate catalogue targets use distinct request IDs.
    std::map<uint64_t,std::vector<int>> distances;
    DeckTracker deck;deck.init(d.S_init);
    long long checked=0,moves_total=0;
    auto exercise=[&](int start,int product){
        for(int k=0;k<K;k++)deck.order[k]=k;
        for(int i=0;i<B;i++)deck.order[i]=table.permutations[start][i];
        deck.rebuild_positions();const auto initial=deck.order;
        std::vector<std::pair<int,int>> moves;graph.route(deck,product,moves);
        check(moves.size()<=size_t(B-1),"custom route exceeded the top-insertion bound");
        auto replay=initial;
        for(auto [paper,depth]:moves){
            check(paper>=0 && paper<B && depth>=0 && depth<B,"custom route moved outside the core");
            check(visible(d.S_init,replay,paper),"custom route selected an invisible paper");
            auto at=std::find(replay.begin(),replay.end(),paper);check(at!=replay.end(),"moved paper missing");
            replay.erase(at);replay.insert(replay.begin()+depth,paper);
        }
        check(replay==deck.order,"independent replay differs from router's final order");
        for(int i=0;i<K;i++)check(deck.position[deck.order[i]]==i,"deck position index is inconsistent");
        const uint64_t target=d.internal_catalog[product];
        check(display(d.S_init,replay)==target,"custom route displayed the wrong target");
        auto found=distances.find(target);
        if(found==distances.end())found=distances.emplace(target,independent.distances(target,outputs)).first;
        check(int(moves.size())==found->second[start],"custom route is not independently shortest");
        checked++;moves_total+=moves.size();return moves;
    };
    for(int product:chosen)for(int start=0;start<table.states;start++)exercise(start,product);
    const size_t hits=graph.route_cache_hits;
    const auto first=exercise(table.states-1,chosen[0]);
    const auto second=exercise(table.states-1,chosen[0]);
    check(first==second && graph.route_cache_hits>hits,"route cache replay was not exercised correctly");
    int lazy_built=0;for(const auto& r:graph.req)lazy_built+=!r.custom_distance.empty();
    if(lazy)check(lazy_built>0,"mandatory lazy distance construction was not exercised");
    std::cout<<"PASS B="<<B<<" family="<<family<<" lazy="<<lazy<<" states="<<table.states
             <<" targets="<<chosen.size()<<" routes="<<checked<<" moves="<<moves_total
             <<" lazy_distances="<<lazy_built<<" cache_hits="<<graph.route_cache_hits<<'\n';
    return checked;
}
} // namespace custom_core_test

int main() {
    try {
#if defined(_WIN32)
        _putenv_s("CORE_ARBITRARY","1");
#else
        setenv("CORE_ARBITRARY","1",1);
#endif
        long long total=0;
        for(int B:{5,6}){
            total+=custom_core_test::run_case(B,0,false);
            total+=custom_core_test::run_case(B,1,true);
        }
        std::cout<<"PASS custom labeled routing: "<<total<<" independently checked routes\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"FAIL custom labeled routing: "<<e.what()<<'\n';return 1;
    }
}

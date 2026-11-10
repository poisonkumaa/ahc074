// Standalone research check. No contest input is read or emitted.
// Build separately; the production main is included only to expose its types.
#define main ahc074_original_main_for_legal_metric_test
#include "legal_metric_candidate.cpp"
#undef main
#include <numeric>
#include <stdexcept>

namespace independent_legal_metric_test {
static void check(bool ok,const char* message) {
    if(!ok)throw std::runtime_error(message);
}
struct Edge {int to,paper,destination;};
struct LiteralGraph {
    int n;
    std::vector<std::string> sheets;
    std::vector<std::vector<int>> orders;
    std::map<std::vector<int>,int> index;
    std::vector<std::vector<Edge>> edges;
    std::vector<std::vector<int>> distances,unrestricted;
    explicit LiteralGraph(int size):n(size),sheets(n,std::string(n*(n-1)/2,'.')) {
        int column=0;
        for(int a=0;a<n;a++)for(int b=a+1;b<n;b++,column++) {
            sheets[a][column]='0';sheets[b][column]='1';
        }
        std::vector<int> p(n);std::iota(p.begin(),p.end(),0);
        do{index[p]=int(orders.size());orders.push_back(p);}while(std::next_permutation(p.begin(),p.end()));
        edges.resize(orders.size());
        for(int id=0;id<int(orders.size());id++) {
            const auto visible=visibility(orders[id]);
            for(int from=0;from<n;from++)if(visible[orders[id][from]])
                for(int to=0;to<n;to++)if(from!=to) {
                    auto next=orders[id];const int paper=next[from];
                    next.erase(next.begin()+from);next.insert(next.begin()+to,paper);
                    edges[id].push_back({index.at(next),paper,to});
                }
        }
        distances.assign(orders.size(),std::vector<int>(orders.size(),999));
        unrestricted=distances;
        for(int start=0;start<int(orders.size());start++) {
            std::vector<int> queue{start};distances[start][start]=0;
            for(size_t head=0;head<queue.size();head++)for(const auto& edge:edges[queue[head]])
                if(distances[start][edge.to]==999) {
                    distances[start][edge.to]=distances[start][queue[head]]+1;queue.push_back(edge.to);
                }
            queue={start};unrestricted[start][start]=0;
            for(size_t head=0;head<queue.size();head++)for(int from=0;from<n;from++)
                for(int to=0;to<n;to++)if(from!=to) {
                    auto next=orders[queue[head]];const int paper=next[from];
                    next.erase(next.begin()+from);next.insert(next.begin()+to,paper);
                    const int id=index.at(next);
                    if(unrestricted[start][id]==999) {
                        unrestricted[start][id]=unrestricted[start][queue[head]]+1;queue.push_back(id);
                    }
                }
        }
    }
    std::vector<bool> visibility(const std::vector<int>& order) const {
        std::vector<bool> visible(n,false);
        for(int column=0;column<int(sheets[0].size());column++)
            for(int k:order)if(sheets[k][column]!='.') {visible[k]=true;break;}
        return visible;
    }
    bool goal(const std::array<uint16_t,16>& predecessors,const std::vector<int>& order) const {
        std::vector<int> position(n);
        for(int i=0;i<n;i++)position[order[i]]=i;
        for(int v=0;v<n;v++)for(int u=0;u<n;u++)
            if((predecessors[v]>>u&1) && position[u]>=position[v])return false;
        return true;
    }
    std::array<uint16_t,16> transpose(const std::array<uint16_t,16>& predecessors) const {
        std::array<uint16_t,16> result{};
        for(int v=0;v<n;v++)for(int u=0;u<n;u++)if(predecessors[v]>>u&1)result[u]|=1u<<v;
        return result;
    }
    double mean(const std::array<uint16_t,16>& predecessors,
                const std::vector<std::array<int,16>>& ranks,bool all) const {
        std::vector<int> goals,starts;
        for(int id=0;id<int(orders.size());id++)if(goal(predecessors,orders[id]))goals.push_back(id);
        check(!goals.empty(),"independent DAG has no goal");
        if(all || ranks.empty())for(int id=0;id<int(orders.size());id++)starts.push_back(id);
        else for(const auto& rank:ranks) {
            std::vector<int> order(n);
            for(int k=0;k<n;k++)order[rank[k]]=k;
            starts.push_back(index.at(order));
        }
        int total=0;
        for(int start:starts) {
            int best=999;for(int target:goals)best=std::min(best,distances[start][target]);
            check(best<999,"literal forward graph disconnected");total+=best;
        }
        return double(total)/starts.size();
    }
    int replay_shortest(int source,int target) const {
        auto order=orders[source];int current=source,count=0;
        while(current!=target) {
            bool found=false;
            for(const auto& edge:edges[current])if(distances[edge.to][target]+1==distances[current][target]) {
                check(visibility(order)[edge.paper],"witness attempts invisible sheet");
                const auto at=std::find(order.begin(),order.end(),edge.paper);
                order.erase(at);order.insert(order.begin()+edge.destination,edge.paper);
                check(index.at(order)==edge.to,"literal operation replay destination mismatch");
                current=edge.to;count++;found=true;break;
            }
            check(found && count<=n,"no shortest witness or route too long");
        }
        check(order==orders[target],"literal replay target mismatch");
        return count;
    }
};
static std::array<uint16_t,16> closure_of(std::array<uint16_t,16> p,int n) {
    for(int k=0;k<n;k++)for(int v=0;v<n;v++)if(p[v]>>k&1)p[v]|=p[k];
    return p;
}
static void clear_cache() {
    requirement_distance_cache::entries().clear();
    requirement_distance_cache::statistics()={};
}
static int run() {
    clear_cache();
    LiteralGraph three(3);
    const int start=three.index.at({0,1,2}),target=three.index.at({2,0,1});
    check(three.distances[start][target]==2,"bottom-to-top legal distance must be two");
    check(three.unrestricted[start][target]==1,"bottom-to-top unrestricted distance must be one");
    check(three.replay_shortest(start,target)==2,"bottom-to-top witness must use two visible moves");
    std::array<uint16_t,16> before{};before[0]=1u<<2;before[1]=1u<<0;
    std::array<int,16> identity{};for(int k=0;k<3;k++)identity[k]=k;
    std::vector<std::array<int,16>> single{identity};
    check(graph_dag_metric(before.data(),3,3,single)==1,"old metric should see one unrestricted move");
    legal_pair_metric::Evaluator sampled(3,single,0,false);
    double first=0;
    check(sampled.pair(before.data(),first),"sampled legal pair metric failed");
    const double expected=.5*(three.mean(before,single,false)+three.mean(three.transpose(before),single,false));
    check(first==expected && first==1.5,"endpoint orientation average is wrong");
    check(sampled.hits==0 && sampled.bfs_runs==2,"first request must build both labeled orientations");
    check(requirement_distance_cache::entries().size()==2,"endpoint keys were incorrectly collapsed");
    double repeated=0;
    check(sampled.pair(before.data(),repeated) && repeated==first,"cache hit changed legal mean");
    check(sampled.hits==2 && sampled.bfs_runs==2,"repeated pair did not reuse both complete tables");
    double forward=0,backward=0;
    check(sampled.orientation(closure_of(before,3),forward),"forward orientation lookup failed");
    check(sampled.orientation(closure_of(three.transpose(before),3),backward),"reverse orientation lookup failed");
    check(forward==2 && backward==1,"directed endpoint costs must remain distinct");
    int checked=0;
    for(int n:{3,4}) {
        LiteralGraph graph(n);
        const auto samples=graph_sample_orders(0xb5c42d1f835b8279ULL,n,128);
        const int arcs=n*(n-1)/2;
        legal_pair_metric::Evaluator all(n,samples,0,true),sample(n,samples,0,false);
        for(int mask=0;mask<(1<<arcs);mask++)for(int rotation=0;rotation<n;rotation++) {
            std::array<uint16_t,16> requirement{};int edge=0;
            for(int u=0;u<n;u++)for(int v=u+1;v<n;v++,edge++)if(mask>>edge&1)
                requirement[(v+rotation)%n]|=1u<<((u+rotation)%n);
            for(bool full:{false,true}) {
                double got=0;
                check((full?all:sample).pair(requirement.data(),got),"DAG metric calculation failed");
                const double want=.5*(graph.mean(requirement,samples,full)+
                    graph.mean(graph.transpose(requirement),samples,full));
                check(std::abs(got-want)<1e-12,"reverse BFS disagrees with independent literal forward BFS");
                checked++;
            }
        }
        for(int a=0;a<int(graph.orders.size());a++)for(int b=0;b<int(graph.orders.size());b++)
            check(graph.replay_shortest(a,b)==graph.distances[a][b],"literal shortest witness replay failed");
    }
    clear_cache();
    legal_pair_metric::Evaluator warm(3,single,0,false);
    double warm_mean=0;check(warm.pair(before.data(),warm_mean),"cache warmup failed");
    const size_t entries=requirement_distance_cache::entries().size();
    const size_t bytes=requirement_distance_cache::statistics().accounted_bytes;
    const clock_t expired_at=std::max<clock_t>(1,clock());
    while(clock()<expired_at){} // At most one clock tick on coarse Windows clocks.
    legal_pair_metric::Evaluator expired(4,{},expired_at,true);
    std::array<uint16_t,16> unseen{};unseen[2]=1u<<0;unseen[3]=1u<<1;
    double untouched=-123;
    check(!expired.pair(unseen.data(),untouched) && expired.aborted,"expired optional work must fail");
    check(untouched==-123,"failed pair published a partial mean");
    check(requirement_distance_cache::entries().size()==entries &&
          requirement_distance_cache::statistics().accounted_bytes==bytes,
          "failed optional work published a partial cache entry");
    std::cerr<<"LEGAL_METRIC_TEST passed cases="<<checked
        <<" literal_replays="<<(6*6+24*24)<<" bottom_to_top_legal=2 unrestricted=1"
        <<" endpoint_mean=1.5 cache=pass deadline=pass\n";
    return 0;
}
} // namespace independent_legal_metric_test
int main() {
    try{return independent_legal_metric_test::run();}
    catch(const std::exception& e){std::cerr<<"LEGAL_METRIC_TEST failure="<<e.what()<<"\n";return 1;}
}

// Exact directed distances for a complete comparison core. Every paper except
// the bottom paper is visible. Requirements use predecessor masks, matching
// comparison_pair_dag and ExactGraphBacking::Requirement.
namespace legal_pair_metric {
struct Evaluator {
    int n;
    const ExactBackingPermutations& table;
    clock_t deadline;
    bool all_permutations;
    std::vector<int> starts;
    size_t requests=0,hits=0,bfs_runs=0;
    bool aborted=false;
    Evaluator(int count,const std::vector<std::array<int,16>>& samples,
              clock_t stop,bool all):n(count),table(ExactBackingPermutations::get(count)),
              deadline(stop),all_permutations(all) {
        if(all_permutations || samples.empty()) {
            starts.reserve(table.states);
            for(int id=0;id<table.states;id++)starts.push_back(id);
        }else {
            starts.reserve(samples.size());
            for(const auto& rank:samples) {
                std::array<uint8_t,9> order{};
                for(int k=0;k<n;k++)order[rank[k]]=uint8_t(k);
                starts.push_back(table.rank(order));
            }
        }
    }
    bool expired() {
        if(deadline>0 && clock()>=deadline)aborted=true;
        return aborted;
    }
    bool orientation(const std::array<uint16_t,16>& closure,double& mean) {
        if(expired())return false;
        requests++;
        unsigned constrained=0;
        for(int k=0;k<n;k++) {
            if(closure[k]&(1u<<k)){aborted=true;return false;}
            constrained|=closure[k];
        }
        if(!constrained){mean=0;return true;}
        // Do not transpose-canonicalize: reversing a target partial order need
        // not preserve distances in the bottom-forbidden directed move graph.
        const auto key=requirement_distance_cache::key(n,closure);
        auto cached=requirement_distance_cache::lookup(key);
        std::vector<int> goals,queue;
        std::vector<uint8_t> distance;
        if(cached)hits++;
        else {
            if(expired())return false;
            auto enumerate=[&](auto&& self,unsigned used,int depth,int rank)->void {
                if(depth==n){goals.push_back(rank);return;}
                for(int k=0;k<n;k++)if(!(used>>k&1) && !(closure[k]&~used)) {
                    const int add=__builtin_popcount(((1u<<k)-1)&~used)*table.fact[n-1-depth];
                    self(self,used|(1u<<k),depth+1,rank+add);
                }
            };
            enumerate(enumerate,0,0,0);
            if(goals.empty()){aborted=true;return false;}
            distance.assign(table.states,255);queue.reserve(table.states);
            for(int id:goals){distance[id]=0;queue.push_back(id);}
            for(size_t head=0;head<queue.size();head++) {
                if(!(head&31) && expired())return false;
                const int id=queue[head];
                for(int before:table.reverse_edges[id])if(distance[before]==255) {
                    distance[before]=uint8_t(distance[id]+1);queue.push_back(before);
                }
            }
            if(int(queue.size())!=table.states){aborted=true;return false;}
            if(expired())return false;
            bfs_runs++;requirement_distance_cache::statistics().bfs_runs++;
            cached=requirement_distance_cache::remember(key,goals,distance);
        }
        const auto& values=cached?cached->distance:distance;
        assert(int(values.size())==table.states);
        unsigned sum=0;for(int id:starts)sum+=values[id];
        mean=double(sum)/starts.size();
        return !expired();
    }
    bool pair(const uint16_t* before,double& mean) {
        std::array<uint16_t,16> closure{},opposite{};
        for(int k=0;k<n;k++)closure[k]=before[k];
        for(int k=0;k<n;k++)for(int v=0;v<n;v++)
            if(closure[v]&(1u<<k))closure[v]|=closure[k];
        for(int v=0;v<n;v++)for(unsigned row=closure[v];row;row&=row-1)
            opposite[__builtin_ctz(row)]|=uint16_t(1u<<v);
        double first=0,second=0;
        if(!orientation(closure,first) || !orientation(opposite,second))return false;
        mean=.5*(first+second);
        return true;
    }
};
} // namespace legal_pair_metric

struct ReorderStatistics {
    long long trials=0,nodes=0,completed=0,improved=0,saved=0,aborted=0,invalid=0;
    double cpu=0;
};
inline ReorderStatistics& reorder_stats(){static ReorderStatistics value;return value;}
inline bool reorder_enabled(){static const bool value=search_budget::integer("ROUTE_REORDER",0)!=0;return value;}

// Keep the final deck exactly. Each selected paper is either left in place or
// moved once, in any legal order. After fixing a paper, its relative order with
// every previously fixed paper agrees with the final deck. Unfixed papers may
// occupy any gap, so all insertion positions within the permitted gap are tried.
inline std::vector<Move> reorder_without(const std::vector<int>& start,
        const std::vector<Move>& original,const std::vector<int>& target,
        const std::array<int,K>& counts,const std::array<int,K>& target_position,
        const std::vector<uint64_t>& opaque,int omitted,int maximum_moves,clock_t deadline) {
    auto& log=reorder_stats();
    static const double budget=search_budget::seconds(std::max(0.0,search_budget::real("ROUTE_REORDER_CPU",.15)));
    static const int node_limit=std::clamp(search_budget::integer("ROUTE_REORDER_NODES",1000),1,100000);
    if(!reorder_enabled() || log.cpu>=budget || routing_budget::expired(deadline))return original;
    const clock_t begun=clock();
    const clock_t cap=begun+clock_t(std::max(0.0,budget-log.cpu)*CLOCKS_PER_SEC);
    const clock_t stop=deadline>0?std::min(deadline,cap):cap;
    std::array<int,K> index;index.fill(-1);
    std::vector<int> movers;
    uint64_t fixed_above=0;int previous=-1;
    for(int paper:start) {
        const bool fixed=counts[paper]==0 || paper==omitted || !(opaque[paper]&~fixed_above);
        if(fixed) {
            if(target_position[paper]<previous)return original;
            previous=target_position[paper];fixed_above|=opaque[paper];
        }else movers.push_back(paper);
    }
    if(movers.empty())return start==target?std::vector<Move>{}:original;
    if(movers.size()>24)return original;
    std::array<int,K> priority;priority.fill(int(original.size()));
    for(int i=0;i<int(original.size());i++)priority[original[i].first]=std::min(priority[original[i].first],i);
    std::stable_sort(movers.begin(),movers.end(),[&](int a,int b){return priority[a]<priority[b];});
    for(int i=0;i<int(movers.size());i++)index[movers[i]]=i;
    std::array<uint8_t,K> order{},position{};
    for(int i=0;i<K;i++){order[i]=uint8_t(start[i]);position[start[i]]=uint8_t(i);}
    std::vector<Move> path,answer;
    std::unordered_map<std::string,int> failed;failed.reserve(node_limit*2);
    const unsigned full=(1u<<movers.size())-1;
    int nodes=0;bool timed_out=false;
    auto relocate=[&](int from,int to) {
        const uint8_t paper=order[from];
        if(from<to)std::memmove(order.data()+from,order.data()+from+1,size_t(to-from));
        else if(from>to)std::memmove(order.data()+to+1,order.data()+to,size_t(from-to));
        order[to]=paper;
        for(int i=std::min(from,to);i<=std::max(from,to);i++)position[order[i]]=uint8_t(i);
    };
    auto search=[&](auto&& self,unsigned pending)->bool {
        if(int(path.size())>=maximum_moves)return false;
        if(!pending) {
            for(int i=0;i<K;i++)if(order[i]!=target[i])return false;
            answer=path;return true;
        }
        if(nodes>=node_limit){timed_out=true;return false;}
        if(!(nodes&31) && clock()>=stop){timed_out=true;return false;}
        nodes++;
        std::string key;key.reserve(movers.size()+4);
        for(int paper:movers)key.push_back(char(position[paper]));
        for(int byte=0;byte<4;byte++)key.push_back(char(pending>>(8*byte)));
        auto prior=failed.find(key);
        if(prior!=failed.end() && prior->second<=int(path.size()))return false;
        std::array<uint64_t,K+1> covered{};
        for(int i=0;i<K;i++)covered[i+1]=covered[i]|opaque[order[i]];
        for(unsigned bits=pending;bits;bits&=bits-1) {
            const int id=__builtin_ctz(bits),paper=movers[id],from=position[paper];
            const bool visible_now=bool(opaque[paper]&~covered[from]);
            int left=-1,right=K;
            for(int d=0;d<K;d++) {
                const int other=order[d],oi=index[other];
                if(oi>=0 && (pending>>oi&1))continue;
                if(target_position[other]<target_position[paper])left=d;
                else {right=d;break;}
            }
            const int lo=left+1-int(from<left),hi=right-int(from<right);
            const unsigned next=pending^(1u<<id);
            // A recorded move may become unnecessary after other moves. Such a
            // paper can be fixed without requiring it to be visible.
            if(from>=lo && from<=hi)if(self(self,next))return true;
            if(timed_out)break;
            if(!visible_now)continue;
            auto attempt=[&](int to)->bool {
                if(to==from)return false;
                relocate(from,to);path.emplace_back(paper,to);
                const bool success=self(self,next);
                path.pop_back();relocate(to,from);
                return success;
            };
            if(attempt(hi))return true;
            if(timed_out)break;
            if(lo!=hi && attempt(lo))return true;
            if(timed_out)break;
            for(int to=lo+1;to<hi;to++) {
                if(attempt(to))return true;
                if(timed_out)break;
            }
            if(timed_out)break;
        }
        if(!timed_out) {
            auto prior=failed.find(key);
            if(prior==failed.end())failed.emplace(std::move(key),int(path.size()));
            else prior->second=std::min(prior->second,int(path.size()));
        }
        return false;
    };
    log.trials++;
    const bool found=search(search,full);
    log.nodes+=nodes;log.aborted+=timed_out;
    log.cpu+=double(clock()-begun)/CLOCKS_PER_SEC;
    if(!found)return original;
    log.completed++;
    auto replay=start;
    for(auto op:answer) {
        if(op.second<0 || op.second>=K || !visible(replay,op.first,opaque)) {log.invalid++;return original;}
        replay=after_move(std::move(replay),op);
    }
    if(replay!=target || int(answer.size())>=maximum_moves){log.invalid++;return original;}
    log.improved++;log.saved+=maximum_moves-int(answer.size());
    return answer;
}

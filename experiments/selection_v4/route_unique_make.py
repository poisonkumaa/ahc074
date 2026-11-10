from pathlib import Path

directory = Path(__file__).resolve().parent
source = (directory / "main_before_v4.cpp").read_text(encoding="utf-8")
start = source.index("inline std::vector<Move> optimize(", source.index("namespace trace_shortener {"))
end = source.index("} // namespace trace_shortener", start)
replacement = r'''struct UniqueStatistics {
    long long calls=0,candidates=0,filtered=0,trials=0,improved=0,saved=0,invalid=0;
};
inline UniqueStatistics& unique_stats(){static UniqueStatistics value;return value;}
inline bool unique_enabled(){static const bool value=search_budget::integer("ROUTE_UNIQUE",0)!=0;return value;}

// If one operation is removed, every never-moved paper and that single-use
// paper become immutable. They must retain their relative target order. Also,
// a remaining mover needs an ink cell not permanently hidden by immutable
// papers initially above it; those papers stay above until its first move.
inline bool unique_possible(const std::vector<int>& start,const std::array<int,K>& counts,
                            const std::array<int,K>& final_position,
                            const std::vector<uint64_t>& opaque,int omitted_paper) {
    int previous=-1;uint64_t fixed_above=0;
    for(int paper:start) {
        if(counts[paper]==0 || paper==omitted_paper) {
            if(final_position[paper]<previous)return false;
            previous=final_position[paper];fixed_above|=opaque[paper];
        }else if(!(opaque[paper]&~fixed_above))return false;
    }
    return true;
}

inline std::vector<Move> optimize(const std::vector<int>& start,std::vector<Move> moves,
                                  const std::vector<uint64_t>& opaque,clock_t optional_deadline=0) {
    if(routing_budget::expired(optional_deadline)) {
        routing_budget::counters().trace_skipped_calls++;return moves;
    }
    const bool unique=unique_enabled();
    if(unique)unique_stats().calls++;
    while(true) {
        std::array<int,K> counts{};
        for(auto op:moves)counts[op.first]++;
        std::vector<int> final_order;
        std::array<int,K> final_position{};
        if(unique) {
            final_order=start;
            for(auto op:moves)final_order=after_move(std::move(final_order),op);
            for(int i=0;i<K;i++)final_position[final_order[i]]=i;
        }
        auto best=moves;
        for(int i=0;i<int(moves.size());i++) {
            const bool single=counts[moves[i].first]==1;
            if(single) {
                if(!unique)continue;
                unique_stats().candidates++;
                if(!unique_possible(start,counts,final_position,opaque,moves[i].first)) {
                    unique_stats().filtered++;continue;
                }
            }
            if(routing_budget::expired(optional_deadline)) {
                routing_budget::counters().trace_skipped_calls++;return best;
            }
            if(single)unique_stats().trials++;
            auto candidate=omit_one(start,moves,opaque,i,optional_deadline);
            if(candidate.size()<best.size()) {
                if(single) {
                    auto order=start;bool legal=true;
                    for(auto op:candidate) {
                        if(op.second<0 || op.second>=K || !visible(order,op.first,opaque)) {legal=false;break;}
                        order=after_move(std::move(order),op);
                    }
                    if(!legal || order!=final_order){unique_stats().invalid++;continue;}
                    unique_stats().improved++;unique_stats().saved+=best.size()-candidate.size();
                }
                best=std::move(candidate);
            }
        }
        if(best.size()==moves.size())return moves;
        moves=std::move(best);
    }
}
'''
source = source[:start] + replacement + source[end:]
anchor = '    cerr<<"TRACE_SHORTEN saved="<<trace_saved<<" queries="<<trace_queries<<"\\n";'
assert source.count(anchor) == 1
source = source.replace(anchor, anchor + r'''
    const auto&us=trace_shortener::unique_stats();
    cerr<<"ROUTE_UNIQUE enabled="<<trace_shortener::unique_enabled()<<" calls="<<us.calls
        <<" candidates="<<us.candidates<<" filtered="<<us.filtered<<" trials="<<us.trials
        <<" improved="<<us.improved<<" saved="<<us.saved<<" invalid="<<us.invalid<<"\n";
''')
(directory / "route_unique.cpp").write_text(source, encoding="utf-8")

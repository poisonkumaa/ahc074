from pathlib import Path

directory = Path(__file__).resolve().parent
source = (directory / "route_unique.cpp").read_text(encoding="utf-8")
source = source.replace('inline bool unique_enabled(){static const bool value=search_budget::integer("ROUTE_UNIQUE",0)!=0;return value;}',
    'inline bool unique_enabled(){static const bool value=search_budget::integer("ROUTE_UNIQUE",0)!=0 || search_budget::integer("ROUTE_REORDER",0)!=0;return value;}')
source = source.replace('''        if(counts[paper]==0 || paper==omitted_paper) {
            if(final_position[paper]<previous)return false;
            previous=final_position[paper];fixed_above|=opaque[paper];
        }else if(!(opaque[paper]&~fixed_above))return false;''', '''        if(counts[paper]==0 || paper==omitted_paper || !(opaque[paper]&~fixed_above)) {
            if(final_position[paper]<previous)return false;
            previous=final_position[paper];fixed_above|=opaque[paper];
        }''')
anchor = 'inline std::vector<Move> optimize(const std::vector<int>& start,std::vector<Move> moves,'
assert source.count(anchor) == 1
source = source.replace(anchor, '#include "route_reorder_search.inc"\n\n' + anchor)
anchor = '''            auto candidate=omit_one(start,moves,opaque,i,optional_deadline);
            if(candidate.size()<best.size()) {'''
assert source.count(anchor) == 1
source = source.replace(anchor, '''            auto candidate=omit_one(start,moves,opaque,i,optional_deadline);
            if(single && candidate.size()>=best.size() && reorder_enabled())
                candidate=reorder_without(start,moves,final_order,counts,final_position,opaque,
                    moves[i].first,int(best.size()),optional_deadline);
            if(candidate.size()<best.size()) {''')
anchor = '    const auto&nr=nov5_relaxed_trace::stats();'
assert source.count(anchor) == 1
source = source.replace(anchor, r'''    const auto&rs=trace_shortener::reorder_stats();
    cerr<<"ROUTE_REORDER enabled="<<trace_shortener::reorder_enabled()<<" trials="<<rs.trials
        <<" nodes="<<rs.nodes<<" completed="<<rs.completed<<" improved="<<rs.improved
        <<" saved="<<rs.saved<<" aborted="<<rs.aborted<<" invalid="<<rs.invalid<<" cpu="<<rs.cpu<<"\n";
''' + anchor)
(directory / "route_reorder.cpp").write_text(source, encoding="utf-8")

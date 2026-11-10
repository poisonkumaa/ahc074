from pathlib import Path


directory = Path(__file__).resolve().parent
source = (directory / "pair_snapshot_candidate.cpp").read_text(encoding="utf-8")
baseline = (directory / "main_before_append.cpp").read_text(encoding="utf-8")
start = '    for(int i=0;i<min(int(snapshots.size()),3);i++){'
end = '    if(candidates.empty())return fallback();'
assert source.count(start) == baseline.count(start) == 1
assert source.count(end) == baseline.count(end) == 1
source_start, source_end = source.index(start), source.index(end)
baseline_start, baseline_end = baseline.index(start), baseline.index(end)
original_loop = baseline[baseline_start:baseline_end]
append_loop = '''    const int original_candidate_count=int(candidates.size());
    int appended_candidates=0;
    const bool append_enabled=search_budget::integer("PAIR_SNAPSHOT",0)!=0;
    if(append_enabled) {
        const int append_count=std::max(0,search_budget::integer("PAIR_APPEND_COUNT",1));
        for(int i=0;i<std::min(int(snapshots.size()),append_count);i++) {
            if(design_elapsed()>layout_deadline-.08)break;
            graph_seed_offset=pool+3+i;
            const double cap=search_budget::seconds(std::max(0.0,search_budget::real("PAIR_SNAPSHOT_CAP",.06)));
            const clock_t stop=std::min(absolute_deadline(search_budget::real("PAIR_SNAPSHOT_END",layout_deadline-.06)),
                clock()+clock_t(cap*CLOCKS_PER_SEC));
            auto alternative=pair_snapshot_rematch::remake(snapshots[i],stop,
                search_budget::real("PAIR_SNAPSHOT_WEIGHT",2.0));
            if(!alternative)continue;
            auto d=build_design(alternative->B,0,0,alternative->W,&*alternative);
            if(d){candidates.push_back(move(*d));appended_candidates++;}
        }
    }
    cerr<<"PAIR_APPEND enabled="<<append_enabled<<" original_candidates="<<original_candidate_count
        <<" added="<<appended_candidates<<" total="<<candidates.size()<<"\\n";
'''
source = source[:source_start] + original_loop + append_loop + source[source_end:]
(directory / "pair_snapshot_append.cpp").write_text(source, encoding="utf-8")

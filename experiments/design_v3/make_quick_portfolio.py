from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'portfolio.cpp').read_text(encoding='utf-8')
start=s.index('namespace nov5_relaxed_trace {')
end=s.index('} // namespace nov5_relaxed_trace',start)
q=s[start:end]
signature='                                      clock_t deadline) {'
assert q.count(signature)==1
q=q.replace(signature,'                                      clock_t deadline, bool portfolio_quick=false) {')
needle='    auto&log=stats();'
assert q.count(needle)==1
q=q.replace(needle,'''    const int effective_pairs=portfolio_quick?search_budget::integer("PORTFOLIO_PAIRS",12):pair_limit;
    const int effective_triples=portfolio_quick?search_budget::integer("PORTFOLIO_TRIPLES",0):triple_limit;
'''+needle)
q=q.replace('tried<pair_limit','tried<effective_pairs').replace('triples<triple_limit','triples<effective_triples')
q=q.replace('return optimize(router,product,start,std::move(best_moves),deadline);','return optimize(router,product,start,std::move(best_moves),deadline,portfolio_quick);')
s=s[:start]+q+s[end:]
needle='nov5_relaxed_trace::optimize(actual,a,trace_start,alternative.moves,deadline);'
assert s.count(needle)==1
s=s.replace(needle,'nov5_relaxed_trace::optimize(actual,a,trace_start,alternative.moves,deadline,true);')
(p/'quick_portfolio.cpp').write_text(s,encoding='utf-8')

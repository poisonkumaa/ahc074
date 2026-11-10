from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'fast_post_beam.cpp').read_text(encoding='utf-8')
needle='''                if(score<best-1.e-9) {
                    post_beam_selected++;'''
replacement='''                const bool strict_ok=candidate.size()<best_moves.size() &&
                    int(active_supports.size())<=int(best_state.supports.size())+search_budget::integer("POST_BEAM_EXTRA_LOANS",0);
                if(score<best-1.e-9 && strict_ok) {
                    post_beam_selected++;'''
assert needle in s
s=s.replace(needle,replacement,1)
(p/'strict_post.cpp').write_text(s,encoding='utf-8')

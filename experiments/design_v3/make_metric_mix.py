from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'metric_candidate.cpp').read_text(encoding='utf-8')
old='''        const double metric_weight=[](){
            const double value=search_budget::real("PAIR_CORE_WEIGHT",4.0);
            return std::isfinite(value)?std::clamp(value,0.0,32.0):4.0;
        }();'''
assert s.count(old)==1
s=s.replace(old,'        const double metric_weight=pair_core_weight_for_offset(graph_seed_offset);')
helper='''
double pair_core_weight_for_offset(int offset) {
    const unsigned mask=unsigned(search_budget::integer("PAIR_ALT_MASK",0));
    const char* setting=((mask>>(offset&31))&1)?"PAIR_ALT_WEIGHT":"PAIR_CORE_WEIGHT";
    const double value=search_budget::real(setting,setting[5]=='A'?2.0:4.0);
    return std::isfinite(value)?std::clamp(value,.01,32.0):4.0;
}

'''
s=s.replace('int main() {',helper+'int main() {',1)
old='pair_route_cost[p.u*P+p.v]/32.;'
assert s.count(old)==1
s=s.replace(old,'pair_route_cost[p.u*P+p.v]/32.*(4.0/pair_core_weight_for_offset(graph_seed_offset));')
(p/'metric_mix.cpp').write_text(s,encoding='utf-8')

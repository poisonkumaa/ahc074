from pathlib import Path
import subprocess
import sys

directory = Path(__file__).resolve().parent
source = (directory / "main_baseline.cpp").read_text(encoding="utf-8")
source = '''// Apply the target before standard headers, including bitset::count.
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC target("popcnt")
#endif
''' + source


def replace_once(old, new):
    global source
    assert source.count(old) == 1, (old[:100], source.count(old))
    source = source.replace(old, new, 1)


replace_once('''    clock_t core_ticks=0, primary_ticks=0, regular_ticks=0;
    for(int q=0;q<R;q++) {''', '''    clock_t core_ticks=0, primary_ticks=0, regular_ticks=0;
    const bool anchored_choice=search_budget::integer("ROUTE_ANCHORED",0)!=0;
    long long anchored_comparisons=0,anchored_learned=0,anchored_wins=0;
    for(int q=0;q<R;q++) {''')
replace_once('''        auto compare_route=[&](const OnlineRouter::RouteState& challenger,vector<pair<int,int>>& trace,clock_t limit){''', '''        // Every candidate is mapped to one scalar relative to this fixed
        // primary result. This keeps the learned/raw applicability gate and
        // its deadband independent of which earlier challenger happened to win.
        OnlineRouter::RouteState ranking_origin;
        int ranking_length=0;double ranking_value=0,ranking_best=0;
        if(anchored_choice) {
            ranking_origin=actual.save_route_state();ranking_length=int(moves.size());
            ranking_value=actual.route_state_value();
        }
        auto compare_route=[&](const OnlineRouter::RouteState& challenger,vector<pair<int,int>>& trace,clock_t limit){''')
replace_once('''            bool choose=alt.size()+alt_value<first_score-1e-9;
            const int lengths=int(alt.size())-int(moves.size());
            if(abs(lengths)<=1 && abs(alt.size()+alt_value-first_score)<1.15 && other.order!=first_state.order){''', '''            bool choose=alt.size()+alt_value<first_score-1e-9;
            const auto& model_origin=anchored_choice?ranking_origin:first_state;
            const int model_length=anchored_choice?ranking_length:int(moves.size());
            const double model_value=anchored_choice?ranking_value:first_score-moves.size();
            const int lengths=int(alt.size())-model_length;
            const double raw_difference=anchored_choice?lengths+alt_value-model_value:
                alt.size()+alt_value-first_score;
            double candidate_rank=raw_difference;
            anchored_comparisons+=anchored_choice;
            if(abs(lengths)<=1 && abs(raw_difference)<1.15 && other.order!=model_origin.order){
                anchored_learned+=anchored_choice;''')
replace_once('''                auto proxy1=proxy(first_state),proxy2=proxy(other);''', '''                auto proxy1=proxy(model_origin),proxy2=proxy(other);''')
replace_once('''                auto fa=features(first_state,proxy1,first_score-moves.size()),fb=features(other,proxy2,alt_value);''', '''                auto fa=features(model_origin,proxy1,model_value),fb=features(other,proxy2,alt_value);''')
replace_once('''                const double prior=alt_value-(first_score-moves.size());
                const double forecast=lengths+.75*avg+.25*prior;
                if(forecast<-.05)choose=true;
                if(forecast>.05)choose=false;
            }
            if(choose){moves=move(alt);actual.restore_route_state(other);}''', '''                const double prior=alt_value-model_value;
                const double forecast=lengths+.75*avg+.25*prior;
                if(anchored_choice) {
                    // Preserve the old .05 deadband as a primary-relative
                    // scalar rule, rather than an incumbent-dependent test.
                    if(forecast<-.05 || forecast>.05)candidate_rank=forecast;
                }else{
                    if(forecast<-.05)choose=true;
                    if(forecast>.05)choose=false;
                }
            }
            if(anchored_choice) {
                choose=candidate_rank<ranking_best-1e-9;
                if(choose){ranking_best=candidate_rank;anchored_wins++;}
            }
            if(choose){moves=move(alt);actual.restore_route_state(other);}''')
anchor = '    cerr<<"FINAL_DECK saved="<<end_deck_saved<<"\\n";'
replace_once(anchor, r'''    cerr<<"ROUTE_ANCHORED enabled="<<anchored_choice<<" comparisons="<<anchored_comparisons
        <<" learned="<<anchored_learned<<" wins="<<anchored_wins<<"\n";
''' + anchor)
target = directory / "route_anchor.cpp"
target.write_text(source, encoding="utf-8")
fixture = directory / "route_anchor_fixture.cpp"
subprocess.run([sys.executable, str(directory.parent / "beam_v1/make_fixture.py"), str(target), str(fixture)], check=True)
fixture_source = fixture.read_text(encoding="utf-8")
assert fixture_source.count('#include "fixture_io.inc"') == 1
fixture.write_text(fixture_source.replace('#include "fixture_io.inc"', '#include "../beam_v1/fixture_io.inc"'), encoding="utf-8")

from pathlib import Path
root=Path(__file__).resolve().parent
s=(root/'main_baseline.cpp').read_text(encoding='utf-8')
needle='        shelf58::apply(actual,trace_start,moves,a);'
replace=needle+r'''
        // Placement adjustments can turn a paid operation into a no-op.
        // Remove these after the last adjustment; every intermediate deck is unchanged.
        if(search_budget::integer("FINAL_NOOP",1)){
            auto deck=trace_start;size_t write=0;
            for(auto op:moves){
                int from=find(deck.begin(),deck.end(),op.first)-deck.begin();
                if(from==op.second){++final_noop_saved;continue;}
                if(from<op.second)rotate(deck.begin()+from,deck.begin()+from+1,deck.begin()+op.second+1);
                else rotate(deck.begin()+op.second,deck.begin()+from,deck.begin()+from+1);
                moves[write++]=op;
            }
            moves.resize(write);
            assert(deck==actual.sim.order);
        }'''
assert s.count(needle)==1
s=s.replace('int main() {','static long long final_noop_saved=0;\nint main() {',1)
s=s.replace(needle,replace,1)
s=s.replace('    cerr<<"U48 eligible="','    cerr<<"FINAL_NOOP saved="<<final_noop_saved<<"\\n";\n    cerr<<"U48 eligible="',1)
(root/'route_noop.cpp').write_text(s,encoding='utf-8')

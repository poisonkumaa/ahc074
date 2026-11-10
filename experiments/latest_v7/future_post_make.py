"""Compare endpoints after the actual paid-placement finalizers.

The baseline computes its critic before three unconditional shelf edits. This
experiment applies those edits before extracting endpoint features, and repeats
them at the end only if a subsequent tail optimizer changed the chosen trace.
"""
from pathlib import Path
import hashlib
import subprocess
import sys

here = Path(__file__).resolve().parent
source = here / 'main_baseline.cpp'
assert hashlib.sha256(source.read_bytes()).hexdigest() == '407a5355d6a75bbe07bd332a265fcb694b532dc7f35de0dee465006bf136a7e7'
s = source.read_text(encoding='utf-8')

def once(old, new):
    global s
    assert s.count(old) == 1, (old[:80], s.count(old))
    s = s.replace(old, new, 1)

once('int main() {', r'''namespace future_post {
long long calls=0,edits=0;clock_t spent=0;
inline bool enabled(){static const bool on=search_budget::integer("FUTURE_POST",0);return on;}
void normalize(OnlineRouter& r,const vector<int>& start,vector<pair<int,int>>& ops,int a){
    const auto began=clock();++calls;
    edits+=return_pair53::adjust(r,start,ops,a);
    edits+=ordinary54::apply(r,start,ops,a);
    edits+=shelf58::apply(r,start,ops,a);
    spent+=clock()-began;
}
}

int main() {''')

once('''        // Spend secondary-search time only after reserving CPU for the
''', '''        vector<pair<int,int>> normalized_trace;
        if(future_post::enabled()){
            future_post::normalize(actual,trace_start,moves,a);
            normalized_trace=moves;
        }

        // Spend secondary-search time only after reserving CPU for the
''')
once('''            auto alt=nov5_relaxed_trace::optimize(actual,a,trace_start,move(trace),limit);
            const auto other=actual.save_route_state();''', '''            auto alt=nov5_relaxed_trace::optimize(actual,a,trace_start,move(trace),limit);
            if(future_post::enabled())future_post::normalize(actual,trace_start,alt,a);
            const auto other=actual.save_route_state();''')
once('''            if(choose){if(!compared)have_features=false;moves=move(alt);actual.restore_route_state(other);}
''', '''            if(choose){if(!compared)have_features=false;moves=move(alt);actual.restore_route_state(other);
                if(future_post::enabled())normalized_trace=moves;
            }
''')
once('''        return_pair53::adjust(actual,trace_start,moves,a);
        ordinary54::apply(actual,trace_start,moves,a);
        shelf58::apply(actual,trace_start,moves,a);''', '''        if(!future_post::enabled()){
            return_pair53::adjust(actual,trace_start,moves,a);
            ordinary54::apply(actual,trace_start,moves,a);
            shelf58::apply(actual,trace_start,moves,a);
        }else if(moves!=normalized_trace){
            future_post::normalize(actual,trace_start,moves,a);
        }''')
once('''    cerr<<"U48 eligible="''', '''    cerr<<"FUTURE_POST calls="<<future_post::calls<<" edits="<<future_post::edits<<" cpu="<<double(future_post::spent)/CLOCKS_PER_SEC<<"\\n";
    cerr<<"U48 eligible="''')
target = here / 'future_post.cpp'
target.write_text(s, encoding='utf-8')
subprocess.run([sys.executable, str(here.parent/'beam_v1/make_fixture.py'), str(target), str(here/'future_post_fixture.cpp')], check=True)
print(hashlib.sha256(target.read_bytes()).hexdigest())

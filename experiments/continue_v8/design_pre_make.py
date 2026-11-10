"""Price pre-main donors when designing erasures and initial support utility."""
from pathlib import Path
import hashlib

here=Path(__file__).resolve().parent
source=here/'main_baseline.cpp'
assert hashlib.sha256(source.read_bytes()).hexdigest()=='d6810b13fa94eb0bdfb28135d8551340d97648df9b170b49f84d1d668ec3d049'
s=source.read_text(encoding='utf-8')

def once(old,new):
    global s
    assert s.count(old)==1,(old[:100],s.count(old))
    s=s.replace(old,new,1)

once('''static void history_holes(Design& d,const vector<HeadSample>& hist){
    if(hist.empty()||d.C!=0)return;''','''static void history_holes(Design& d,const vector<HeadSample>& hist){
    if(hist.empty()||d.C!=0)return;
    const bool pre_donor=search_budget::integer("DESIGN_PRE_HOLES",1)!=0;''')
once('''        const int w=d.paper_bucket[k];if(!(missing>>w&1))return uint64_t(0);''','''        const int w=d.paper_bucket[k];
        // An earlier bank can lend before the new main is promoted even when
        // that main subsequently covers the donor's own anchor.
        if(!(missing>>w&1) && (!pre_donor || w>=d.paper_bucket[main[a]]))return uint64_t(0);''')
once('''            if(!(missing&b))continue;
            int cut=bad?63-__builtin_clzll(bad):-1;''','''            if(!(missing&b) && (!pre_donor || d.paper_bucket[k]>=d.paper_bucket[main[a]]))continue;
            int cut=bad?63-__builtin_clzll(bad):-1;''')
once(r'''    cerr<<"HEAD_ERASE samples="<<H<<" cuts="<<cuts<<" proxy="<<predicted<<'\n';''',r'''    cerr<<"HEAD_ERASE samples="<<H<<" cuts="<<cuts<<" proxy="<<predicted<<" pre_donor="<<pre_donor<<'\n';''')
once('''static void empirical_utility(OnlineRouter& t,const vector<HeadSample>& hist,bool live=false){
    if(hist.empty()||t.C!=0)return;''','''static void empirical_utility(OnlineRouter& t,const vector<HeadSample>& hist,bool live=false){
    if(hist.empty()||t.C!=0)return;
    const bool initial_pre=search_budget::integer("DESIGN_PRE_UTILITY",1)!=0;''')
once('''            if(!(holes>>w&1) && (!live || w>=t.paper_bucket[main]))continue;''','''            if(!(holes>>w&1) && (!(live || initial_pre) || w>=t.paper_bucket[main]))continue;''')
target=here/'design_pre.cpp'
target.write_text(s,encoding='utf-8')
print(hashlib.sha256(target.read_bytes()).hexdigest())

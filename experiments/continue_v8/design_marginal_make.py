"""Revalidate each hole erasure against the current exact sampled objective."""
from pathlib import Path
import hashlib

here=Path(__file__).resolve().parent
baseline=here/'main_baseline.cpp'
assert hashlib.sha256(baseline.read_bytes()).hexdigest()=='d6810b13fa94eb0bdfb28135d8551340d97648df9b170b49f84d1d668ec3d049'
s=baseline.read_text(encoding='utf-8')

def once(old,new):
    global s
    assert s.count(old)==1,(old[:100],s.count(old))
    s=s.replace(old,new,1)

once('''static void history_holes(Design& d,const vector<HeadSample>& hist){
    if(hist.empty()||d.C!=0)return;''','''static void history_holes(Design& d,const vector<HeadSample>& hist,clock_t absolute_deadline=0){
    if(hist.empty()||d.C!=0)return;
    const bool revalidate=search_budget::integer("DESIGN_MARGINAL",1)!=0;
    const clock_t begun=clock();
    clock_t stop=begun+clock_t(search_budget::seconds(search_budget::real("DESIGN_MARGINAL_CAP",.06))*CLOCKS_PER_SEC);
    if(absolute_deadline>0)stop=min(stop,absolute_deadline);
    bool timed_out=false;int checked=0,rejected=0,rebuilt=0,verified=0;
    const bool verify=search_budget::integer("DESIGN_MARGINAL_VERIFY",0)!=0;
    auto expired=[&](){if(!revalidate)return false;timed_out=clock()>=stop;return timed_out;};''')
once('''    bool used[K]={};int cuts=0;double predicted=0;
    int loops=search_budget::integer("HIST_LOOPS",2);''','''    bool used[K]={};int cuts=0;double predicted=0;
    auto report=[&](){if(revalidate)cerr<<"DESIGN_MARGINAL checked="<<checked<<" rejected="<<rejected
        <<" rebuilt="<<rebuilt<<" verified="<<verified<<" cuts="<<cuts<<" gain="<<predicted<<" timeout="<<timed_out
        <<" cpu="<<double(clock()-begun)/CLOCKS_PER_SEC<<"\\n";};
    if(expired()){report();return;}
    int loops=search_budget::integer("HIST_LOOPS",2);''')
once('''        for(int a=0;a<P;a++)for(int k=B;k<K;k++)if(k!=main[a]){
            uint64_t cov=cover(k,a,ink[k],holes[a]);if(!cov)continue;''','''        for(int a=0;a<P;a++)for(int k=B;k<K;k++)if(k!=main[a]){
            if((k&31)==0 && expired()){report();return;}
            uint64_t cov=cover(k,a,ink[k],holes[a]);if(!cov)continue;''')
once('''        for(int k=B;k<K;k++)if(!used[k])for(int a=0;a<P;a++)if(main[a]!=k){
            uint64_t missing=holes[a],b=1ull<<d.paper_bucket[k],bad=(black[k]^A[a])&ink[k]&missing;''','''        for(int k=B;k<K;k++)if(!used[k])for(int a=0;a<P;a++)if(main[a]!=k){
            if((a&63)==0 && expired()){report();return;}
            uint64_t missing=holes[a],b=1ull<<d.paper_bucket[k],bad=(black[k]^A[a])&ink[k]&missing;''')
once('''        for(auto &c:local){
            uint64_t bit=1ull<<c.j;''','''        for(auto &c:local){
            if(expired()){report();return;}
            uint64_t bit=1ull<<c.j;''')
once('''        int changed=0;
        for(auto c:local)if(!used[c.k]&&c.score>=threshold&&changed*loops<budget&&cuts<budget){
            uint64_t bit=1ull<<c.j;d.S_init[c.k][c.j]='.';ink[c.k]&=~bit;
            for(int a:own[c.k])holes[a]|=bit;
            used[c.k]=true;predicted+=c.score;cuts++;changed++;
        }
        if(!changed)break;
    }
    cerr<<"HEAD_ERASE samples="''','''        int changed=0;bool dirty=false;
        // The original ranker supplies candidates. Before committing one,
        // evaluate its exact marginal effect on the CURRENT sampled objective:
        // own_price * mismatching main holes minus best single-donor saving.
        auto rebuild_top=[&](){
            fill(top.begin(),top.end(),Best{});++rebuilt;
            for(int a=0;a<P;a++){
                if((a&7)==0 && expired())return false;
                for(int k=B;k<K;k++)if(k!=main[a]){
                    uint64_t cov=cover(k,a,ink[k],holes[a]);if(!cov)continue;
                    for(int h=0;h<H;h++){
                        int v=value(a,k,h,cov);auto& b=top[a*H+h];
                        if(v>b.max){b.second=b.max;b.max=v;b.k=k;}
                        else if(v>b.second)b.second=v;
                    }
                }
            }
            return true;
        };
        auto exact_gain=[&](int paper,int column){
            const uint64_t bit=1ULL<<column,new_ink=ink[paper]&~bit;
            double total=0;
            for(int a=0;a<P;a++){
                if((a&15)==0 && expired())return -1e100;
                if(main[a]==paper){
                    // The edited main adds a new repair target. Every other
                    // donor must be rescored against that enlarged hole set.
                    vector<int> best(H);
                    const uint64_t missing=holes[a]|bit;
                    for(int k=B;k<K;k++)if(k!=paper){
                        uint64_t cov=cover(k,a,ink[k],missing);if(!cov)continue;
                        for(int h=0;h<H;h++)best[h]=max(best[h],value(a,k,h,cov));
                    }
                    for(int h=0;h<H;h++)total+=best[h]-top[a*H+h].max
                        -own_price*bool((hist[h].black^A[a])&bit);
                }else{
                    // Only this donor changes; the current second-best entry
                    // gives the exact best value after excluding it.
                    uint64_t cov=cover(paper,a,new_ink,holes[a]);
                    for(int h=0;h<H;h++){
                        const auto& b=top[a*H+h];
                        int other=b.k==paper?b.second:b.max;
                        total+=max(other,value(a,paper,h,cov))-b.max;
                    }
                }
            }
            return total/H;
        };
        auto full_objective=[&](int paper,int column){
            const uint64_t bit=column<0?0:1ULL<<column;double objective=0;
            for(int a=0;a<P;a++){
                const uint64_t missing=holes[a]|(main[a]==paper?bit:0);
                vector<int> best(H);
                for(int k=B;k<K;k++)if(k!=main[a]){
                    uint64_t mask=ink[k]&~(k==paper?bit:0);
                    uint64_t cov=cover(k,a,mask,missing);if(!cov)continue;
                    for(int h=0;h<H;h++)best[h]=max(best[h],value(a,k,h,cov));
                }
                for(int h=0;h<H;h++)objective+=own_price*__builtin_popcountll((hist[h].black^A[a])&missing)-best[h];
            }
            return objective/H;
        };
        for(auto c:local)if(!used[c.k]&&c.score>=threshold&&changed*loops<budget&&cuts<budget){
            if(revalidate){
                if(expired()){report();return;}
                if(dirty && !rebuild_top()){report();return;}
                dirty=false;++checked;c.score=exact_gain(c.k,c.j);
                if(timed_out){report();return;}
                if(verify){
                    double reference=full_objective(-1,-1)-full_objective(c.k,c.j);
                    assert(abs(reference-c.score)<1e-8);++verified;
                    if(expired()){report();return;}
                }
                if(c.score<threshold){++rejected;continue;}
            }
            uint64_t bit=1ull<<c.j;d.S_init[c.k][c.j]='.';ink[c.k]&=~bit;
            for(int a:own[c.k])holes[a]|=bit;
            used[c.k]=true;predicted+=c.score;cuts++;changed++;dirty=true;
        }
        if(!changed)break;
    }
    report();
    cerr<<"HEAD_ERASE samples="''')
once('''        if(search_budget::integer("HIST_REDESIGN",1))history_holes(chosen,heads);''','''        if(search_budget::integer("HIST_REDESIGN",1))history_holes(chosen,heads,
            absolute_deadline(search_budget::real("NOV8_HOLES_END",2.39)));''')
target=here/'design_marginal.cpp'
target.write_text(s,encoding='utf-8')
print(hashlib.sha256(target.read_bytes()).hexdigest())

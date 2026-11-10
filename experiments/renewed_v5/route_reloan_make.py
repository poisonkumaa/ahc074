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


replace_once('''static int wins=0,saved=0,by_type[4]{},savings[4]{},vetos=0;static double cpu=0;
static void improve''', '''static int wins=0,saved=0,by_type[4]{},savings[4]{},vetos=0;static double cpu=0;
static long long reloan_options=0,reloan_wins=0,reloan_trials=0;
static bool reloan_enabled(){static const bool enabled=search_budget::integer("RELOAN_ENABLE",0)!=0;return enabled;}
static bool exact_run_enabled(){static const bool enabled=search_budget::integer("TAIL_EXACT_RUN",0)!=0;return enabled;}
static void improve''')
replace_once('''    double best=ops.size()+r.route_state_value();
    int core_index[3],ncore=0;''', '''    double best=ops.size()+r.route_state_value();
    int best_reloans=0;
    const bool allow_reloan=reloan_enabled();
    const bool exact_run=exact_run_enabled();
    int core_index[3],ncore=0;''')
replace_once('''        array<int,64> run{},first{};first.fill(-1);''', '''        array<int,64> run{},first{},full_run{};first.fill(-1);''')
replace_once('''            if((r.black[k]^r.A[a])>>w&1){int z=++run[w];if(z==2)repeat2|=1ULL<<w;if(z==3)repeat3|=1ULL<<w;}''', '''            if((r.black[k]^r.A[a])>>w&1){int z=++run[w];full_run[w]=z;if(z==2)repeat2|=1ULL<<w;if(z==3)repeat3|=1ULL<<w;}''')
replace_once('''        struct Option{int k,d;uint64_t paint,bad;};''', '''        array<uint64_t,8> run_planes{};int run_bits=0;
        if(exact_run)for(int w=r.C;w<r.T;w++)if(repair>>w&1) {
            // No regular home paper has the desired color. Moving donors
            // earlier cannot add one, so this anchor must be painted over.
            if(run[w]>=0){repair&=~(1ULL<<w);continue;}
            for(int value=full_run[w],b=0;value;value>>=1,b++) {
                if(value&1)run_planes[b]|=1ULL<<w;
                run_bits=max(run_bits,b+1);
            }
        }
        struct Option{int k,d;uint64_t paint,bad;};''')
replace_once('''            int est=1+(j>=0)+__builtin_popcountll(left)+__builtin_popcountll(left&repeat2)+__builtin_popcountll(left&repeat3);''', '''            int est=1+(j>=0);
            if(exact_run)for(int b=0;b<run_bits;b++)est+=(1<<b)*__builtin_popcountll(left&run_planes[b]);
            else est+=__builtin_popcountll(left)+__builtin_popcountll(left&repeat2)+__builtin_popcountll(left&repeat3);''')
replace_once('''            int k=order[d];if(k<r.tot_B || loan[k] || !visible[k] || ((busy>>r.paper_bucket[k])&1))continue;''', '''            int k=order[d];
            if(k<r.tot_B || !visible[k] || r.paper_bucket[k]==r.paper_bucket[main])continue;
            if(loan[k]){if(!allow_reloan)continue;}
            else if((busy>>r.paper_bucket[k])&1)continue;''')
replace_once('''                pool.push_back({k,u,cover,bad});''', '''                pool.push_back({k,u,cover,bad});
                reloan_options+=loan[k];''')
replace_once('''        for(auto trial:top)for(int rev=0;rev<(trial.upper>=0?2:1);rev++){
            vector<int> cur=order;auto path=prefix;bool ok=true;''', '''        for(auto trial:top)for(int rev=0;rev<(trial.upper>=0?2:1);rev++){
            const int reused=int(loan[pool[trial.lower].k])+(trial.upper>=0?int(loan[pool[trial.upper].k]):0);
            reloan_trials+=reused>0;
            vector<int> cur=order;auto path=prefix;bool ok=true;''')
replace_once('''            if(score<best-1e-9){best=score;bestops=move(path);beststate=move(st);}''', '''            if(score<best-1e-9){best=score;bestops=move(path);beststate=move(st);best_reloans=reused;}''')
replace_once('''        ++wins;++by_type[type];saved+=ops.size()-bestops.size();savings[type]+=ops.size()-bestops.size();''', '''        ++wins;++by_type[type];saved+=ops.size()-bestops.size();savings[type]+=ops.size()-bestops.size();
        reloan_wins+=best_reloans>0;''')
anchor = '    cerr<<"FINAL_DECK saved="<<end_deck_saved<<"\\n";'
replace_once(anchor, r'''    cerr<<"RELOAN enabled="<<loan_tail::reloan_enabled()<<" options="<<loan_tail::reloan_options
        <<" trials="<<loan_tail::reloan_trials<<" wins="<<loan_tail::reloan_wins
        <<" exact_run="<<loan_tail::exact_run_enabled()<<"\n";
''' + anchor)
target = directory / "route_reloan.cpp"
target.write_text(source, encoding="utf-8")
fixture = directory / "route_reloan_fixture.cpp"
subprocess.run([sys.executable, str(directory.parent / "beam_v1/make_fixture.py"), str(target), str(fixture)], check=True)
fixture_source = fixture.read_text(encoding="utf-8")
assert fixture_source.count('#include "fixture_io.inc"') == 1
fixture.write_text(fixture_source.replace('#include "fixture_io.inc"', '#include "../beam_v1/fixture_io.inc"'), encoding="utf-8")

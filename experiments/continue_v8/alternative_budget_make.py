"""Spread pair68's global budget over requests and rank promising donor plans.

PAIR_QUERY_CAP is native CPU seconds per eligible request (0 keeps no local
cap). PAIRS_PLAN_SORT=1 stably prioritizes twice the actual removable repair
operations minus newly paid donor moves, then fewer newly paid donor moves.
The global PAIRS_BUDGET and inner PAIRS_TIME_GUARD from the lazy experiment stay.
"""
from pathlib import Path
import hashlib
import subprocess
import sys

here = Path(__file__).resolve().parent
source = here / 'alternative_lazy.cpp'
assert hashlib.sha256(source.read_bytes()).hexdigest() == 'e201a61d0ffe5aef336d7a5258c7909454d64aab53c37d95faf73248682e45d9'
s = source.read_text(encoding='utf-8')
begin = s.index('namespace pair68 {')
end = s.index('namespace headvalue64 {', begin)
part = s[begin:end]

def once(old, new):
    global part
    assert part.count(old) == 1, (old[:100], part.count(old))
    part = part.replace(old, new, 1)

once('''static clock_t distance_ticks=0;
''', '''static clock_t distance_ticks=0;
static long long budget_requests=0,budget_handled=0,budget_capstops=0,budget_sorted=0,budget_plans=0;
''')
once('''    if(!sh)return;
    vector<int> options;
''', '''    if(!sh)return;
    static const clock_t query_cap=clock_t(max(0.,search_budget::real("PAIR_QUERY_CAP",0.))*CLOCKS_PER_SEC);
    static const bool sort_plans=search_budget::integer("PAIRS_PLAN_SORT",0)!=0;
    const clock_t request_began=clock(),absolute_end=end;
    const clock_t request_limit=query_cap>0?request_began+query_cap:absolute_end;
    if(query_cap>0)end=min(end,request_limit);
    ++budget_requests;
    auto local_expired=[&](){return query_cap>0 && clock()>=end;};
    auto early_exit=[&](){
        if(query_cap>0){const clock_t at=clock();spent+=at-request_began;
            budget_capstops+=at>=request_limit && request_limit<=absolute_end;}
    };
    vector<int> options;
''')
once('''    if(options.size()<2)return;
''', '''    if(options.size()<2){early_exit();return;}
''')
once('''    struct Plan{int u,l;uint64_t reset,bad,missing;};
    vector<Plan> p;
    for(int u:options)for(int l:options){
''', '''    struct Plan{int u,l;uint64_t reset,bad,missing;int priority=0,incremental=0;};
    vector<Plan> p;
    unsigned plan_ticks=0;bool plan_timeout=false;
    for(int u:options){
      for(int l:options){
        if((++plan_ticks&15)==0 && local_expired()){plan_timeout=true;break;}
''')
once('''        p.push_back({u,l,ink&ph&~block_reset&~((1ull<<hu)|(1ull<<hl)),bad,sh&~ink});
    }
    if(p.empty())return;clock_t now=clock();++calls;pairs+=p.size();
''', '''        p.push_back({u,l,ink&ph&~block_reset&~((1ull<<hu)|(1ull<<hl)),bad,sh&~ink});
      }
      if(plan_timeout)break;
    }
    if(p.empty() || local_expired()){early_exit();return;}
    clock_t now=query_cap>0?request_began:clock();++calls;pairs+=p.size();
    if(sort_plans){
        ++budget_sorted;
        for(auto& plan:p){
            int reset_moves=0,extra_paid=0;
            // Count operations, not shelf population or distinct paper count:
            // resetting one heavily repaired shelf may remove several moves.
            for(auto [k,d]:ops)if(k>=B && (plan.reset>>r.paper_bucket[k]&1))++reset_moves;
            for(int k:{plan.u,plan.l})if(!paid[k] &&
                (find(old.begin(),old.end(),k)==old.end() ||
                 find(priorcore.begin(),priorcore.begin()+ncore,k)==priorcore.begin()+ncore))++extra_paid;
            plan.priority=2*reset_moves-extra_paid;plan.incremental=extra_paid;
        }
        stable_sort(p.begin(),p.end(),[](const Plan& x,const Plan& y){
            if(x.priority!=y.priority)return x.priority>y.priority;
            return x.incremental<y.incremental;
        });
    }
''')
once('''    for(auto z:p){
        if(stopped || clock()>=end || spent+clock()-now>allowance)break;
''', '''    bool handled=false;
    for(auto z:p){
        if(stopped || clock()>=end || spent+clock()-now>allowance)break;
        if(!handled){handled=true;++budget_handled;}++budget_plans;
''')
once('''    spent+=clock()-now;
''', '''    const clock_t finished=clock();
    budget_capstops+=query_cap>0 && finished>=request_limit && request_limit<=absolute_end;
    spent+=finished-now;
''')
s = s[:begin] + part + s[end:]
needle = '    cerr<<"PAIR_LAZY '
assert s.count(needle) == 1
log = '    cerr<<"PAIR_BUDGET requests="<<pair68::budget_requests<<" calls="<<pair68::calls<<" handled="<<pair68::budget_handled<<" capstops="<<pair68::budget_capstops<<" sorted="<<pair68::budget_sorted<<" plans="<<pair68::budget_plans<<" wins="<<pair68::wins<<"\\n";\n'
s = s.replace(needle, log + needle, 1)
target = here / 'alternative_budget.cpp'
target.write_text(s, encoding='utf-8')
subprocess.run([sys.executable,str(here/'make_fixture.py'),str(target),str(here/'alternative_budget_fixture.cpp')],check=True)
print(hashlib.sha256(target.read_bytes()).hexdigest())

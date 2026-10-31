from pathlib import Path
p=Path(__file__).parent
original=(p/'round4_portfolio_v7.cpp').read_text()
s=original
fast=(p/'policy2_agent_sparse_baseline.cpp').read_text()
fast=fast[fast.index('struct PrefixDP {'):fast.index('\n#include <algorithm>')]
s=s[:s.index('struct PrefixDP {')]+fast+s[s.index('\n#include <algorithm>'):]
s=s.replace('c.dp=make_unique<PrefixDP>(N,c.paper);','c.dp=make_unique<PrefixDP>(N,c.paper);c.dp->public_data(c.target);')
a=s.index(' auto build=[&]');b=s.index(' vector<Candidate>bank',a)
build=s[a:b].replace('c.f1.resize(P);c.f2.resize(P);','c.f1.resize(P);c.f2.resize(P);c.dp->prepare(c.order);').replace('c.dp->solve(c.order,c.target[a],m)','c.dp->solve_prepared(c.target[a],m)')
s=s[:a]+build+s[b:]
direct=(p/'policy4_agent_direct.h').read_text()
direct=direct[direct.index('namespace GlobalPrefix {'):]
direct=direct.replace('std::vector<uint64_t>*future','std::vector<Access>*future').replace('uint64_t below[192]','Access below[192]').replace(':0ULL',':Access(0)').replace('__builtin_popcountll','maskpop')
s=s.replace('struct Candidate {',direct+'\nstruct Candidate {')
# First improving visible paper: every emitted move still lowers exact distance.
assert s.count('        }\n    }\n    return answer;')==1
s=s.replace('        }\n    }\n    return answer;','        }\n        if(answer.first>=0)break;\n    }\n    return answer;')
oldmove='for(int i=0;i<m;i++){auto[from,to]=GlobalPrefix::best_move(c.order,c.length,selected,.5,&c.future);if(from<0)abort();int k=c.order[from];c.order.erase(c.order.begin()+from);c.order.insert(c.order.begin()+to,k);}'
assert s.count(oldmove)==1
s=s.replace(oldmove,'c.order=GlobalPrefix::final_order(c.order,c.length,selected,&c.future);')
configuration='''
#ifndef R4_MAXC
#define R4_MAXC 4
#endif
#ifndef R4_PASSES
#define R4_PASSES 1
#endif
#ifndef R4_PILOT
#define R4_PILOT 544
#endif
#ifndef R4_BUDGET
#define R4_BUDGET 1.15
#endif
#ifndef R4_FINALIST
#define R4_FINALIST 800
#endif
#ifndef R4_ORDER
#define R4_ORDER 0
#endif
'''
s=s.replace('struct Candidate {',configuration+'\nstruct Candidate {')
s=s.replace('constexpr int MAXC=4, PASSES=1, PILOT=544;','constexpr int MAXC=R4_MAXC, PASSES=R4_PASSES, PILOT=R4_PILOT;').replace('constexpr double BUDGET=1.15;','constexpr double BUDGET=R4_BUDGET;').replace('simulate(bank[i],800);','simulate(bank[i],R4_FINALIST);')
s=s.replace(' for(int v=0;v<MAXC;v++){',' const int variant_order[4][8]={{0,1,2,3,4,5,6,7},{0,3,1,2,4,5,6,7},{3,0,1,2,4,5,6,7},{1,3,0,2,4,5,6,7}};\n for(int index=0;index<MAXC;index++){int v=variant_order[R4_ORDER][index];')
(p/'round4_portfolio_fast.cpp').write_text(s)
# Comparisons use the untouched full best_move implementation as their oracle.
a=original.index('inline std::pair<int,int> best_move(');b=original.index('\n}\n}',a)+2
full=original[a:b].replace('best_move(', 'full_move(',1)
check=r'''
vector<int> checked_final(const vector<int>&order,const vector<int>&length,const vector<char>&selected,const vector<Access>*future){
 auto fast=order,full=order,direct=GlobalPrefix::final_order(order,length,selected,future);
 int distance=GlobalPrefix::fixed_distance(order,length,selected);
 for(int it=0;it<distance;it++){
  for(int mode=0;mode<2;mode++){auto &v=mode?full:fast;auto [from,to]=mode?GlobalPrefix::full_move(v,length,selected,.5,future):GlobalPrefix::best_move(v,length,selected,.5,future);if(from<0||to<0||to>=int(v.size()))abort();int high=0;for(int i=0;i<from;i++)high=max(high,length[v[i]]);if(length[v[from]]<=high)abort();int k=v[from];v.erase(v.begin()+from);v.insert(v.begin()+to,k);if(GlobalPrefix::fixed_distance(v,length,selected)!=distance-it-1)abort();}
 }
 if(fast!=full||direct!=full){cerr<<"ACCESS256_FINAL_MISMATCH\n";abort();}return direct;
}
'''
at=s.index('struct Candidate {')
checked=s[:at]+'namespace GlobalPrefix {\n'+full+'\n}\n'+check+s[at:]
checked=checked.replace('c.order=GlobalPrefix::final_order(c.order,c.length,selected,&c.future);','c.order=checked_final(c.order,c.length,selected,&c.future);')
checked=checked.replace('vector<pair<int,int>>ops;ops.reserve(m);','auto expected=checked_final(c.order,c.length,selected,&c.future);vector<pair<int,int>>ops;ops.reserve(m);')
checked=checked.replace('Access bit=Access::bit(a);for(int k=0;k<int(c.paper.size());k++)c.future[k]=(c.future[k]&~bit)|(selected[k]?bit:Access(0));\n  cout','if(c.order!=expected)abort();Access bit=Access::bit(a);for(int k=0;k<int(c.paper.size());k++)c.future[k]=(c.future[k]&~bit)|(selected[k]?bit:Access(0));\n  cout')
(p/'round4_portfolio_check.cpp').write_text(checked)
# A generous-budget original distinguishes architectural behavior from cutoffs.
original=original.replace('constexpr double BUDGET=1.15;','constexpr double BUDGET=100;')
(p/'round4_portfolio_v7_diagnostic.cpp').write_text(original)

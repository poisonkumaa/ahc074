from pathlib import Path
import argparse

p=argparse.ArgumentParser()
p.add_argument('source',type=Path)
p.add_argument('target',type=Path)
p.add_argument('--dump',action='store_true')
a=p.parse_args()
s=a.source.read_text(encoding='utf-8')
start=s.index('int main() {')
tail=s.index('    cerr<<"CHOSEN W="<<chosen.W',start)
prefix=s[:start]+'#include "fixture_io.inc"\n'
if a.dump:
    s=prefix+s[start:tail]+'    save_fixture(chosen,heads,A);\n'+s[tail:]
else:
    fresh=r'''int main() {
    ios::sync_with_stdio(false);cin.tie(nullptr);
    const clock_t design_started=clock();
    auto design_elapsed=[&](){return double(clock()-design_started)/CLOCKS_PER_SEC/search_budget::scale();};
    auto absolute_deadline=[&](double s){return design_started+clock_t(search_budget::seconds(s)*CLOCKS_PER_SEC);};
    int n,k,p,r;if(!(cin>>n>>k>>p>>r))return 0;
    vector<uint64_t> A(P);
    for(auto& value:A){string s;cin>>s;for(int j=0;j<N;j++)if(s[j]=='1')value|=1ULL<<j;}
    Design chosen;vector<HeadSample> heads;load_fixture(chosen,heads,A);
    vector<Design> candidates;int selected=0,synthetic_queries=0;
    cerr<<"FIXED_LAYOUT online_only=1 W="<<chosen.W<<" B="<<chosen.tot_B<<"\n";
'''
    s=prefix+fresh+s[tail:]
a.target.write_text(s,encoding='utf-8')

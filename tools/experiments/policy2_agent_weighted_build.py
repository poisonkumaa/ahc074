from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'global_agent_allocate_tune.cpp').read_text()
s=s.replace('#include "global_agent_allocate_tune.h"',(p/'global_agent_allocate_tune.h').read_text().replace('#pragma once','').replace('std::array<int,17> dp,split','std::array<int,7> dp,split'))
s=s.replace('train_prefix_dictionary(A,papers,n,argc>6?atoi(argv[6]):16,argc>1?atoi(argv[1]):1,argc>2?atoi(argv[2]):8,argc>3?atoi(argv[3]):0,argc>4?atoi(argv[4]):6,argc>5?atoi(argv[5]):0);','train_prefix_dictionary(A,papers,n,64,3,8,0,6,0);')
# Optional paper weights: preserve the exact original potential for calibration.
s=s.replace('double decay=.5) {','double decay=.5, const std::vector<double>* access=nullptr) {')
s=s.replace('double initial_quality=0;', '''std::vector<double> chance(K,decay), survive(K), fenwickLog(N+2);
    if(access)chance=*access;
    for(int i=K-1;i>=0;i--){int k=order[i];double value=0;for(int x=length[k]-(access?0:1);x>0;x-=x&-x)value+=fenwickLog[x];survive[k]=std::exp(value);for(int x=length[k];x<=N;x+=x&-x)fenwickLog[x]+=std::log(chance[k]);}
    double initial_quality=0;''')
s=s.replace('initial_quality+=1-power[inversions[k]];', 'initial_quality+=1-survive[k];')
s=s.replace('int following_min=suffix[i+1];double quality=initial_quality;', 'int following_min=suffix[i+1];double quality=initial_quality, sk=survive[k];')
s=s.replace('int x=order[d],xl=length[x],old_invk=invk;', 'int x=order[d],xl=length[x],old_invk=invk;double old_sk=sk;')
s=s.replace('if(xl<kl){invk--;invsum--;}\n                    else if(xl>kl){quality+=power[inversions[x]]-power[inversions[x]+1];invsum++;}', '''if(xl<kl){invk--;invsum--;}else if(xl>kl)invsum++;
                    if(xl<kl||(access&&xl==kl))sk/=chance[x];
                    if(xl>kl||(access&&xl==kl))quality+=survive[x]*(1-chance[k]);''')
s=s.replace('if(xl<kl){invk++;invsum++;}\n                    else if(xl>kl){quality-=power[inversions[x]-1]-power[inversions[x]];invsum--;}', '''if(xl<kl){invk++;invsum++;}else if(xl>kl)invsum--;
                    if(xl<kl||(access&&xl==kl))sk*=chance[x];
                    if(xl>kl||(access&&xl==kl))quality+=survive[x]*(1-1/chance[k]);''')
s=s.replace('quality+=power[old_invk]-power[invk];','quality+=old_sk-sk;')
start=s.index(' for(int k=0;k<K;k++){\n  string pattern')
end=s.index(' for(int r=0;r<R;r++){',start)
design=s[start:end]
# Run only internally generated public targets, before publishing a design.
setup=''' PrefixDP dp(n,papers);vector<int>order(papers.size()),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);
 vector<double>freq(papers.size()),chance(papers.size());mt19937 freq_rng(912367);double records=0;
 for(int r=0;r<384;r++){
  int target=freq_rng()%P,moves;auto chosen=dp.solve(order,A[target],moves);vector<char>selected(papers.size());for(int k:chosen){selected[k]=true;if(r>=128)freq[k]++;}if(r>=128)records+=chosen.size();
  for(int t=0;t<moves;t++){auto[from,to]=GlobalPrefix::best_move(order,length,selected);if(from<0)abort();int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);}
 }
 double mean=records/papers.size();double scale=argc>1?atof(argv[1]):1.0,base=argc>2?atof(argv[2]):.2;
 for(int k=0;k<int(papers.size());k++)chance[k]=pow(.5,base+scale*freq[k]/mean);
 cerr<<"Records="<<records/256<<" CPU="<<double(clock())/CLOCKS_PER_SEC<<endl;
 iota(order.begin(),order.end(),0);
'''
design=design[:design.index(' PrefixDP dp(n,papers);')]
s=s[:start]+setup+design+s[end:]
s=s.replace('GlobalPrefix::best_move(order,length,selected);if(from<0)return 0;', 'GlobalPrefix::best_move(order,length,selected,.5,argc>3&&atoi(argv[3])?nullptr:&chance);if(from<0)return 0;')
s=s.replace('\n}\n','\n}\n')
# Diagnostics do not enter the reactive protocol.
pos=s.rfind('\n}')
s=s[:pos]+'\n cerr<<"FinishCPU="<<double(clock())/CLOCKS_PER_SEC<<endl;'+s[pos:]
(p/'policy2_agent_weighted.cpp').write_text(s)

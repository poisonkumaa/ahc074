from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'global_agent_weighted.h').read_text()
s=s.replace('int n,int cap,int passes,int importance)', 'int n,int cap,int passes,int importance,int price,int maxpapers=6,int shape=0)')
s=s.replace('const int P=products.size(),K=papers.size(),INF=1000;', '''const int P=products.size(),K=papers.size(),INF=1000;
    std::vector<int> spare;
    bool has[65][2]{};
    for(int k=0;k<K;k++){int l=papers[k].len,b=papers[k].color>>(l-1)&1;if(has[l][b]){papers[k].len=0;papers[k].color=0;spare.push_back(k);}else has[l][b]=true;}''')
s=s.replace('for(int k=0;k<K;k++)level[papers[k].len].push_back(k);','for(int k=0;k<K;k++)if(papers[k].len)level[papers[k].len].push_back(k);')
s=s.replace('std::array<int,5>','std::array<int,17>')
s=s.replace('auto update_start=[&](int k){','''std::vector<int> plan(n+1,2);
    if(shape){std::vector<std::pair<int,int>> rank;for(int l=2;l<=n;l++)for(int q=0;q<maxpapers-2;q++)rank.push_back({l-shape*q,l});std::sort(rank.rbegin(),rank.rend());for(int j=0;j<int(spare.size());j++)plan[rank[j].second]++;}
    auto update_start=[&](int k){''')
s=s.replace('int amount=level[l].size(),depth=std::min(l,cap);','int previous_amount=level[l].size(); int amount=std::min(shape?plan[l]:maxpapers,previous_amount+int(spare.size())),depth=std::min(l,cap);')
s=s.replace('int use=amount;while(use>1&&trie[0].dp[use]<0)use--;','''int use=previous_amount;
        double unit_price=price*std::max(.35,double(l-1)/std::max(1,int(spare.size())));
        for(int q=2;q<=amount;q++)if(trie[0].dp[q]-unit_price*q>trie[0].dp[use]-unit_price*use)use=q;
        if(trie[0].dp[use]<0){use=amount;while(use>1&&trie[0].dp[use]<0)use--;}''')
s=s.replace('if(int(patterns.size())!=amount)continue;','''amount=patterns.size();
        while(int(level[l].size())>amount){int k=level[l].back();level[l].pop_back();papers[k].len=0;papers[k].color=0;spare.push_back(k);}
        while(int(level[l].size())<amount){int k=spare.back();spare.pop_back();level[l].push_back(k);papers[k].len=l;}''')
s=s.replace('std::cerr<<"intrinsic "<<initial<<" -> "<<distances()<<"\\n";','''std::cerr<<"intrinsic "<<initial<<" -> "<<distances()<<" papers="<<K-spare.size()<<"\\n";
    std::vector<PrefixPaper> compact;for(auto p:papers)if(p.len)compact.push_back(p);
    std::stable_sort(compact.begin(),compact.end(),[](auto a,auto b){return a.len<b.len;});papers=compact;''')
(d/'prefix_allocate_train.h').write_text(s)
t=(d/'global_agent_weighted_submit.cpp').read_text().replace('"global_agent_weighted.h"','"prefix_allocate_train.h"')
t=t.replace('argc>2?atoi(argv[2]):8);','argc>2?atoi(argv[2]):8,argc>3?atoi(argv[3]):0,argc>4?atoi(argv[4]):6,argc>5?atoi(argv[5]):0);')
t=t.replace('train_prefix_dictionary(A,papers,n,16,','train_prefix_dictionary(A,papers,n,argc>6?atoi(argv[6]):16,')
(d/'prefix_allocate_submit.cpp').write_text(t)

from pathlib import Path
root=Path(__file__).resolve().parent
s=(root/'main_baseline.cpp').read_text(encoding='utf-8')
a=s.index('namespace fixed_deck {');b=s.index('\n// Rebuild a post-promotion',a)
p=s[a:b]
p=p.replace('using Mask=bitset<K>;','using Mask=bitset<K>;\nstatic long long unique_calls=0,unique_wins=0,unique_saved=0;',1)
old='''    if(!repeated)return;
    Work w(r.opaque,start,r.sim.order,int(moves.size())-1);
    if(w.search({},0))moves=move(w.answer);'''
new='''    static const bool all_routes=search_budget::integer("DECK_LIS",1)!=0;
    if(!repeated && !all_routes)return;
    // Every stationary paper belongs to a common subsequence of the initial
    // and final decks. Its LIS complement is a necessary move count even
    // before visibility constraints. Unique-paper traces can also be shortened
    // by choosing a different set of moved papers.
    if(!repeated){
        array<int,K> rank,tails;int n=0;
        for(int i=0;i<K;i++)rank[r.sim.order[i]]=i;
        for(int k:start){int v=rank[k];auto it=lower_bound(tails.begin(),tails.begin()+n,v);*it=v;if(it==tails.begin()+n)++n;}
        if(K-n>=int(moves.size()))return;
        ++unique_calls;
    }
    int before=moves.size();
    Work w(r.opaque,start,r.sim.order,before-1);
    if(w.search({},0)){
        moves=move(w.answer);
        if(!repeated){++unique_wins;unique_saved+=before-int(moves.size());}
    }'''
assert old in p;p=p.replace(old,new,1)
s=s[:a]+p+s[b:]
s=s.replace('    cerr<<"LOAN_TAIL wins="','    cerr<<"DECK_LIS calls="<<fixed_deck::unique_calls<<" wins="<<fixed_deck::unique_wins<<" saved="<<fixed_deck::unique_saved<<"\\n";\n    cerr<<"LOAN_TAIL wins="',1)
(root/'route_lis.cpp').write_text(s,encoding='utf-8')

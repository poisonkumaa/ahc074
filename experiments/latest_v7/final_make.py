from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'main_baseline.cpp').read_text(encoding='utf-8')
code=r'''
namespace final_compact {
long long calls=0,eligible=0,saved=0,noops=0;double cpu=0;
void optimize(const OnlineRouter& r,const vector<int>& start,vector<pair<int,int>>& moves){
    static const int mode=search_budget::integer("FINAL_COMPACT",1);
    if(!mode)return;
    const clock_t began=clock();++calls;
    auto deck=start;size_t write=0;
    for(auto op:moves){
        int f=find(deck.begin(),deck.end(),op.first)-deck.begin();
        if(f==op.second){++noops;continue;}
        if(f<op.second)rotate(deck.begin()+f,deck.begin()+f+1,deck.begin()+op.second+1);
        else rotate(deck.begin()+op.second,deck.begin()+f,deck.begin()+f+1);
        moves[write++]=op;
    }
    moves.resize(write);assert(deck==r.sim.order);
    // Every untouched paper belongs to a common subsequence of the two decks.
    // Only invoke the visibility-constrained scheduler if this lower bound
    // allows an improvement. The resulting physical deck is unchanged.
    if(mode>=1 && moves.size()>=2 && moves.size()<=22 && cpu<.025){
        array<int,K> ranks{},tails{};int n=0;
        for(int i=0;i<K;i++)ranks[r.sim.order[i]]=i;
        for(int k:start){int v=ranks[k],i=lower_bound(tails.begin(),tails.begin()+n,v)-tails.begin();tails[i]=v;n+=i==n;}
        if(K-n<int(moves.size())){
            ++eligible;fixed_deck::Work w(r.opaque,start,r.sim.order,int(moves.size())-1);
            if(w.search({},0)){saved+=int(moves.size())-int(w.answer.size());moves=move(w.answer);}
        }
    }
    cpu+=double(clock()-began)/CLOCKS_PER_SEC;
}
}
'''
s=s.replace('int main() {',code+'\nint main() {',1)
s=s.replace('        shelf58::apply(actual,trace_start,moves,a);','        shelf58::apply(actual,trace_start,moves,a);\n        final_compact::optimize(actual,trace_start,moves);',1)
s=s.replace('    cerr<<"U48 eligible="','    cerr<<"FINAL_COMPACT calls="<<final_compact::calls<<" eligible="<<final_compact::eligible<<" saved="<<final_compact::saved<<" noops="<<final_compact::noops<<" cpu="<<final_compact::cpu<<"\\n";\n    cerr<<"U48 eligible="',1)
(p/'final.cpp').write_text(s,encoding='utf-8')

// Last-order-only physical search. Terminal shelf structure has no future value.
namespace final_physical_beam {
struct Statistics {long long calls=0,expanded=0,proposals=0,goals=0,saved=0;double cpu=0;};
static Statistics stats;
using Op=pair<int,int>;
using Deck=array<uint8_t,K>;
static uint64_t display(const OnlineRouter& r,const Deck& deck){
    uint64_t seen=0,bits=0;for(int k:deck){bits|=r.black[k]&~seen;seen|=r.opaque[k];if(seen==~0ULL)break;}return bits;
}
static void relocate(Deck& deck,int from,int to){
    uint8_t k=deck[from];
    if(from<to)memmove(deck.data()+from,deck.data()+from+1,to-from);
    else if(to<from)memmove(deck.data()+to+1,deck.data()+to,from-to);
    deck[to]=k;
}
static bool verify(const OnlineRouter& r,const vector<int>& start,const vector<Op>& path,uint64_t target){
    if(path.size()>2*K)return false;
    vector<int> deck=start;
    for(auto [k,d]:path){
        if(k<0 || k>=K || d<0 || d>=K)return false;
        auto it=find(deck.begin(),deck.end(),k);if(it==deck.end())return false;
        uint64_t free=r.opaque[k];for(auto p=deck.begin();p!=it && free;++p)free&=~r.opaque[*p];
        if(!free)return false;
        deck.erase(it);deck.insert(deck.begin()+d,k);
    }
    uint64_t seen=0,bits=0;for(int k:deck){bits|=r.black[k]&~seen;seen|=r.opaque[k];}
    return bits==target;
}
static void apply(const OnlineRouter& r,int product,const vector<int>& start,vector<Op>& moves,clock_t hard_deadline){
    static const bool enabled=search_budget::integer("FINAL_BEAM",1)!=0;
    if(!enabled || moves.empty() || clock()>=hard_deadline)return;
    const clock_t began=clock();++stats.calls;
    struct Charge {clock_t begin;~Charge(){stats.cpu+=double(clock()-begin)/CLOCKS_PER_SEC;}} charge{began};
    const clock_t deadline=min(hard_deadline,began+clock_t(max(0.,search_budget::real("FINAL_BEAM_CPU",.03))*CLOCKS_PER_SEC));
    const int width=clamp(search_budget::integer("FINAL_BEAM_WIDTH",32),1,128);
    const int local_width=8,limit=int(moves.size());
    const uint64_t target=r.A[product];const int main=r.prod_to_paper[product];const uint64_t main_mask=r.opaque[main];
    array<uint64_t,K+1> power;array<uint64_t,K> symbol;power[0]=1;
    for(int i=0;i<K;i++){
        power[i+1]=power[i]*0x9e3779b185ebca87ULL;
        uint64_t x=uint64_t(i+1)*0x9e3779b97f4a7c15ULL;
        x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;x=(x^(x>>27))*0x94d049bb133111ebULL;symbol[i]=x^(x>>31);
    }
    auto hash=[&](const Deck& deck){uint64_t h=0;for(int i=0;i<K;i++)h+=symbol[deck[i]]*power[i];return h;};
    struct State {Deck order;vector<Op> path;uint64_t hash;};
    State initial;for(int i=0;i<K;i++)initial.order[i]=start[i];initial.hash=hash(initial.order);
    auto accept=[&](const vector<Op>& path){
        if(path.size()>=moves.size() || !verify(r,start,path,target))return false;
        ++stats.goals;stats.saved+=moves.size()-path.size();moves=path;return true;
    };
    if(display(r,initial.order)==target){accept(initial.path);return;}
    vector<State> guides;guides.reserve(limit);guides.push_back(initial);
    for(int i=0;i+1<limit;i++){
        if(clock()>=deadline)return;
        State next=guides.back();auto [k,d]=moves[i];
        int from=find(next.order.begin(),next.order.end(),k)-next.order.begin();
        relocate(next.order,from,d);next.path.push_back(moves[i]);next.hash=hash(next.order);
        if(display(r,next.order)==target && accept(next.path))return;
        guides.push_back(move(next));
    }
    struct Choice {double score;uint64_t hash;int parent,paper,depth;};
    vector<State> beam{initial};unordered_map<uint64_t,uint8_t> visited;visited.reserve(width*limit*3);visited[initial.hash]=1;
    unsigned ticks=0;
    for(int depth=0;depth+1<limit && !beam.empty();depth++){
        vector<Choice> choices;choices.reserve(beam.size()*local_width);
        for(int parent=0;parent<int(beam.size());parent++){
            if(clock()>=deadline)return;
            const auto& state=beam[parent];++stats.expanded;
            array<int,64> blockers{};int main_position=0;
            for(int k:state.order){if(k==main)break;++main_position;for(uint64_t bits=r.opaque[k]&main_mask;bits;bits&=bits-1)++blockers[__builtin_ctzll(bits)];}
            int access=K;for(uint64_t bits=main_mask;bits;bits&=bits-1)access=min(access,blockers[__builtin_ctzll(bits)]);
            array<Choice,local_width> local;int count=0;uint64_t above=0;
            for(int from=0;from<K;from++){
                const int k=state.order[from];const uint64_t fresh=r.opaque[k]&~above;above|=r.opaque[k];
                if(!fresh)continue;
                Deck remaining;for(int p=0;p<from;p++)remaining[p]=state.order[p];for(int p=from;p<K-1;p++)remaining[p]=state.order[p+1];
                array<uint64_t,K> prefix_mask{},prefix_black{},suffix_black{},prefix_hash{},suffix_hash{};
                for(int d=0;d<K-1;d++){
                    int q=remaining[d];prefix_mask[d+1]=prefix_mask[d]|r.opaque[q];
                    prefix_black[d+1]=prefix_black[d]|(r.black[q]&~prefix_mask[d]);
                    prefix_hash[d+1]=prefix_hash[d]+symbol[q]*power[d];
                }
                for(int d=K-2;d>=0;d--){int q=remaining[d];suffix_black[d]=r.black[q]|(suffix_black[d+1]&~r.opaque[q]);suffix_hash[d]=suffix_hash[d+1]+symbol[q]*power[d+1];}
                int removed_access=K,added_access=K;
                for(uint64_t bits=main_mask;bits;bits&=bits-1){int b=__builtin_ctzll(bits),v=int(r.opaque[k]>>b&1);removed_access=min(removed_access,blockers[b]-v);added_access=min(added_access,blockers[b]+v);}
                array<int,64> main_above{};int moved_access=0;
                for(int d=0;d<K;d++){
                    if((++ticks&255)==0 && clock()>=deadline)return;
                    if(k==main && d){
                        for(uint64_t bits=r.opaque[remaining[d-1]]&main_mask;bits;bits&=bits-1)++main_above[__builtin_ctzll(bits)];
                        moved_access=K;for(uint64_t bits=main_mask;bits;bits&=bits-1)moved_access=min(moved_access,main_above[__builtin_ctzll(bits)]);
                    }
                    if(d==from)continue;++stats.proposals;
                    const uint64_t shown=prefix_black[d]|(r.black[k]&~prefix_mask[d])|
                        (suffix_black[d]&~(prefix_mask[d]|r.opaque[k]));
                    if(shown==target){auto path=state.path;path.emplace_back(k,d);if(accept(path))return;}
                    const int bad=__builtin_popcountll(shown^target),holes=__builtin_popcountll((shown^target)&~main_mask);
                    int mp,reach;
                    if(k==main){mp=d;reach=moved_access;}
                    else {int mp_removed=main_position-int(from<main_position);mp=mp_removed+int(d<=mp_removed);reach=access;
                        if(from<main_position && d>=main_position)reach=removed_access;
                        else if(from>main_position && d<=main_position)reach=added_access;}
                    const double score=min(4*bad,4*holes+5*reach+3*int(mp>0))+.05*bad+.002*mp;
                    if(count==local_width && score>=local[count-1].score)continue;
                    uint64_t h=prefix_hash[d]+symbol[k]*power[d]+suffix_hash[d];
                    if(visited.find(h)!=visited.end())continue;
                    bool duplicate=false;for(int z=0;z<count;z++)duplicate|=local[z].hash==h;if(duplicate)continue;
                    int at=0;while(at<count && local[at].score<=score)++at;
                    if(count<local_width)++count;
                    for(int z=count-1;z>at;z--)local[z]=local[z-1];local[at]={score,h,parent,k,d};
                }
                if(above==~0ULL)break;
            }
            choices.insert(choices.end(),local.begin(),local.begin()+count);
        }
        stable_sort(choices.begin(),choices.end(),[](const Choice& x,const Choice& y){return x.score<y.score;});
        vector<State> next;next.reserve(width+1);
        for(auto c:choices){
            if(int(next.size())>=width)break;
            if(!visited.emplace(c.hash,1).second)continue;
            State state=beam[c.parent];int from=find(state.order.begin(),state.order.end(),c.paper)-state.order.begin();
            relocate(state.order,from,c.depth);state.path.emplace_back(c.paper,c.depth);state.hash=c.hash;
            next.push_back(move(state));
        }
        // The incumbent's legal prefix gives the search a useful route toward
        // a deeply buried main even when its intermediate display looks worse.
        if(depth+1<int(guides.size()) && visited.emplace(guides[depth+1].hash,1).second)next.push_back(guides[depth+1]);
        beam=move(next);
    }
}
} // namespace final_physical_beam

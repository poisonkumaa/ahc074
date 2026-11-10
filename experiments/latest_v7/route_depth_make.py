from pathlib import Path
root=Path(__file__).resolve().parent
s=(root/'main_baseline.cpp').read_text(encoding='utf-8')
s=s.replace('''        RouteState state;
    };
    vector<RouteCandidate> reservoir;''','''        RouteState state;
        uint64_t group=0;
    };
    vector<RouteCandidate> reservoir;''',1)
needle='''    static uint64_t paper_hash(const vector<pair<int,int>>& ops){
        uint64_t h=0x9e3779b97f4a7c15ULL;
        for(auto m:ops)h=(h^unsigned(m.first+1))*0x100000001b3ULL;
        return h;
    }'''
replace='''    static bool depth_portfolio(){static bool enabled=search_budget::integer("DEPTH_PORTFOLIO",1)!=0;return enabled;}
    static uint64_t paper_group(const vector<pair<int,int>>& ops){
        uint64_t h=0x9e3779b97f4a7c15ULL;
        for(auto m:ops)h=(h^unsigned(m.first+1))*0x100000001b3ULL;
        return h;
    }
    static uint64_t paper_hash(const vector<pair<int,int>>& ops){
        if(!depth_portfolio())return paper_group(ops);
        uint64_t h=0x9e3779b97f4a7c15ULL;
        for(auto m:ops)h=(h^unsigned((m.first+1)*256+m.second))*0x100000001b3ULL;
        return h;
    }'''
assert needle in s;s=s.replace(needle,replace,1)
needle='''                int found=-1,worst=-1;
                for(int i=0;i<int(reservoir.size());i++){
                    if(reservoir[i].key==key){found=i;break;}
                    if(worst<0 || reservoir[worst].rank<reservoir[i].rank)worst=i;
                }
                if(found<0 && reservoir.size()<8){found=int(reservoir.size());reservoir.emplace_back();reservoir.back().rank=1e100;}
                if(found<0)found=worst;
                if(rank<reservoir[found].rank-1.e-9)
                    reservoir[found]={key,rank,score,moves,save_route_state()};'''
replace='''                const bool depths=depth_portfolio();
                const uint64_t group=depths?paper_group(moves):key;
                int found=-1,worst=-1,group_worst=-1,matches=0;
                for(int i=0;i<int(reservoir.size());i++){
                    if(reservoir[i].key==key || (depths && reservoir[i].group==group && reservoir[i].state.order==sim.order)){
                        found=i;break;
                    }
                    if(worst<0 || reservoir[worst].rank<reservoir[i].rank)worst=i;
                    if(depths && reservoir[i].group==group){
                        ++matches;
                        if(group_worst<0 || reservoir[group_worst].rank<reservoir[i].rank)group_worst=i;
                    }
                }
                // Retain at most two distinct terminal decks for a moved-paper
                // sequence; the remaining slots preserve deletion-search diversity.
                if(found<0 && matches>=2)found=group_worst;
                if(found<0 && reservoir.size()<8){found=int(reservoir.size());reservoir.emplace_back();reservoir.back().rank=1e100;}
                if(found<0)found=worst;
                if(rank<reservoir[found].rank-1.e-9)
                    reservoir[found]={key,rank,score,moves,save_route_state(),group};'''
assert needle in s;s=s.replace(needle,replace,1)
(root/'route_depth.cpp').write_text(s,encoding='utf-8')

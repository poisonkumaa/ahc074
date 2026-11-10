from pathlib import Path
import argparse

HERE = Path(__file__).resolve().parent
parser = argparse.ArgumentParser()
parser.add_argument('--source', type=Path, default=HERE / 'main_baseline.cpp')
parser.add_argument('--target', type=Path, default=HERE / 'fast_fixed.cpp')
args = parser.parse_args()
source = args.source.read_text(encoding='utf-8')


def replace_once(old, new):
    global source
    assert source.count(old) == 1, (old[:100], source.count(old))
    source = source.replace(old, new, 1)


replace_once('    map<string,vector<int>> joint_extract_cache;\n', '''    map<string,vector<int>> joint_extract_cache;
    // Opacity never changes after router construction. Build this only when
    // the exact fixed-deck optimizer is first invoked for this router.
    mutable std::unique_ptr<std::array<std::bitset<K>,64>> fixed_deck_column_masks;
''')
replace_once('''namespace fixed_deck {
using Mask=bitset<K>;
struct Work {''', '''namespace fixed_deck {
using Mask=bitset<K>;
using Columns=array<Mask,64>;
inline bool fast_enabled(){static const bool enabled=search_budget::integer("FAST_FIXED_DECK",0)!=0;return enabled;}
inline bool verify_enabled(){static const bool enabled=search_budget::integer("FAST_FIXED_VERIFY",0)!=0;return enabled;}
struct Statistics {long long calls=0,nodes=0,subsets=0,column_builds=0,blocker_builds=0,verified=0;};
inline Statistics& statistics(){static Statistics value;return value;}
struct Work {''')
replace_once('''    array<Mask,64> columns;
    array<uint64_t,1<<12> cover;''', '''    Columns local_columns;
    const Columns* columns;
    bool cache_blockers;
    array<vector<Mask>,K> blocker_masks;
    array<uint8_t,K> blockers_ready{};
    int blocker_builds=0;
    array<uint64_t,1<<12> cover;''')
replace_once('''    Work(const vector<uint64_t>& ink,const vector<int>& start,const vector<int>& goal,int limit):
           ink(ink),start(start),goal(goal),limit(limit){''', '''    Work(const vector<uint64_t>& ink,const vector<int>& start,const vector<int>& goal,int limit,
         const Columns* shared_columns=nullptr,bool cache_blockers=false):
           ink(ink),start(start),goal(goal),columns(shared_columns?shared_columns:&local_columns),
           cache_blockers(cache_blockers),limit(limit){''')
replace_once('''        for(int k=0;k<K;k++)for(uint64_t v=ink[k];v;v&=v-1)columns[__builtin_ctzll(v)].set(k);
''', '''        if(!shared_columns)for(int k=0;k<K;k++)for(uint64_t v=ink[k];v;v&=v-1)local_columns[__builtin_ctzll(v)].set(k);
''')
replace_once('''            for(uint64_t v=ink[k];v;v&=v-1){
                Mask needed=columns[__builtin_ctzll(v)]&above[k]&~take;
                if(needed.none()){exposed=true;break;}
                if(int(needed.count())<=limit-used && find(opts.begin(),opts.end(),needed)==opts.end())opts.push_back(needed);
            }
''', '''            if(cache_blockers && !blockers_ready[k]) {
                auto& masks=blocker_masks[k];masks.reserve(__builtin_popcountll(ink[k]));
                // Keep first-occurrence column order. Equal pre-intersection
                // masks remain equal after removing any selected mover set.
                for(uint64_t v=ink[k];v;v&=v-1) {
                    Mask needed=(*columns)[__builtin_ctzll(v)]&above[k];
                    if(find(masks.begin(),masks.end(),needed)==masks.end())masks.push_back(needed);
                }
                blockers_ready[k]=true;blocker_builds++;
            }
            auto consider=[&](const Mask& blocked) {
                Mask needed=blocked&~take;
                if(needed.none()){exposed=true;return true;}
                if(int(needed.count())<=limit-used && find(opts.begin(),opts.end(),needed)==opts.end())opts.push_back(needed);
                return false;
            };
            if(cache_blockers) {
                for(const Mask& blocked:blocker_masks[k])if(consider(blocked))break;
            }else {
                for(uint64_t v=ink[k];v;v&=v-1)if(consider((*columns)[__builtin_ctzll(v)]&above[k]))break;
            }
''')
replace_once('''    Work w(r.opaque,start,r.sim.order,int(moves.size())-1);
    if(w.search({},0))moves=move(w.answer);
''', '''    const bool fast=fast_enabled();
    auto& log=statistics();log.calls++;
    if(fast && !r.fixed_deck_column_masks) {
        r.fixed_deck_column_masks=make_unique<Columns>();
        for(int k=0;k<K;k++)for(uint64_t v=r.opaque[k];v;v&=v-1)
            (*r.fixed_deck_column_masks)[__builtin_ctzll(v)].set(k);
        log.column_builds++;
    }
    Work w(r.opaque,start,r.sim.order,int(moves.size())-1,
           fast?r.fixed_deck_column_masks.get():nullptr,fast);
    const bool found=w.search({},0);
    log.nodes+=w.nodes;log.subsets+=w.subsets;log.blocker_builds+=w.blocker_builds;
    if(fast && verify_enabled()) {
        Work reference(r.opaque,start,r.sim.order,int(moves.size())-1);
        const bool expected=reference.search({},0);
        if(expected!=found || reference.answer!=w.answer || reference.nodes!=w.nodes || reference.subsets!=w.subsets) {
            cerr<<"FAST_FIXED_MISMATCH expected="<<expected<<" actual="<<found
                <<" reference_nodes="<<reference.nodes<<" actual_nodes="<<w.nodes<<"\\n";
            std::abort();
        }
        log.verified++;
    }
    if(found)moves=move(w.answer);
''')
replace_once('''    cerr<<"FINAL_DECK saved="<<end_deck_saved<<"\\n";
''', '''    cerr<<"FINAL_DECK saved="<<end_deck_saved<<"\\n";
    const auto& fast_fixed=fixed_deck::statistics();
    cerr<<"FAST_FIXED_DECK enabled="<<fixed_deck::fast_enabled()<<" calls="<<fast_fixed.calls
        <<" nodes="<<fast_fixed.nodes<<" subsets="<<fast_fixed.subsets
        <<" column_builds="<<fast_fixed.column_builds<<" blocker_builds="<<fast_fixed.blocker_builds
        <<" verified="<<fast_fixed.verified<<"\\n";
''')
args.target.write_text(source, encoding='utf-8', newline='\n')
print(args.target)

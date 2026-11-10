from pathlib import Path

here = Path(__file__).resolve().parent
s = (here / 'main_baseline.cpp').read_text(encoding='utf-8')
old = '    vector<vector<int>> bucket_members;\n'
new = old + '''    // Exact four-way memoization of a shelf's physical order. The full
    // positions of every member form the key; unrelated deck moves may miss
    // but cannot produce a false hit. Exclusion is applied after lookup.
    struct StructBucketEntry {
        array<uint8_t,32> position{},order{};
        bool valid=false;
    };
    array<array<StructBucketEntry,4>,64> struct_bucket_cache{};
    array<unsigned,64> struct_bucket_next{};
    long long struct_bucket_hits=0,struct_bucket_misses=0;
'''
assert s.count(old) == 1
s=s.replace(old,new)
old = '''        auto get_bucket_papers = [&](int u, int exclude_p = -1) {
            vector<int> res;res.reserve(bucket_members[u].size());
            for(int p:bucket_members[u])if(p!=exclude_p)res.push_back(p);
            sort(res.begin(),res.end(),[&](int x,int y){return sim.depth_of(x)<sim.depth_of(y);});
            return res;
        };
'''
new = '''        auto get_bucket_papers = [&](int u, int exclude_p = -1) {
            static const bool enabled=search_budget::integer("STRUCT_BUCKET_CACHE",1)!=0;
            static const bool verify=search_budget::integer("STRUCT_BUCKET_VERIFY",0)!=0;
            const auto& members=bucket_members[u];
            vector<int> res;res.reserve(members.size());
            if(enabled && route_branching && members.size()<=32){
                array<uint8_t,32> key{};int n=members.size();
                for(int i=0;i<n;i++)key[i]=uint8_t(sim.position[members[i]]);
                StructBucketEntry* found=nullptr;
                for(auto& entry:struct_bucket_cache[u])if(entry.valid &&
                    equal(key.begin(),key.begin()+n,entry.position.begin())){found=&entry;break;}
                if(found){
                    ++struct_bucket_hits;
                    for(int i=0;i<n;i++)if(found->order[i]!=exclude_p)res.push_back(found->order[i]);
                    if(verify){
                        vector<int> reference;
                        for(int p:members)if(p!=exclude_p)reference.push_back(p);
                        sort(reference.begin(),reference.end(),[&](int x,int y){return sim.depth_of(x)<sim.depth_of(y);});
                        assert(res==reference);
                    }
                    return res;
                }
                ++struct_bucket_misses;
                auto& entry=struct_bucket_cache[u][struct_bucket_next[u]++%4];
                vector<int> sorted=members;
                sort(sorted.begin(),sorted.end(),[&](int x,int y){return sim.depth_of(x)<sim.depth_of(y);});
                for(int i=0;i<n;i++)entry.order[i]=uint8_t(sorted[i]);
                entry.position=key;entry.valid=true;
                for(int p:sorted)if(p!=exclude_p)res.push_back(p);
                return res;
            }
            for(int p:members)if(p!=exclude_p)res.push_back(p);
            sort(res.begin(),res.end(),[&](int x,int y){return sim.depth_of(x)<sim.depth_of(y);});
            return res;
        };
'''
assert s.count(old) == 1
s=s.replace(old,new)
old = '    cerr<<"U48 eligible="'
new = '''    cerr<<"STRUCT_BUCKET hits="<<actual.struct_bucket_hits<<" misses="<<actual.struct_bucket_misses<<"\\n";
    cerr<<"U48 eligible="'''
assert s.count(old) == 1
s=s.replace(old,new)
(here / 'struct_bucket.cpp').write_text(s,encoding='utf-8')

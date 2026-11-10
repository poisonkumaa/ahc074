"""Enumerate a shelf in physical order with a 192-position bitset."""
from pathlib import Path
import hashlib

here=Path(__file__).resolve().parent
baseline=here/'main_baseline.cpp'
assert hashlib.sha256(baseline.read_bytes()).hexdigest()=='407a5355d6a75bbe07bd332a265fcb694b532dc7f35de0dee465006bf136a7e7'
s=baseline.read_text(encoding='utf-8')

def once(old,new):
    global s
    assert s.count(old)==1,(old[:100],s.count(old))
    s=s.replace(old,new,1)

once('    vector<vector<int>> bucket_members;\n',
     '    vector<vector<int>> bucket_members;\n    long long linear_bucket_calls=0,linear_bucket_verified=0;\n')
once('''        auto get_bucket_papers = [&](int u, int exclude_p = -1) {
            vector<int> res;res.reserve(bucket_members[u].size());
            for(int p:bucket_members[u])if(p!=exclude_p)res.push_back(p);
            sort(res.begin(),res.end(),[&](int x,int y){return sim.depth_of(x)<sim.depth_of(y);});
            return res;
        };
''','''        auto get_bucket_papers = [&](int u, int exclude_p = -1) {
            static const bool linear=search_budget::integer("LINEAR_BUCKET",1)!=0;
            static const bool verify=search_budget::integer("LINEAR_BUCKET_VERIFY",0)!=0;
            vector<int> res;res.reserve(bucket_members[u].size());
            if(linear){
                ++linear_bucket_calls;
                array<uint64_t,(K+63)/64> occupied{};
                for(int p:bucket_members[u])if(p!=exclude_p){
                    int at=sim.position[p];occupied[at>>6]|=uint64_t(1)<<(at&63);
                }
                for(int word=0;word<int(occupied.size());word++)
                    for(uint64_t bits=occupied[word];bits;bits&=bits-1)
                        res.push_back(sim.order[word*64+__builtin_ctzll(bits)]);
                if(verify){
                    vector<int> ref;for(int p:bucket_members[u])if(p!=exclude_p)ref.push_back(p);
                    sort(ref.begin(),ref.end(),[&](int x,int y){return sim.depth_of(x)<sim.depth_of(y);});
                    assert(res==ref);++linear_bucket_verified;
                }
            }else{
                for(int p:bucket_members[u])if(p!=exclude_p)res.push_back(p);
                sort(res.begin(),res.end(),[&](int x,int y){return sim.depth_of(x)<sim.depth_of(y);});
            }
            return res;
        };
''')
once('    cerr<<"U48 eligible="','''    cerr<<"LINEAR_BUCKET calls="<<actual.linear_bucket_calls<<" verified="<<actual.linear_bucket_verified<<"\\n";
    cerr<<"U48 eligible="''')
target=here/'struct_linear.cpp'
target.write_text(s,encoding='utf-8')
print(hashlib.sha256(target.read_bytes()).hexdigest())

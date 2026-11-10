from pathlib import Path

here = Path(__file__).resolve().parent
s = (here / 'main_baseline.cpp').read_text(encoding='utf-8')

old = '    mutable array<double,64> valued_queue{};\n'
new = old + '''    mutable long long struct_value_hits=0,struct_value_mismatches=0;
    long long struct_value_invalidations=0;
    mutable double struct_value_max_error=0;
    void invalidate_route_value_cache() {
        static const bool enabled=search_budget::integer("STRUCT_VALUE_INVALIDATE",1)!=0;
        ++struct_value_invalidations;
        if(enabled)valued_shape.fill(0);
    }
'''
assert s.count(old) == 1
s = s.replace(old,new)
old = '''                if(shape==valued_shape[w] && equal(b,b+length[w],prior)){
                    cost+=valued_queue[w];continue;
                }
'''
new = '''                const bool was_cached=shape==valued_shape[w] && equal(b,b+length[w],prior);
                static const bool audit=search_budget::integer("STRUCT_VALUE_VERIFY",0)!=0;
                double cached=valued_queue[w];
                if(was_cached){
                    if(!audit){cost+=cached;continue;}
                    ++struct_value_hits;
                }
'''
assert s.count(old) == 1
s = s.replace(old,new)
old = '                valued_queue[w]=route_queue_weight*q/P;cost+=valued_queue[w];\n'
new = '''                const double fresh=route_queue_weight*q/P;
                if(was_cached && audit){
                    double error=abs(fresh-cached);
                    if(error>1e-12){
                        ++struct_value_mismatches;
                        struct_value_max_error=max(struct_value_max_error,error);
                        if(search_budget::integer("STRUCT_VALUE_INVALIDATE",1))abort();
                    }
                    // Auditing does not silently repair the control's old value.
                    valued_queue[w]=cached;
                }else valued_queue[w]=fresh;
                cost+=valued_queue[w];
'''
assert s.count(old) == 1
s = s.replace(old,new)
old = '''        r.joint_extract_cache.clear();
    }
};

struct HeadSample'''
new = '''        r.joint_extract_cache.clear();
        r.invalidate_route_value_cache();
    }
};

struct HeadSample'''
assert s.count(old) == 1
s = s.replace(old,new)
old = '''        n+=counts[k][r];s+=n;
    }}
}

struct BankTemplates'''
new = '''        n+=counts[k][r];s+=n;
    }}
    t.invalidate_route_value_cache();
}

struct BankTemplates'''
assert s.count(old) == 1
s = s.replace(old,new)
old = '    cerr<<"U48 eligible="'
new = '''    cerr<<"STRUCT_VALUE invalidations="<<actual.struct_value_invalidations
        <<" audit_hits="<<actual.struct_value_hits<<" stale="<<actual.struct_value_mismatches
        <<" max_error="<<actual.struct_value_max_error<<"\\n";
    cerr<<"U48 eligible="'''
assert s.count(old) == 1
s = s.replace(old,new)
(here / 'struct_cache.cpp').write_text(s,encoding='utf-8')

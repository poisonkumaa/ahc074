"""Remove an exact redundant CoreGenesis graph-price lookup.

FAST_GENESIS=0 preserves the original formula (default).
FAST_GENESIS=1 uses the remaining Hamming count and the new graph's price.
FAST_GENESIS_VERIFY=1 checks both formulas on every updated live pair.
Pass --no-popcnt to generate a source without an extra target pragma.
"""
from pathlib import Path
import argparse
import hashlib
import subprocess
import sys

parser = argparse.ArgumentParser()
parser.add_argument("--no-popcnt", action="store_true")
parser.add_argument("--output", default="fast_genesis.cpp")
args = parser.parse_args()
here = Path(__file__).resolve().parent
baseline = here / "main_baseline.cpp"
assert hashlib.sha256(baseline.read_bytes()).hexdigest() == "a9e5cb6b45ee1562c60721be09bde2d0246ad8eeb95585c7721048d74143d7da"
source = baseline.read_text(encoding="utf-8")
if not args.no_popcnt:
    source = '''// Enable population count before all standard-library definitions.
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC target("popcnt")
#endif
''' + source


def once(old, new):
    global source
    assert source.count(old) == 1, (old[:100], source.count(old))
    source = source.replace(old, new, 1)


once('''        const auto colbias=catalogue_column_penalty(cat);
        vector<Pair> pairs;''', '''        const auto colbias=catalogue_column_penalty(cat);
        const bool fast_genesis=search_budget::integer("FAST_GENESIS",0)!=0;
        const bool fast_genesis_verify=search_budget::integer("FAST_GENESIS_VERIFY",0)!=0;
        long long fast_genesis_updates=0,fast_genesis_verified=0;
        vector<Pair> pairs;''')
once('''                        uint64_t g=add(q.pred,u[k],v[k]);q.price+=cost(g)-cost(q.pred)-16;q.pred=g;''', '''                        uint64_t g=add(q.pred,u[k],v[k]);
                        if(fast_genesis || fast_genesis_verify){
                            // Inductive invariant: price = 16 * remaining
                            // differing columns + current closure price.
                            // Clearing this differing column removes one unit;
                            // directly price the remaining bits and new graph.
                            const int graph_price=cost(g);
                            const int direct=16*__builtin_popcountll(p.delta&pref)+graph_price;
                            if(fast_genesis_verify){
                                const int reference=q.price+graph_price-cost(q.pred)-16;
                                if(direct!=reference){
                                    std::cerr<<"FAST_GENESIS mismatch B="<<B<<" step="<<step
                                        <<" pair="<<q.id<<" direct="<<direct<<" reference="<<reference<<"\\n";
                                    std::abort();
                                }
                                fast_genesis_verified++;
                                if(!fast_genesis)q.price=reference;
                            }
                            if(fast_genesis){q.price=direct;fast_genesis_updates++;}
                        }else q.price+=cost(g)-cost(q.pred)-16;
                        q.pred=g;''')
once('''        for(int op:best.path){order.push_back(op/32);layout.emplace_back(u[op%32],v[op%32]);}
        return true;''', '''        for(int op:best.path){order.push_back(op/32);layout.emplace_back(u[op%32],v[op%32]);}
        if(fast_genesis || fast_genesis_verify)std::cerr<<"FAST_GENESIS enabled="<<fast_genesis
            <<" B="<<B<<" W="<<W<<" seed="<<seed<<" updates="<<fast_genesis_updates
            <<" verified="<<fast_genesis_verified<<"\\n";
        return true;''')

target = here / args.output
assert target.parent.resolve() == here.resolve(), "Output must remain in v25_v6"
target.write_text(source, encoding="utf-8", newline="\n")
fixture = target.with_name(target.stem + "_fixture.cpp")
subprocess.run([sys.executable, str(here.parent / "beam_v1/make_fixture.py"), str(target), str(fixture)], check=True)
s = fixture.read_text(encoding="utf-8")
assert s.count('#include "fixture_io.inc"') == 1
fixture.write_text(s.replace('#include "fixture_io.inc"', '#include "../beam_v1/fixture_io.inc"'), encoding="utf-8", newline="\n")
print(target.name, hashlib.sha256(target.read_bytes()).hexdigest())

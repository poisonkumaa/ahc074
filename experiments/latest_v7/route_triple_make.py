from pathlib import Path
root=Path(__file__).resolve().parent
s=(root/'main_baseline.cpp').read_text(encoding='utf-8')
a=s.index('namespace loan_tail {');b=s.index('\n}\n\n// Minimum reduced',a)
p=s[a:b]
p=p.replace('static int wins=0,saved=0,by_type[4]{},savings[4]{},vetos=0;static double cpu=0;', '''static int wins=0,saved=0,by_type[4]{},savings[4]{},vetos=0;static double cpu=0;
static long long triple_seeds=0,triple_trials=0,triple_wins=0;static clock_t triple_cpu=0;''')
p=p.replace('    clock_t started=clock();', '''    clock_t started=clock();
    static const bool enable_triple=search_budget::integer("TAIL_TRIPLE",0)!=0;
    static const double triple_budget=search_budget::seconds(search_budget::real("TAIL_TRIPLE_CPU",.04));
    bool best_triple=false;''',1)
p=p.replace('struct Trial{int lower,upper,estimate;};', 'struct Trial{int lower,upper,estimate,highest=-1;};',1)
old='''        for(auto trial:top)for(int rev=0;rev<(trial.upper>=0?2:1);rev++){
            vector<int> cur=order;auto path=prefix;bool ok=true;
            int ids[2]={trial.lower,trial.upper};if(rev)swap(ids[0],ids[1]);'''
new='''        // Extend only a bounded beam of complementary upper pairs. A lower
        // donor may paint the wrong color only where an upper donor covers it.
        if(enable_triple && state.supports.size()<=2 && prefix.size()+3<bestops.size() &&
           double(triple_cpu)/CLOCKS_PER_SEC<triple_budget && clock()<deadline){
            clock_t began=clock();
            struct Pair{int middle,upper,value;uint64_t paint;};
            vector<Pair> beam;
            for(int u=0;u<int(pool.size());u++)if(!pool[u].bad){
                if((u&31)==0 && clock()>=deadline)break;
                for(int m=0;m<int(pool.size());m++){
                    auto& high=pool[u];auto& mid=pool[m];
                    if(high.d>mid.d || (mid.bad&~high.paint) ||
                       r.paper_bucket[high.k]==r.paper_bucket[mid.k] || !(wrong&mid.paint&~high.paint))continue;
                    uint64_t paint=high.paint|mid.paint,left=wrong&~paint;
                    if(!left)continue;
                    int value=2*__builtin_popcountll(wrong&paint)+
                       __builtin_popcountll(repeat2&paint)+__builtin_popcountll(repeat3&paint)
                       -int(!visible[high.k])-int(!visible[mid.k]);
                    if(beam.size()>=12 && value<=beam.back().value)continue;
                    beam.push_back({m,u,value,paint});
                    stable_sort(beam.begin(),beam.end(),[](auto& x,auto& y){return x.value>y.value;});
                    if(beam.size()>12)beam.pop_back();
                }
            }
            triple_seeds+=beam.size();
            for(auto pair:beam)for(int l=0;l<int(pool.size());l++){
                auto& lower=pool[l];auto& mid=pool[pair.middle];auto& high=pool[pair.upper];
                if(lower.d<mid.d || (lower.bad&~pair.paint) ||
                   r.paper_bucket[lower.k]==r.paper_bucket[mid.k] ||
                   r.paper_bucket[lower.k]==r.paper_bucket[high.k] ||
                   !(wrong&lower.paint&~pair.paint))continue;
                uint64_t left=wrong&~(pair.paint|lower.paint);if(left&~repair)continue;
                int est=3+int(!visible[lower.k])+int(!visible[mid.k])+int(!visible[high.k])+
                    __builtin_popcountll(left)+__builtin_popcountll(left&repeat2)+__builtin_popcountll(left&repeat3);
                if(est+int(prefix.size())>=int(bestops.size()) || (top.size()>=6 && est>=top.back().estimate))continue;
                top.push_back({l,pair.middle,est,pair.upper});
                stable_sort(top.begin(),top.end(),[](auto x,auto y){return x.estimate<y.estimate;});
                if(top.size()>6)top.pop_back();
            }
            triple_cpu+=clock()-began;
        }
        for(auto trial:top)for(int rev=0;rev<(trial.highest>=0?6:(trial.upper>=0?2:1));rev++){
            clock_t triple_started=0;
            if(trial.highest>=0){
                if(double(triple_cpu)/CLOCKS_PER_SEC>=triple_budget || clock()>=deadline)break;
                triple_started=clock();++triple_trials;
            }
            struct Charge{clock_t start;~Charge(){if(start)triple_cpu+=clock()-start;}} charge{triple_started};
            vector<int> cur=order;auto path=prefix;bool ok=true;
            const int permutations[6][3]={{0,1,2},{1,0,2},{0,2,1},{2,0,1},{1,2,0},{2,1,0}};
            const int layered[3]={trial.lower,trial.upper,trial.highest};
            int ids[3];for(int z=0;z<3;z++)ids[z]=layered[permutations[rev][z]];
            bool placed[3]={};'''
assert old in p;p=p.replace(old,new,1)
old='''                int anchor=order[e.d];int other=i==trial.upper?trial.lower:trial.upper;
                if(other>=0 && pool[other].d==e.d && i==trial.upper &&
                   ids[0]!=i)anchor=pool[other].k;
                int d=find(cur.begin(),cur.end(),anchor)-cur.begin();'''
new='''                int anchor=order[e.d],layer=0;
                while(layer<3 && layered[layer]!=i)++layer;
                // Among already placed lower donors at this boundary, anchor
                // immediately above the uppermost one.
                for(int z=0;z<layer;z++)if(placed[z] && pool[layered[z]].d==e.d)anchor=pool[layered[z]].k;
                placed[layer]=true;
                int d=find(cur.begin(),cur.end(),anchor)-cur.begin();'''
assert old in p;p=p.replace(old,new,1)
p=p.replace('if(score<best-1e-9){best=score;bestops=move(path);beststate=move(st);}', 'if(score<best-1e-9){best=score;bestops=move(path);beststate=move(st);best_triple=trial.highest>=0;}',1)
p=p.replace('        ++wins;++by_type[type];', '        triple_wins+=best_triple;\n        ++wins;++by_type[type];',1)
s=s[:a]+p+s[b:]
s=s.replace('    cerr<<"LOAN_TAIL wins="', '    cerr<<"TAIL_TRIPLE seeds="<<loan_tail::triple_seeds<<" trials="<<loan_tail::triple_trials<<" wins="<<loan_tail::triple_wins<<" cpu="<<double(loan_tail::triple_cpu)/CLOCKS_PER_SEC<<"\\n";\n    cerr<<"LOAN_TAIL wins="',1)
(root/'route_triple.cpp').write_text(s,encoding='utf-8')

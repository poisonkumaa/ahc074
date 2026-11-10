from pathlib import Path
import hashlib

HERE=Path(__file__).resolve().parent
baseline=HERE/'main_baseline.cpp'
assert hashlib.sha256(baseline.read_bytes()).hexdigest()=='a9e5cb6b45ee1562c60721be09bde2d0246ad8eeb95585c7721048d74143d7da'
s=baseline.read_text(encoding='utf-8')
s='''#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC target("popcnt")
#endif
'''+s

def once(old,new):
    global s
    assert s.count(old)==1,(old[:100],s.count(old))
    s=s.replace(old,new,1)

once('''    auto build_design = [&](int pack_bf,int pack_bl,int pack_br,int pack_start,const PackSnapshot* replay=nullptr) -> optional<Design> {''','''    auto build_design = [&](int pack_bf,int pack_bl,int pack_br,int pack_start,const PackSnapshot* replay=nullptr,
                            bool exact_width=false) -> optional<Design> {''')
once('''        for(int w=pack_start;w>=20&&!packed;w-=2)packed=try_pack(w,pack_bf,pack_bl,pack_br);''','''        for(int w=pack_start;w>=20&&!packed;w-=2) {
            packed=try_pack(w,pack_bf,pack_bl,pack_br);
            if(exact_width)break;
        }''')
once('''            graph_seed_offset=14;
            auto child=build_design(snap.B,0,0,snap.W,&snap);
            if(child)candidates.push_back(move(*child));''','''            // Preserve the original seven candidates. Optionally replace only
            // the crossover child, or append one wider child after it.
            const int wide_mode=std::clamp(search_budget::integer("CHILDMAP_WIDE",0),0,2);
            std::optional<Design> wide_parent;
            std::optional<PackSnapshot> original_child;
            if(wide_mode && first.tot_B==6 && first.W>=38 && first.W<40) {
                // New construction appends to snapshots, and retaining the
                // old child can reallocate candidates. Own both inputs first.
                wide_parent=first;
                if(wide_mode==1)original_child=snap;
            }
            int wide_attempts=0,wide_built=0,ordinary_built=0;
            auto make_wide=[&]() -> optional<Design> {
                if(!wide_parent || design_elapsed()>pack_deadline ||
                   design_elapsed()>layout_deadline-.08)return nullopt;
                const Design* saved_parent=parent_design;
                parent_design=&*wide_parent;graph_seed_offset=15;wide_attempts++;
                // Exact width prevents spending the remaining budget rebuilding
                // an already represented narrower layout when W40 is infeasible.
                auto wider=build_design(6,0,0,40,nullptr,true);
                parent_design=saved_parent;
                if(wider && wider->W!=40)return nullopt;
                wide_built+=bool(wider);
                return wider;
            };
            std::optional<Design> child;
            if(wide_mode==1 && wide_parent) {
                child=make_wide();
                if(!child && design_elapsed()<layout_deadline-.08) {
                    graph_seed_offset=14;
                    child=build_design(original_child->B,0,0,original_child->W,&*original_child);
                    ordinary_built+=bool(child);
                }
            }else {
                graph_seed_offset=14;
                child=build_design(snap.B,0,0,snap.W,&snap);
                ordinary_built+=bool(child);
            }
            if(child)candidates.push_back(move(*child));
            if(wide_mode==2 && wide_parent) {
                auto wider=make_wide();
                if(wider)candidates.push_back(move(*wider));
            }
            if(wide_mode)cerr<<"CHILDMAP_WIDE mode="<<wide_mode<<" parent_w="<<(wide_parent?wide_parent->W:-1)
                <<" attempts="<<wide_attempts<<" built="<<wide_built<<" ordinary="<<ordinary_built
                <<" candidates="<<candidates.size()<<"\\n";''')
(HERE/'design_wide.cpp').write_text(s,encoding='utf-8',newline='\n')
print('Wrote design_wide.cpp')

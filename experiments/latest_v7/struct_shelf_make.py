from pathlib import Path

here=Path(__file__).resolve().parent
s=(here/'main_baseline.cpp').read_text(encoding='utf-8')
old='        ordinary54::apply(actual,trace_start,moves,a);\n'
new='''        bool changed_shelf=ordinary54::apply(actual,trace_start,moves,a);
        static const bool shelf_fixpoint=search_budget::integer("STRUCT_SHELF_FIXPOINT",1)!=0;
        static clock_t shelf_fixpoint_spent=0;
        static const clock_t shelf_fixpoint_cap=clock_t(search_budget::seconds(.06)*CLOCKS_PER_SEC);
        if(shelf_fixpoint && changed_shelf && shelf_fixpoint_spent<shelf_fixpoint_cap && clock()<tail_deadline){
            const clock_t begun=clock();
            // Each accepted placement strictly decreases a finite shelf-order
            // objective, and reconstructs a legal trace of the same length.
            for(int pass=0;pass<7 && changed_shelf;pass++){
                if(clock()>=tail_deadline || clock()-begun+shelf_fixpoint_spent>=shelf_fixpoint_cap)break;
                changed_shelf=ordinary54::apply(actual,trace_start,moves,a);
            }
            shelf_fixpoint_spent+=clock()-begun;
        }
'''
assert s.count(old)==1
s=s.replace(old,new)
(here/'struct_shelf.cpp').write_text(s,encoding='utf-8')

from pathlib import Path
root=Path(__file__).resolve().parent
s=(root/'joint_core_global.inc').read_text(encoding='utf-8')
before='    while(calls<maximum_calls && !expired()){'
insert=r'''
    // Same synthetic orders for every candidate, independent of actual orders.
    const int measured_count=std::max(64,search_budget::integer("CORE_MEASURE_QUERIES",384));
    std::vector<int> measured_sequence;
    uint64_t ms=original.internal_catalog[0]^0xbadbdee328d51ecbULL;if(!ms)ms=1;
    auto mrand=[&](){ms^=ms<<7;ms^=ms>>9;return uint32_t(ms);};
    int last=-1;
    for(int i=0;i<measured_count+32;i++){
        int next;do{next=mrand()%P0;}while(next==last);
        measured_sequence.push_back(next);last=next;
    }
    int measured_calls=0,measured_improvements=0;
    auto measure=[&](const Design& trial){
        if(expired())return 1.e20;
        // Skip optional future/utility precomputation; retain all exact goals.
        OnlineRouter router(trial,trial.internal_catalog,clock());
        router.route_deadline=deadline;
        router.route_branching=search_budget::integer("CORE_MEASURE_BRANCH",0);
        double total=0;
        for(int q=0;q<int(measured_sequence.size());q++){
            if((q&7)==0 && expired())return 1.e20;
            auto moves=router.query(measured_sequence[q]);
            if(router.sim.current_display()!=trial.internal_catalog[measured_sequence[q]])return 1.e20;
            if(q>=32)total+=moves.size();
        }
        measured_calls++;return total/measured_count;
    };
    const double measured_before=measure(original);
    double measured_best=measured_before;
    Design measured_design=original;
    while(calls<maximum_calls && !expired()){
'''
assert before in s
s=s.replace(before,insert,1)
before='        feasible++;\n'
after='''        feasible++;
        if(result.cost<=baseline.cost+P0*search_budget::real("CORE_MEASURE_SLACK",.3)){
            double empirical=measure(candidate);
            if(empirical<measured_best-1.e-9){
                measured_best=empirical;measured_design=candidate;measured_improvements++;
            }
        }
'''
assert s.count(before)==1
s=s.replace(before,after,1)
before='    int changed_columns=0;for(int j:columns)changed_columns+=changed_column(d,j);'
after='''    d=original;
    if(measured_best+search_budget::real("CORE_MEASURE_MIN_GAIN",.03)<measured_before)d=std::move(measured_design);
    std::cerr<<"CORE_MEASURE before="<<measured_before<<" after="<<measured_best
             <<" calls="<<measured_calls<<" improved="<<measured_improvements<<"\\n";
    int changed_columns=0;for(int j:columns)changed_columns+=changed_column(d,j);'''
assert before in s
s=s.replace(before,after,1)
(root/'joint_core_measured.inc').write_text(s,encoding='utf-8')
src=(root/'global.cpp').read_text(encoding='utf-8')
src=src.replace('#include "joint_core_global.inc"','#include "joint_core_measured.inc"',1)
(root/'measured.cpp').write_text(src,encoding='utf-8')

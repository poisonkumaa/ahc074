from pathlib import Path

HERE = Path(__file__).resolve().parent
source = (HERE / "main_before_v4.cpp").read_text(encoding="utf-8")

def replace_once(old, new):
    global source
    assert source.count(old) == 1, (old[:100], source.count(old))
    source = source.replace(old, new, 1)

replace_once("// Grow a comparison map from an empty suffix.",
             '#include "selection_controls.inc"\n\n// Grow a comparison map from an empty suffix.')
replace_once("    vector<vector<array<double,32>>> rank_hist(candidates.size(),vector<array<double,32>>(K));", """    // 0: unchanged; 1: exact shuffled-support controls; 2: exact IID controls;
    // 3: IID + fitted paired controls; 4: shuffled support + fitted controls.
    const int selection_mode=std::clamp(search_budget::integer("SEL4_MODE",0),0,4);
    const bool selection_iid=selection_mode==2 || selection_mode==3;
    const bool selection_fit=selection_mode==3 || selection_mode==4;
    selection_controls::PairedFit selection_model;
    vector<selection_controls::Feature> selection_control(candidates.size());
    vector<double> selection_raw(candidates.size()),selection_raw_total(candidates.size());
    vector<int> selection_count(candidates.size()),selection_support;
    selection_support.reserve(P);
    vector<vector<array<double,32>>> rank_hist(candidates.size(),vector<array<double,32>>(K));""")
replace_once("    // Race layouts on the same orders. Eliminating a layout compares only", """    auto uniform_below=[&](uint32_t bound) {
        const uint32_t threshold=uint32_t(0-bound)%bound;
        uint32_t v;do { v=random32(); } while(v<threshold);
        return v%bound;
    };
    // Race layouts on the same orders. Eliminating a layout compares only""")
replace_once("""        int index=q%P;
        if(index==0) {
            for(int j=P-1;j>0;--j)swap(evaluation_order[j],evaluation_order[random32()%(j+1)]);
            if(evaluation_order[0]==previous)swap(evaluation_order[0],evaluation_order[1+random32()%(P-1)]);
        }
        int a=evaluation_order[index];
        const int last=previous;""", """        int index=q%P,a;
        // Fit uses only earlier observations, before drawing this request.
        if(selection_fit)selection_model.prepare();
        selection_support.clear();
        if(selection_iid) {
            a=int(uniform_below(previous<0?P:P-1));
            if(previous>=0 && a>=previous)++a;
            for(int product=0;product<P;product++)if(product!=previous)
                selection_support.push_back(product);
        }else{
            if(index==0) {
                for(int j=P-1;j>0;--j)swap(evaluation_order[j],evaluation_order[random32()%(j+1)]);
                if(evaluation_order[0]==previous)swap(evaluation_order[0],evaluation_order[1+random32()%(P-1)]);
            }
            a=evaluation_order[index];
            if(selection_mode)for(int j=index;j<P;j++)
                if(index!=0 || evaluation_order[j]!=previous)
                    selection_support.push_back(evaluation_order[j]);
        }
        const int last=previous;""")
replace_once("""            scores[i]+=.9*order_adjustment(*trials[i],a,last);
            auto moves=trials[i]->query(a);
            scores[i]+=moves.size();""", """            if(selection_mode) {
                selection_control[i]=selection_controls::get(*trials[i],a,selection_support);
                scores[i]+=selection_model.correction(selection_control[i]);
            }else scores[i]+=.9*order_adjustment(*trials[i],a,last);
            auto moves=trials[i]->query(a);
            selection_raw[i]=double(moves.size());
            selection_raw_total[i]+=selection_raw[i];selection_count[i]++;
            scores[i]+=moves.size();""")
replace_once("""        for(int i=0;i<int(trials.size());i++)if(feasible[i])
            for(int j=0;j<i;j++)if(feasible[j]){""", """        if(selection_fit)selection_model.observe(selection_control,selection_raw,feasible);
        for(int i=0;i<int(trials.size());i++)if(feasible[i])
            for(int j=0;j<i;j++)if(feasible[j]){""")
replace_once("""    if(!feasible[selected])return fallback();

    Design &chosen=candidates[selected];""", """    if(!feasible[selected])return fallback();
    cerr<<"SELECTION_V4 mode="<<selection_mode<<" queries="<<synthetic_queries<<" selected="<<selected
        <<" beta="<<selection_model.beta[0]<<","<<selection_model.beta[1]<<","<<selection_model.beta[2]<<"\\n";
    for(int i=0;i<int(candidates.size());i++)if(selection_count[i])
        cerr<<"SELECTION_V4_CAND index="<<i<<" n="<<selection_count[i]<<" live="<<feasible[i]
            <<" raw="<<selection_raw_total[i]/selection_count[i]<<" corrected="<<scores[i]/selection_count[i]<<"\\n";

    Design &chosen=candidates[selected];""")
(HERE / "selection_sampling.cpp").write_text(source, encoding="utf-8", newline="\n")
print("Wrote selection_sampling.cpp")

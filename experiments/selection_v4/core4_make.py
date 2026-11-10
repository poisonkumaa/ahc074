from pathlib import Path

directory = Path(__file__).resolve().parent
source = (directory / "main_before_v4.cpp").read_text(encoding="utf-8")


def replace_once(old, new):
    global source
    assert source.count(old) == 1, (old[:80], source.count(old))
    source = source.replace(old, new, 1)


replace_once('if((B!=5 && B!=6 && B!=7) || prod_to_pair_idx[a]<0 || active_supports.size()>=3)return false;',
             'if((B!=4 && B!=5 && B!=6 && B!=7) || prod_to_pair_idx[a]<0 || active_supports.size()>=3)return false;')
replace_once('''            bool ok=tot_B==5?CoreGenesis<5>::generate(original_catalog,column_order,gedges,W,graph_seed_offset,preprocessing_deadline):
                CoreGenesis<6>::generate(original_catalog,column_order,gedges,W,graph_seed_offset,preprocessing_deadline);''', '''            bool ok=tot_B==4?CoreGenesis<4>::generate(original_catalog,column_order,gedges,W,graph_seed_offset,preprocessing_deadline):
                tot_B==5?CoreGenesis<5>::generate(original_catalog,column_order,gedges,W,graph_seed_offset,preprocessing_deadline):
                CoreGenesis<6>::generate(original_catalog,column_order,gedges,W,graph_seed_offset,preprocessing_deadline);''')
replace_once('''        if(tot_B==5 || tot_B==6){
            // Alternating''', '''        if(tot_B==4 || tot_B==5 || tot_B==6){
            // Alternating''')
replace_once('''            if(tot_B==5)refine(std::integral_constant<int,5>{});
            else refine(std::integral_constant<int,6>{});''', '''            if(tot_B==4)refine(std::integral_constant<int,4>{});
            else if(tot_B==5)refine(std::integral_constant<int,5>{});
            else refine(std::integral_constant<int,6>{});''')
replace_once('''    const int width=getenv("PACK_W")?atoi(getenv("PACK_W")):38;

    candidates.reserve(pool);''', '''    const int width=getenv("PACK_W")?atoi(getenv("PACK_W")):38;
    // 0 preserves the original 5/6 pool, 1 tries 4/5, 2 uses only 4.
    const int core4_mode=std::clamp(search_budget::integer("CORE4_MODE",0),0,2);
    const int core4_width=std::clamp(search_budget::integer("CORE4_W",30),20,40);
    cerr<<"CORE4 mode="<<core4_mode<<" width="<<core4_width<<" pool="<<pool<<"\\n";

    candidates.reserve(pool);''')
replace_once('''        const int backing=(offset<2)?5:6;
        if(parent_design && parent_design->tot_B!=backing)parent_design=nullptr;
        int w=(backing==5?36:width)+(parent_design!=nullptr);''', '''        const int backing=core4_mode==2?4:core4_mode==1?(offset<2?4:5):(offset<2?5:6);
        if(parent_design && parent_design->tot_B!=backing)parent_design=nullptr;
        int w=(backing==4?core4_width:backing==5?36:width)+(parent_design!=nullptr);''')
(directory / "core4_candidate.cpp").write_text(source, encoding="utf-8")

from pathlib import Path

root = Path(__file__).resolve().parent
s = (root / 'baseline.cpp').read_text(encoding='utf-8')
specialization = r'''
// Seven backings have too many distinct posets for the full constructor.
// Evaluate reversal-paired rank samples with the existing exact cover table.
template<> struct OrderMetric<7> {
    std::unordered_map<uint64_t,int> memo;
    std::vector<std::array<int,16>> samples;
    OrderMetric():samples(graph_sample_orders(0xf3e2d1c0b9a88776ULL,7,32)) {
        memo.reserve(40000);memo.emplace(0,0);
    }
    int operator()(uint64_t graph) {
        auto it=memo.find(graph);if(it!=memo.end())return it->second;
        uint16_t pred[7];for(int k=0;k<7;k++)pred[k]=(graph>>(8*k))&127;
        int score=lround(64*graph_dag_metric(pred,7,3,samples));
        memo.emplace(graph,score);return score;
    }
};

'''
s = s.replace('template<int B> struct CoreGenesis{', specialization + 'template<int B> struct CoreGenesis{', 1)
s = s.replace('tot_B<=6 && W>tot_B*(tot_B-1)', 'tot_B<=7 && W>tot_B*(tot_B-1)')
s = s.replace('CoreGenesis<6>::generate(original_catalog,column_order,gedges,W,graph_seed_offset,preprocessing_deadline);',
              'tot_B==6?CoreGenesis<6>::generate(original_catalog,column_order,gedges,W,graph_seed_offset,preprocessing_deadline):\n                CoreGenesis<7>::generate(original_catalog,column_order,gedges,W,graph_seed_offset,preprocessing_deadline);', 1)
s = s.replace('const int backing=(offset<2)?5:6;',
              'const int backing=search_budget::integer("STRUCT_BACKING",(offset<2)?5:6);')
s = s.replace('int w=(backing==5?36:width)+(parent_design!=nullptr);',
              'int w=(backing==5?36:backing==7?search_budget::integer("SEVEN_WIDTH",44):width)+(parent_design!=nullptr);')
# B=7 has 21 unordered edges and 42 directed arcs. Keep every witness bit
# and give the column/arc encoding enough low bits for all 42 directions.
s = s.replace('uint16_t covered=0;', 'uint32_t covered=0;')
s = s.replace('child.path.push_back(m.col*32+m.arc);', 'child.path.push_back(m.col*64+m.arc);')
s = s.replace('order.push_back(op/32);layout.emplace_back(u[op%32],v[op%32]);',
              'order.push_back(op/64);layout.emplace_back(u[op%64],v[op%64]);')
(root / 'seven.cpp').write_text(s, encoding='utf-8')

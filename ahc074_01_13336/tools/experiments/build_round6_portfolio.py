from pathlib import Path
d=Path(__file__).resolve().parent
fast=(d/'round6_initial_fast.cpp').read_text()
fast=fast[:fast.index('int main(){')]
fast=fast.replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100').replace('#define AHC_REFINEMENT_BUDGET 1.65','#define AHC_REFINEMENT_BUDGET 100')
old=(d/'round5_portfolio.cpp').read_text();tail=old[old.index('struct ModelSimulation'):]
tail=tail.replace(' for(int id:{12345,32}){',' vector<int>ids={12345,32,1,12346,78989,123};ids.resize(argc>3?atoi(argv[3]):3);for(int id:ids){')
(d/'round6_portfolio.cpp').write_text(fast+tail)

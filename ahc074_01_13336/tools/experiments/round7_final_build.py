from pathlib import Path
p=Path(__file__).resolve().parent
s=(p.parent.parent/'main.cpp').read_text()
s=s.replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100').replace('#define AHC_REFINEMENT_BUDGET 1.65','#define AHC_REFINEMENT_BUDGET 100')
s=s.replace('exact_state_refit(A,papers,1,43,32,-1,0);','exact_state_refit(A,papers,1,43,32,8,0);')
(p/'round7_final_reference.cpp').write_text(s)
s=s.replace('inline std::vector<int> final_order_incremental(', 'inline std::vector<int> final_order_incremental_reference(',1)
at=s.index('inline vector<int> direct_final(')
s=s[:at]+'#include "round7_final_fast.h"\n'+s[at:]
s=s.replace('cerr<<"PathFallbacks="','cerr<<"R7FinalCalls="<<GlobalPrefix::r7_final_calls<<" R7FinalFallbacks="<<GlobalPrefix::r7_final_fallbacks<<" R7FinalInsertions="<<GlobalPrefix::r7_final_inserts<<"\\n";\n cerr<<"PathFallbacks="')
(p/'round7_final_fast.cpp').write_text(s)

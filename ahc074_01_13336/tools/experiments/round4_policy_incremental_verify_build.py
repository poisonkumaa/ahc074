from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy4_agent_direct_reachable.cpp').read_text()
s=s.replace('#undef main','#undef main\n#include "round4_policy_direct_incremental.h"')
s=s.replace('auto a=order,b=direct_final(order,length,selected,&future);','auto a=order,b=GlobalPrefix::final_order_incremental(order,length,selected,&future);if(direct_final(order,length,selected)!=GlobalPrefix::final_order_incremental(order,length,selected)){cerr<<"Uniform mismatch";return 5;}')
(p/'round4_policy_incremental_random.cpp').write_text(s)
s=(p/'round4_policy_direct_incremental.cpp').read_text()
s=s.replace('auto target=GlobalPrefix::final_order_incremental(order,length,selected,&future);vector<pair<int,int>>operations;', '''auto target=GlobalPrefix::final_order_incremental(order,length,selected,&future);
#ifdef VERIFY_DIRECT
  if(target!=direct_final(order,length,selected,&future)){cerr<<"Incremental target mismatch\\n";abort();}
#endif
  vector<pair<int,int>>operations;''')
(p/'round4_policy_direct_incremental.cpp').write_text(s)
s=(p/'round4_policy_direct_validate.py').read_text()
s=s.replace("for name,args,expected in [('round4_policy_direct_moves',[],old_score),('round4_policy_direct_exact',['1','43','12345','4','0','2'],new_score)]:", "for name,args,expected in [('round4_policy_incremental_check',['1','43','12345','4','0','2'],new_score)]:")
s=s.replace('range(100)','range(10)').replace('/100','/10').replace('round4_policy_direct_validation.json','round4_policy_incremental_validation.json')
(p/'round4_policy_incremental_validate.py').write_text(s)

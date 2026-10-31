from pathlib import Path
p=Path(__file__).parent
h=(p/'round6_tailplan.h').read_text()
h=h.replace('#pragma once','#pragma once\n#include "round6_general_future.h"')
pos=h.index(' vector<U64>teacher(){')
method=''' vector<pair<int,int>>execute(vector<int>&order,const Choice&c,const vector<U64>*future){
  vector<pair<int,int>>ops;auto&length=models[c.model].length;int remaining=GlobalPrefix::fixed_distance(order,length,c.selected);
  while(remaining){vector<char>visible(K);U64 covered=0;for(int k:order){visible[k]=(mask[k]&~covered)!=0;covered|=mask[k];}auto[from,to]=GeneralPrefixFuture::best_move(order,length,c.selected,visible,length,.5,future);if(from<0)throw runtime_error("tail actual step missing");int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);ops.push_back({k,to});int next=GlobalPrefix::fixed_distance(order,length,c.selected);if(next>=remaining)throw runtime_error("tail actual no decrease");remaining=next;}
  return ops;
 }
'''
h=h[:pos]+method+h[pos:]
h=h.replace('order=direct_final(order,models[c.model].length,c.selected);','execute(order,c,nullptr);')
start=h.index('  auto target=direct_final(order,length,c.selected,&future);')
end=h.index('  auto verify=old;',start)
h=h[:start]+'  auto old=order;auto operations=planner.execute(order,c,&future);\n'+h[end:]
(p/'round6_tailplan_actual.h').write_text(h)
s=(p/'round6_tailplan.cpp').read_text().replace('#include "round6_tailplan.h"','#include "round6_tailplan_actual.h"')
(p/'round6_tailplan_actual.cpp').write_text(s)

from pathlib import Path
p=Path(__file__).parent
h=(p/'round6_tailplan_actual.h').read_text()
h=h.replace('int moves=1000000,quality=-1,model=-1;vector<char>selected;', 'int moves=1000000,quality=-1,model=-1,actual=1000000;vector<char>selected;bool evaluated=false;vector<int>after;vector<pair<int,int>>ops;')
start=h.index(' Choice choose(');end=h.index(' vector<pair<int,int>>execute(',start)
h=h[:start]+''' Choice choose(const vector<int>&order,int product,const vector<U64>*future,bool evaluate=true){
  Choice result;for(int q=0;q<int(models.size());q++){
   auto&m=models[q];for(int k=0;k<K;k++)m.dp->reward[k]=16+(future?2*__builtin_popcountll((*future)[k]):0);
   int moves;auto goal=m.dp->solve(order,m.target[product],moves);if(moves>=1000000)continue;Choice c;c.moves=moves;c.model=q;c.quality=0;c.selected.assign(K,false);for(int k:goal)c.quality+=m.dp->reward[k],c.selected[k]=true;
   if(evaluate){c.after=order;c.ops=execute(c.after,c,future);c.actual=c.ops.size();c.evaluated=true;}else c.actual=moves;
   if(c.actual<result.actual||(c.actual==result.actual&&c.quality>result.quality))result=move(c);
  }return result;
 }
'''+h[end:]
h=h.replace('  vector<pair<int,int>>ops;auto&length=models[c.model].length;', '  if(c.evaluated){order=c.after;return c.ops;}vector<pair<int,int>>ops;auto&length=models[c.model].length;')
h=h.replace('auto c=planner.choose(order,v,nullptr);', 'auto c=planner.choose(order,v,nullptr,false);')
(p/'round6_tailplan_all.h').write_text(h)
s=(p/'round6_tailplan_actual.cpp').read_text().replace('#include "round6_tailplan_actual.h"','#include "round6_tailplan_all.h"')
(p/'round6_tailplan_all.cpp').write_text(s)

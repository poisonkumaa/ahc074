from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'round5_policy_base.cpp').read_text()
s=s.replace('int main(){','#include "round5_policy_optional.h"\nint main(int argc,char**argv){')
s=s.replace('ios::sync_with_stdio(false);cin.tie(nullptr);','ios::sync_with_stdio(false);cin.tie(nullptr);\n double optional_weight=argc>1?atof(argv[1]):1;int refinement_passes=argc>2?atoi(argv[2]):1;')
s=s.replace('cout.flush();int fallbacks=0;','cout.flush();int fallbacks=0,refinements=0;')
s=s.replace('  if(!direct_moves(order,length,selected,target,operations)', '''  if(refinement_passes){
   if(refinement_passes<0){auto q=history_distance(target,length,future);int expected=0;for(int sample=0;sample<64;sample++){vector<char>ss(papers.size());for(int k=0;k<int(papers.size());k++)ss[k]=future[k]>>sample&1;expected+=GlobalPrefix::fixed_distance(target,length,ss);}if(q.first+q.second!=expected){cerr<<"HistoryDistanceMismatch "<<q.first<<" "<<q.second<<" "<<expected<<"\\n";return 8;}}
   else if(refinement_passes>=10)refinements+=global_refine(order,target,length,selected,future,int(optional_weight),refinement_passes==10?192:refinement_passes-10);
   else refinements+=relocation_refine(order,target,length,selected,future,optional_weight,refinement_passes);
  }
  if(!direct_moves(order,length,selected,target,operations)''')
s=s.replace('cerr<<"PathFallbacks="','cerr<<"Refinements="<<refinements<<"\\n";\n cerr<<"PathFallbacks="')
(p/'round5_policy_optional.cpp').write_text(s)

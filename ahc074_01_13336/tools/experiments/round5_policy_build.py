from pathlib import Path
p=Path(__file__).resolve().parent
s=(p.parent.parent/'main.cpp').read_text()
(p/'round5_policy_base.cpp').write_text(s)
s=s.replace('int main(){','#include "round5_policy_extra.h"\nint main(int argc,char**argv){')
s=s.replace('ios::sync_with_stdio(false);cin.tie(nullptr);','ios::sync_with_stdio(false);cin.tie(nullptr);\n int sample_count=argc>1?atoi(argv[1]):255,horizon=argc>2?atoi(argv[2]):1;double threshold=argc>3?atof(argv[3]):0.;')
s=s.replace('cout.flush();int fallbacks=0;','cout.flush();int fallbacks=0,plus_used=0,plus_possible=0; double total_predicted=0; ExtraGoalDP extra_dp(dp);')
a=s.index('  int moves;auto chosen=dp.solve(order,A[product],moves);')
b=s.index('  if(!direct_moves(',a)
s=s[:a]+'''  int moves; auto goals=extra_dp.solve(order,A[product],moves); auto chosen=goals[0];
  if(moves<0||moves>2*K)return 0;
  vector<char>selected(papers.size());for(int k:chosen)selected[k]=true;
  vector<pair<int,int>>operations;auto before=order;
  auto target=direct_final(order,length,selected,&future);
  if(sample_count>0&&!goals[1].empty()){
   plus_possible++; vector<char>alternate_selected(papers.size());for(int k:goals[1])alternate_selected[k]=true;
   auto alternate=direct_final(order,length,alternate_selected,&future);
   if(GlobalPrefix::fixed_distance(order,length,alternate_selected)!=moves+1){cerr<<"ExtraDistanceMismatch\\n";return 9;}
   vector<int>nexts; int offset=(r*97+product*13)%255;
   for(int i=0;i<min(sample_count,255);i++){int t=(offset+i*127)%255;if(t>=product)t++;nexts.push_back(t);}
   double score[2]{}; vector<int>*ends[2]={&target,&alternate};vector<char>*selections[2]={&selected,&alternate_selected};
   auto original_reward=dp.reward;
   for(int c=0;c<2;c++){
    if(horizon==1){dp.prepare(*ends[c]);for(int t:nexts){int distance;dp.solve_prepared(A[t],distance,false);score[c]+=distance;}}
    else{
     auto nextfuture=future;uint64_t bit=1ULL<<(r%64);for(int k=0;k<int(papers.size());k++){nextfuture[k]=(nextfuture[k]&~bit)|((*selections[c])[k]?bit:0ULL);dp.reward[k]=16+2*__builtin_popcountll(nextfuture[k]);}
     for(int t:nexts){int distance;auto nextgoal=dp.solve(*ends[c],A[t],distance);score[c]+=distance;vector<char>nextselected(papers.size());for(int k:nextgoal)nextselected[k]=true;
      auto nextorder=direct_final(*ends[c],length,nextselected,&nextfuture);int t2=(r*53+t*103+17)%255;if(t2>=t)t2++;
      dp.solve(nextorder,A[t2],distance,false);score[c]+=distance;
     }
    }
   }
   dp.reward=move(original_reward);double gain=(score[0]-score[1])/nexts.size()-1;
   if(gain>threshold){plus_used++;total_predicted+=gain;moves++;chosen=move(goals[1]);selected=move(alternate_selected);target=move(alternate);}
  }
''' + s[b:]
s=s.replace('cerr<<"PathFallbacks="','cerr<<"PlusUsed="<<plus_used<<" PlusPossible="<<plus_possible<<" PredictedGain="<<total_predicted<<"\\n";\n cerr<<"PathFallbacks="')
(p/'round5_policy_extra.cpp').write_text(s)

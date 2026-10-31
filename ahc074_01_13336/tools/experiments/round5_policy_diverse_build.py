from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'round5_policy_base.cpp').read_text()
s=s.replace('int main(){','#include "round5_policy_extra.h"\nint main(int argc,char**argv){')
s=s.replace('ios::sync_with_stdio(false);cin.tie(nullptr);','ios::sync_with_stdio(false);cin.tie(nullptr);\n int samples=argc>1?atoi(argv[1]):255;double threshold=argc>2?atof(argv[2]):0.;')
s=s.replace('cout.flush();int fallbacks=0;','cout.flush();int fallbacks=0,changes=0,extra=0,totalcandidates=0;ExtraGoalDP alternate_dp(dp);')
a=s.index('  int moves;auto chosen=dp.solve(order,A[product],moves);')
b=s.index('  if(!direct_moves(',a)
s=s[:a]+'''  int moves;auto chosen=dp.solve(order,A[product],moves);if(moves<0||moves>2*K)return 0;
  struct Candidate{int moves;vector<int>chosen,order;vector<char>selected;};vector<Candidate>candidates;
  auto add=[&](const vector<int>&goal,int count){
   if(goal.empty())return;for(auto&c:candidates)if(c.chosen==goal)return;
   vector<char>selected(papers.size());for(int k:goal)selected[k]=1;
   candidates.push_back({count,goal,direct_final(order,length,selected,&future),move(selected)});
  };
  add(chosen,moves);auto oldreward=dp.reward;
  for(int variant=0;variant<4;variant++){
   for(int k=0;k<int(papers.size());k++){int freq=__builtin_popcountll(future[k]);dp.reward[k]=variant==0?16+2*freq:variant==1?-16-2*freq:variant==2?-1024+2*freq:1024+2*freq;}
   int count;auto goals=alternate_dp.solve(order,A[product],count);if(count!=moves)return 8;add(goals[0],count);add(goals[1],count+1);
  }
  dp.reward=move(oldreward);totalcandidates+=candidates.size();
  int best=0;double bestcost=1e100;
  vector<int>nexts;int offset=(r*97+product*13)%255;for(int i=0;i<min(samples,255);i++){int t=(offset+i*127)%255;if(t>=product)t++;nexts.push_back(t);}
  for(int c=0;c<int(candidates.size());c++){
   double cost=candidates[c].moves;dp.prepare(candidates[c].order);int total=0;for(int t:nexts){int distance;dp.solve_prepared(A[t],distance,false);total+=distance;}cost+=double(total)/nexts.size();
   if(cost<bestcost-(c?threshold:0)){best=c;bestcost=cost;}
  }
  if(best){changes++;extra+=candidates[best].moves-moves;}moves=candidates[best].moves;chosen=move(candidates[best].chosen);auto selected=move(candidates[best].selected);auto target=move(candidates[best].order);
  vector<pair<int,int>>operations;auto before=order;
''' + s[b:]
s=s.replace('cerr<<"PathFallbacks="','cerr<<"Changes="<<changes<<" Extra="<<extra<<" Candidates="<<totalcandidates<<"\\n";\n cerr<<"PathFallbacks="')
(p/'round5_policy_diverse.cpp').write_text(s)

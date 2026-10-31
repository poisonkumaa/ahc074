from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy4_agent_base.cpp').read_text()
# This snapshot is a single fixed design, avoiding portfolio timer noise.
a=s.index('int main(');s=s[:a]+(p/'policy4_agent_direct.h').read_text().replace('#pragma once','')+'\n'+(p/'round4_policy_direct_moves.h').read_text().replace('#pragma once','')+'\n'+s[a:]
s=s.replace(' for(int r=0;r<R;r++){',' int failures=0;clock_t path_ticks=0;\n for(int r=0;r<R;r++){')
a=s.index('  vector<pair<int,int>>operations;operations.reserve(moves);');b=s.index('  {int period=1;',a)
s=s[:a]+'''  auto path_start=clock();auto target=GlobalPrefix::final_order(order,length,selected,&future);vector<pair<int,int>>operations;
  if(!direct_moves(order,length,selected,target,operations)){
   if(!failures)cerr<<"DIRECT PATH FAILURE r="<<r<<"\\n";failures++;
   for(int i=0;i<moves;i++){auto[from,to]=GlobalPrefix::best_move(order,length,selected,.5,&future);if(from<0)return 0;int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);operations.push_back({k,to});}
  }
  if(int(operations.size())!=moves||order!=target){cerr<<"Path mismatch\\n";abort();}path_ticks+=clock()-path_start;
''' +s[b:]
s=s.replace(' cerr<<"FinishCPU="',' cerr<<"DirectFailures="<<failures<<" PathCPU="<<double(path_ticks)/CLOCKS_PER_SEC<<"\\n";\n cerr<<"FinishCPU="')
(p/'round4_policy_direct_moves.cpp').write_text(s)

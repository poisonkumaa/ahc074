from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy3_agent_base.cpp').read_text().replace('int main(){','int main(int argc,char**argv){')
t=(p/'policy2_agent_goal_sparse.cpp').read_text();a=s.index('struct PrefixDP {');b=s.index('\n};',a)+3;c=t.index('struct PrefixDP {');d=t.index('\n};',c)+3
s=s[:a]+t[c:d]+s[b:]
s=s.replace(' PrefixDP dp(n,papers);vector<int>order',' PrefixDP dp(n,papers);dp.public_data(A);vector<int>order')
s=s.replace(' for(int r=0;r<R;r++){',' mt19937 predict_rng(715113);int changed=0,forecasted=0;\n for(int r=0;r<R;r++){')
start=s.index('  for(int k=0;k<int(papers.size());k++)dp.reward[k]=16+2*')
end=s.index('  {int period=1;',start)
s=s[:start]+'''  struct Plan{vector<int>order;vector<pair<int,int>>ops;vector<char>selected;};
  auto plan=[&](const vector<uint64_t>&model){
   for(int k=0;k<int(papers.size());k++)dp.reward[k]=16+2*__builtin_popcountll(model[k]);
   int moves;auto chosen=dp.solve(order,A[product],moves);if(moves<0||moves>2*K)exit(2);Plan out;out.order=order;out.selected.assign(papers.size(),false);for(int k:chosen)out.selected[k]=true;out.ops.reserve(moves);
   for(int i=0;i<moves;i++){auto[from,to]=GlobalPrefix::best_move(out.order,length,out.selected,.5,&model);if(from<0)exit(3);int k=out.order[from];out.order.erase(out.order.begin()+from);out.order.insert(out.order.begin()+to,k);out.ops.push_back({k,to});}return out;
  };
  auto baseline=plan(future);int samples=argc>1?atoi(argv[1]):32;
  if(samples&&double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<1.35){
   forecasted++;vector<uint64_t>predicted(papers.size());bool used[256]{};used[product]=true;dp.prepare(baseline.order);
   for(int f=0;f<samples;f++){int target;do{target=predict_rng()%P;}while(used[target]);used[target]=true;int distance;auto chosen=dp.solve_prepared(A[target],distance);for(int k:chosen)predicted[k]|=1ULL<<f;}
   uint64_t retain=samples==64?0ULL:~0ULL<<samples;for(int k=0;k<int(papers.size());k++)predicted[k]|=future[k]&retain;
   auto candidate=plan(predicted);changed+=candidate.order!=baseline.order;baseline=move(candidate);
  }
  order=move(baseline.order);auto operations=move(baseline.ops);auto selected=move(baseline.selected);
''' +s[end:]
s=s.replace(' cerr<<"FinishCPU="',' cerr<<"Forecasted="<<forecasted<<" Changed="<<changed<<"\\n";\n cerr<<"FinishCPU="')
(p/'policy3_agent_conditional.cpp').write_text(s)
s=(p/'policy2_agent_goal_ensemble_grid.py').read_text()
s=s.replace("*map(str,args)","str(args)").replace('policy2_agent_goal_ensemble','policy3_agent_conditional')
start=s.index('for args in [');end=s.index(':\n with concurrent',start)
s=s[:start]+'for args in [0,16,32,64]'+s[end:]
(p/'policy3_agent_conditional_grid.py').write_text(s)

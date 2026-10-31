from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy2_agent_goal_sparse.cpp').read_text().replace(' for(int r=0;r<R;r++){',' long long evalcalls=0,totalvariants=0;\n for(int r=0;r<R;r++){').replace('seen.push_back(chosen);','seen.push_back(chosen);totalvariants++;').replace('dp.solve_prepared(A[target],distance,false);total+=distance;','dp.solve_prepared(A[target],distance,false);total+=distance;evalcalls++;').replace(' cerr<<"FinishCPU="',' cerr<<"EvalCalls="<<evalcalls<<" Variants="<<totalvariants<<"\\n";\n cerr<<"FinishCPU="')
(p/'policy2_agent_goal_stats.cpp').write_text(s)

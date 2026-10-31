from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'round4_exact_iter.cpp').read_text().replace(' for(int r=0;r<R;r++){',' clock_t path_ticks=0;\n for(int r=0;r<R;r++){')
s=s.replace('  vector<pair<int,int>>operations;operations.reserve(moves);','  auto path_start=clock();\n  vector<pair<int,int>>operations;operations.reserve(moves);')
s=s.replace('  {int period=1;','  path_ticks+=clock()-path_start;\n  {int period=1;')
s=s.replace(' cerr<<"FinishCPU="',' cerr<<"PathCPU="<<double(path_ticks)/CLOCKS_PER_SEC<<"\\n";\n cerr<<"FinishCPU="')
(p/'round4_policy_exact_pathbase.cpp').write_text(s)

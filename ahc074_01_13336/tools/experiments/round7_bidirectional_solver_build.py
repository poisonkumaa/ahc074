from pathlib import Path
root=Path(__file__).resolve().parents[2]
text=(root/'tools/experiments/round7_bidirectional.cpp').read_text()
text=text.replace('#include "round7_bidirectional.h"','#include "round7_bidirectional.h"\n#include "round7_bidirectional_plan.h"',1)
old=" cout<<setprecision(12)<<bidirectional_average(A,papers)<<'\\n';for(auto p:papers)cout<<p.len<<' '<<p.right<<' '<<p.color<<'\\n';"
new=r'''
 double access=argc>4?atof(argv[4]):0.;bool merge=argc>5?atoi(argv[5]):1;int dedicated=argc>6?atoi(argv[6]):0;
 BidirectionalPlanner planner(A,papers);cerr<<"PrepCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
 for(int k=0;k<K;k++){string s(N,'.');for(int j=0;j<N;j++)if(planner.masks[k]>>j&1)s[model.cols[j]]=char('0'+(((papers[k].color>>j)&1)^model.flip[j]));cout<<s<<'\n';}cout.flush();
 long long total=0;
 for(int r=0;r<R;r++){int v;if(!(cin>>v)||v<0||v>=P)return 0;auto ops=planner.solve(v,r,access,merge,dedicated);total+=ops.size();cout<<ops.size()<<'\n';for(auto[k,d]:ops)cout<<k<<' '<<d<<'\n';cout.flush();}
 cerr<<"Total="<<total<<" FinishCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
'''
assert old in text;text=text.replace(old,new,1)
(root/'tools/experiments/round7_bidirectional_solver.cpp').write_text(text)

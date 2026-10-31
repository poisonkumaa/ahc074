from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'round4_policy_rollout.cpp').read_text()
a=s.index('struct GoalChoice{');b=s.index('int main(',a)
s=s[:a]+(p/'round4_policy_potential.h').read_text()+'\n'+s[b:]
s=s.replace(' mt19937 rollout_rng(183746);int mode=argc>1?atoi(argv[1]):0,sample_count=argc>2?atoi(argv[2]):32,horizon=argc>3?atoi(argv[3]):2;double threshold=argc>4?atof(argv[4]):.05;', ' PotentialPlanner potential(*c.dp);int weight=argc>1?atoi(argv[1]):65536;')
s=s.replace('auto proposal=choose_rollout(*c.dp,c.order,c.length,c.target,a,c.future,mode,sample_count,horizon,threshold,rollout_rng);int m=proposal.moves;if(m<0||m>2*K)return 0;vector<char>selected(c.paper.size());for(int k:proposal.papers)selected[k]=true;', 'for(int k=0;k<int(c.paper.size());k++)c.dp->reward[k]=16+2*maskpop(c.future[k]);int m;auto chosen=weight?potential.solve(c.order,c.target[a],c.future,weight,m):c.dp->solve(c.order,c.target[a],m);if(m<0||m>2*K)return 0;vector<char>selected(c.paper.size());for(int k:chosen)selected[k]=true;')
pos=s.rfind('  Access bit=Access::bit(a);')
s=s[:pos]+'  if(weight){int actual=total_potential(c.order,c.length,c.future);if(actual!=potential.last_phi){cerr<<"POTENTIAL MISMATCH r="<<r<<" expected="<<potential.last_phi<<" actual="<<actual<<"\\n";abort();}}\n'+s[pos:]
s=s.replace(' cerr<<"Lookahead="<<lookahead_queries<<" Candidates="<<evaluated_goals<<" Changed="<<changed_goals<<" Extra="<<extra_moves<<"\\n";\n','')
(p/'round4_policy_potential.cpp').write_text(s)

from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'round4_policy_rollout.cpp').read_text()
a=s.index('struct GoalChoice{');b=s.index('int main(',a);s=s[:a]+s[b:]
s=s.replace('int n,k,epoch=0,primary=65536;', 'int n,k,epoch=0,primary=65536,compat_weight=0;vector<array<int,65>>compatible;')
s=s.replace('opcount(n+1),reward(k,16){}','opcount(n+1),reward(k,16){compatible.resize(k);}')
s=s.replace('for(int j=start;j<a[z].len;j++)candidate[v][j].push_back(z);','for(int j=start;j<a[z].len;j++){candidate[v][j].push_back(z);compatible[z][j]++;}')
s=s.replace('add*primary-reward[z]', 'add*primary-reward[z]-compat_weight*compatible[z][j]')
s=s.replace(' mt19937 rollout_rng(183746);int mode=argc>1?atoi(argv[1]):0,sample_count=argc>2?atoi(argv[2]):32,horizon=argc>3?atoi(argv[3]):2;double threshold=argc>4?atof(argv[4]):.05;', ' c.dp->primary=1000000;c.dp->compat_weight=argc>1?atoi(argv[1]):1;')
s=s.replace('auto proposal=choose_rollout(*c.dp,c.order,c.length,c.target,a,c.future,mode,sample_count,horizon,threshold,rollout_rng);int m=proposal.moves;if(m<0||m>2*K)return 0;vector<char>selected(c.paper.size());for(int k:proposal.papers)selected[k]=true;', 'for(int k=0;k<int(c.paper.size());k++)c.dp->reward[k]=16+2*maskpop(c.future[k]);int m;auto chosen=c.dp->solve(c.order,c.target[a],m);if(m<0||m>2*K)return 0;vector<char>selected(c.paper.size());for(int k:chosen)selected[k]=true;')
s=s.replace(' cerr<<"Lookahead="<<lookahead_queries<<" Candidates="<<evaluated_goals<<" Changed="<<changed_goals<<" Extra="<<extra_moves<<"\\n";\n','')
(p/'round4_policy_compat.cpp').write_text(s)
s=(p/'round4_policy_potential_grid.py').read_text().replace('round4_policy_potential','round4_policy_compat').replace('[0,1,4,16,65536]','[0,1,4]')
(p/'round4_policy_compat_grid.py').write_text(s)

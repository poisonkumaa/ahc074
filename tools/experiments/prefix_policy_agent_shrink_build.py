from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'prefix_policy_agent_weighted.cpp').read_text()
fn=r'''
double ideal_records(const vector<U64>&targets,const vector<PrefixPaper>&paper){
 int total=0;for(U64 target:targets){int cost[65];fill(cost,cost+65,1000);cost[0]=0;
  for(auto a:paper){U64 d=(a.color^target)&PrefixDP::low(a.len);int start=d?64-__builtin_clzll(d):0;for(int j=start;j<a.len;j++)cost[a.len]=min(cost[a.len],cost[j]+1);}
  total+=cost[64];
 }return double(total)/targets.size();
}
'''
s=s.replace('struct Candidate{',fn+'\nstruct Candidate{\n double intrinsic=0; double estimate()const{return double(score)/max(1,count-16)+SHRINK*intrinsic;}')
s=s.replace('train_prefix_dictionary(c.target,c.paper,64,16,1,8);','train_prefix_dictionary(c.target,c.paper,64,16,1,8);c.intrinsic=ideal_records(c.target,c.paper);')
s=s.replace('return a.score<b.score;', 'return a.estimate()<b.estimate();')
s=s.replace('candidates[i].score<candidates[best].score', 'candidates[i].estimate()<candidates[best].estimate()')
(p/'prefix_policy_agent_shrink.cpp').write_text('#define SHRINK 1.4\n'+s)
si=s.replace('simulate(c,80);','').replace('int finalists=min(3,int(candidates.size()));','int finalists=0;')
(p/'prefix_policy_agent_ideal6.cpp').write_text('#define SHRINK 1.4\n'+si)
si=si.replace('run<3;', 'run<6;')
(p/'prefix_policy_agent_ideal12.cpp').write_text('#define SHRINK 1.4\n'+si)

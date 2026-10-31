from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy_agent_v3.cpp').read_text().split('struct Group')[0]
s=s.replace('array<uint8_t,7>','array<uint8_t,8>')
s=s.replace('if((int)q.size()!=states)throw runtime_error("Unreachable display in template");','if(!q.empty()&&(int)q.size()!=states)throw runtime_error("Unreachable display in template");')
s=s.replace('auto policy=make_policy(vector<int>(values,1));','vector<int>fr(values,1);for(int t=0;t<values;t++)if(distance[t][0]==255)fr[t]=0;auto policy=make_policy(fr);')
s=s.replace('int t=rnd.pick(values);','int t=rnd.pick(values);if(!fr[t]){i--;continue;}')
s=s.replace('transition[a*values+b]+=w*distance[b][s];','if(fr[b])transition[a*values+b]+=w*distance[b][s];')
s+=r'''
int main(){
 for(int bits=4;bits<=8;bits++){
  vector<string> p(bits,string(bits,'.'));for(int i=0;i<bits;i++)p[i][i]='0',p[i][(i+1)%bits]='1';
  Graph g(p);g.build_transition();double avg=accumulate(g.transition.begin(),g.transition.end(),0.)/((g.values-2)*(g.values-2));int md=0;
  for(auto&v:g.distance)for(int x:v)if(x<255)md=max(md,x);
  cerr<<"bits "<<g.bits<<" states "<<g.states<<" uniformcost "<<avg<<" maxdistance "<<md<<endl;
 }
}
'''
(p/'policy_agent_cycle_stats.cpp').write_text(s)

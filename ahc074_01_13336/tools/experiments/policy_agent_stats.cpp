#define main v3_main
#include "policy_agent_v3.cpp"
#undef main
int main(){
 for(auto p:vector<vector<string>>{{"0....0","10...0",".10.00","111000",".11100","11..10",".....1"},{"1110","...1","0000","1.00","..1.","0100"}}){
  Graph g(p);g.build_transition();double avg=accumulate(g.transition.begin(),g.transition.end(),0.)/(g.values*g.values);int md=0;
  for(auto&v:g.distance)for(int x:v)md=max(md,x);
  cerr<<"bits "<<g.bits<<" states "<<g.states<<" uniformcost "<<avg<<" maxdistance "<<md<<endl;
 }
}

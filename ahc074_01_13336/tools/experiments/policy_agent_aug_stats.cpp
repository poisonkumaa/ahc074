#pragma GCC optimize("Ofast,unroll-loops")
#include "policy_agent_auggraph.h"
int main(int argc,char**argv){
 int bits=argc>1?atoi(argv[1]):9,limit=argc>2?atoi(argv[2]):2,improved=argc>3?atoi(argv[3]):1;
 auto start=clock();AugGraph g(bits,limit,improved);cerr<<"Graph "<<bits<<" "<<limit<<" states="<<g.S<<" edges="<<g.edges.size()<<" CPU="<<double(clock()-start)/CLOCKS_PER_SEC<<endl;
 AugPolicy policy(g);mt19937 rng(712367);int s=0;long long moves=0;int steps=argc>4?atoi(argv[4]):50000;vector<int>visits(g.S),total(g.V);
 for(int i=0;i<steps;i++){int t=rng()%g.V;while(g.display[s]!=t){s=policy.step(t,s).next;if(i>=2000)moves++;}if(i>=2000){visits[s]++;total[t]++;}}
 cerr<<"Average="<<double(moves)/(steps-2000)<<" CPU="<<double(clock()-start)/CLOCKS_PER_SEC<<" Cache="<<policy.cache.size()<<endl;
 vector<double>matrix(g.V*g.V);
 for(int s=0;s<g.S;s++)if(visits[s]){int a=g.display[s];double w=double(visits[s])/total[a];for(int t=0;t<g.V;t++)matrix[a*g.V+t]+=w*g.distance(s,t);}
 if(argc>5){ofstream out(argv[5],ios::binary);out.write((char*)matrix.data(),matrix.size()*sizeof(double));}
}

from pathlib import Path
root=Path(__file__).resolve().parent
src=(root/'policy_agent_v3.cpp').read_text()
begin=src.index(' vector<uint8_t> make_policy(')
end=src.index(' // Estimate transition costs',begin)
policy=r'''
 vector<uint8_t> make_policy(const vector<int>&frequency) const {
  vector<double> quality(states,0),cost(states),nextquality(states);
  int sum=accumulate(frequency.begin(),frequency.end(),0);
  for(int t=0;t<values;t++)if(frequency[t])for(int s=0;s<states;s++)quality[s]+=double(frequency[t])/sum*distance[t][s];
  vector<uint8_t> policy(values*states,255);
  const int ITER = POLICY_ITER;
  for(int it=0;it<ITER;it++){
   fill(nextquality.begin(),nextquality.end(),0);
   for(int t=0;t<values;t++){
    if(!frequency[t])continue;
    for(int s:bfs[t]){
     int d=distance[t][s];if(!d){cost[s]=quality[s];continue;}
     double best=1e100;int be=-1;
     for(int e=0;e<choices;e++){
      const auto &v=edges[s][e];
      if(!(visible[s]>>v.paper&1)||distance[t][v.next]+1!=d)continue;
      if(cost[v.next]<best-1e-9){best=cost[v.next];be=e;}
     }
     if(be<0)throw runtime_error("Missing legal shortest-path edge");
     cost[s]=best;policy[t*states+s]=(uint8_t)be;
    }
    double prob=double(frequency[t])/sum;
    for(int s=0;s<states;s++)nextquality[s]+=prob*(distance[t][s]+cost[s]);
   }
   double bias=nextquality[0];for(double &x:nextquality)x-=bias;
   quality.swap(nextquality);
  }
  return policy;
 }
'''
for n in [1,2,3,4,6,10]:
    out=src[:begin]+policy.replace('POLICY_ITER',str(n))+src[end:]
    # CPU-time budget does not shrink when other benchmarks are running.
    out=out.replace('const auto started = chrono::steady_clock::now();','const clock_t started = clock();')
    out=out.replace('chrono::duration<double>(chrono::steady_clock::now() - started).count()', 'double(clock()-started)/CLOCKS_PER_SEC')
    (root/f'policy_agent_iter{n}.cpp').write_text(out)

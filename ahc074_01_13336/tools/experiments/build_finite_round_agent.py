from pathlib import Path
p=Path(__file__).parent
s=(p/'multi_round_fast_agent.cpp').read_text()
s=s.replace('int desired_models=argc>4?atoi(argv[4]):3;','int desired_models=argc>4?atoi(argv[4]):2;')
s=s.replace('int count=128+state_limit*8;','int count=1024;')
a=s.index(' int states=0;bool expired=false;');b=s.index(' int winner=0;',a)
loop=r'''
 vector<int>weight(count);for(int r=0;r<16;r++)weight[r]=1;mt19937 moment_rng(761234);for(int first=16;first<count;first+=8)weight[first+moment_rng()%8]=8;
 int states=0;bool expired=false;
 for(int r=0;r<count&&!expired;r++){
  if(weight[r]){
   vector<int>cost;for(auto&s:sim){int c=s->mean_cost(r?targets[r-1]:-1,deadline);if(c<0){expired=true;break;}cost.push_back(c);}
   if(expired)break;for(int i=0;i<int(sim.size());i++)sim[i]->total+=(long long)cost[i]*weight[r]*(r?256:255);states++;
  }
  for(auto&s:sim){if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>deadline){expired=true;break;}s->step(targets[r],r);}
 }
'''
s=s[:a]+loop+s[b:]
(p/'finite_round_agent.cpp').write_text(s)

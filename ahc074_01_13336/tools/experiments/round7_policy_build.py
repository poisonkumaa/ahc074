from pathlib import Path
p=Path(__file__).resolve().parent
s=(p.parent.parent/'main.cpp').read_text()
s=s.replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100').replace('#define AHC_REFINEMENT_BUDGET 1.65','#define AHC_REFINEMENT_BUDGET 100')
s=s[:s.index('int main()')]+r'''
#include "round7_policy_rollout.h"
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);int n,K,P,R;if(!(cin>>n>>K>>P>>R))return 0;
 vector<U64>original(P);for(int v=0;v<P;v++){string x;cin>>x;for(int j=0;j<n;j++)if(x[j]=='1')original[v]|=1ULL<<j;}
 int period=argc>1?atoi(argv[1]):32,horizon=argc>2?atoi(argv[2]):32,validation=argc>3?atoi(argv[3]):8;double threshold=argc>4?atof(argv[4]):.25;
 structure2_training_origin=clock();auto model=train_multi_model(original,12345);exact_state_refit(model.A,model.papers,1,43,32,8,0);auto future=selection_teacher(model.A,model.papers);
 R7Simulator sim(model.A,model.papers);R7State state;state.future=future;state.order.resize(model.papers.size());iota(state.order.begin(),state.order.end(),0);
 for(int k=0;k<K;k++){string s(n,'.');if(k<int(model.papers.size()))for(int j=0;j<model.papers[k].len;j++)s[model.cols[j]]=char('0'+(((model.papers[k].color>>j)&1)^model.flip[j]));cout<<s<<'\n';}cout.flush();
 int decisions=0,accepted=0,paid=0,total_candidates=0,screened=0;double expected=0;
 for(int r=0;r<R;r++){
  int product;if(!(cin>>product)||product<0)return 0;vector<pair<int,int>>ops;sim.step(state,product,r,&ops);
  if(period>0&&r%period==0){decisions++;auto candidates=r7_candidates(sim,state,product,r,8);total_candidates+=candidates.size();
   auto pilot=r7_scenarios(sim,state,product,r,2,horizon,0x819731ULL+r*32767ULL);vector<pair<double,int>>ranking;
   for(int c=0;c<int(candidates.size());c++){double gain=0;for(auto&s:pilot)gain+=r7_rollout_gain(sim,candidates[c],s,r);ranking.push_back({gain/pilot.size(),c});}
   sort(ranking.rbegin(),ranking.rend());if(ranking.size()>2)ranking.resize(2);
   if(!ranking.empty()&&ranking[0].first>0){screened++;auto holdout=r7_scenarios(sim,state,product,r,validation,horizon,0x472918ULL+r*78233ULL);int best=-1;double bestgain=0;
    for(auto[prediction,c]:ranking)if(prediction>0){double sum=0,square=0;for(auto&s:holdout){double gain=r7_rollout_gain(sim,candidates[c],s,r);sum+=gain;square+=gain*gain;}double mean=sum/validation,variance=max(0.,square/validation-mean*mean),se=sqrt(variance/max(1,validation-1));if(mean>threshold+.5*se&&mean>bestgain){best=c;bestgain=mean;}}
    if(best>=0){accepted++;paid+=candidates[best].extra.size();expected+=bestgain;ops.insert(ops.end(),candidates[best].extra.begin(),candidates[best].extra.end());state=move(candidates[best].state);}
   }
  }
  assert(sim.display(state.order)==model.A[product]);assert(ops.size()<=384);cout<<ops.size()<<'\n';for(auto[k,d]:ops)cout<<k<<' '<<d<<'\n';cout.flush();
 }
 cerr<<"Decisions="<<decisions<<" Candidates="<<total_candidates<<" Screened="<<screened<<" Accepted="<<accepted<<" Paid="<<paid<<" Expected="<<expected<<" SimSteps="<<sim.steps<<" Couplings="<<sim.couplings<<" PathFallbacks="<<sim.fallbacks<<" FinishCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
}
'''
(p/'round7_policy_rollout.cpp').write_text(s)

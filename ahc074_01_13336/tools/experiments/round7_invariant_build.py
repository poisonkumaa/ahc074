from pathlib import Path
p=Path(__file__).resolve().parent
s=(p.parent.parent/'main.cpp').read_text()
s=s.replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100').replace('#define AHC_REFINEMENT_BUDGET 1.65','#define AHC_REFINEMENT_BUDGET 100')
s=s[:s.index('int main()')]
s=s.replace('namespace GlobalPrefix {','static bool r7_record=false;static long long r7_insertions=0,r7_forced_inversion=0,r7_quality_tradeoff=0;\nnamespace GlobalPrefix {',1)
old='int M=v.size(),kl=length[k];uint64_t remain[193];'
new='''int M=v.size(),kl=length[k];int r7mininv=0;if(r7_record){r7_insertions++;int lower=0,larger=0;for(int x:v)lower+=length[x]<kl;r7mininv=lower;for(int x:v){lower-=length[x]<kl;larger+=length[x]>kl;r7mininv=min(r7mininv,lower+larger);}r7_forced_inversion+=r7mininv>0;}uint64_t remain[193];'''
assert old in s;s=s.replace(old,new)
s=s.replace('  below[k]=remain[at];','  if(r7_record)r7_quality_tradeoff+=bestinv>r7mininv;\n  below[k]=remain[at];')
s+=r'''
#include "round7_policy_rollout.h"
int main(){
 ios::sync_with_stdio(false);cin.tie(nullptr);int n,K,P,R;if(!(cin>>n>>K>>P>>R))return 0;vector<U64>original(P);for(int v=0;v<P;v++){string x;cin>>x;for(int j=0;j<n;j++)if(x[j]=='1')original[v]|=1ULL<<j;}
 structure2_training_origin=clock();auto model=train_multi_model(original,12345);exact_state_refit(model.A,model.papers,1,43,32,8,0);auto future=selection_teacher(model.A,model.papers);
 R7Simulator sim(model.A,model.papers);R7State initial;initial.future=future;initial.order.resize(model.papers.size());iota(initial.order.begin(),initial.order.end(),0);auto sample=initial;mt19937 rng(441991);
 int unsorted_states=0,descents=0,avoid231_failures=0,weak_failures=0,maximum=0;bool example=false;r7_record=true;
 for(int r=0;r<4096;r++){
  int target=rng()%P;sim.step(sample,target,r);int high=0,previous=-1,count=0;vector<int>group,badgroup;
  for(int k:sample.order){int len=sim.length[k];if(len>high){high=len;previous=-1;group.clear();}else{if(previous>len){count++;badgroup=group;badgroup.push_back(len);}group.push_back(len);previous=len;}}
  if(count&&!example){example=true;cerr<<"UnsortedExample=";for(int x:badgroup)cerr<<x<<",";cerr<<"\n";}unsorted_states+=count>0;descents+=count;maximum=max(maximum,count);
  int suffix[193];suffix[sample.order.size()]=65;for(int i=int(sample.order.size())-1;i>=0;i--)suffix[i]=min(suffix[i+1],sim.length[sample.order[i]]);
  bool seen[65]{},failure=false,weak=false;for(int i=0;i<int(sample.order.size());i++){int len=sim.length[sample.order[i]];for(int lower=suffix[i+1]+1;lower<len;lower++)if(seen[lower])failure=true;for(int lower=suffix[i+1];lower<=len;lower++)if(seen[lower]&&(lower<len||suffix[i+1]<lower))weak=true;seen[len]=true;}avoid231_failures+=failure;weak_failures+=weak;
 }
 r7_record=false;cerr<<"PublicStates=4096 UnsortedStates="<<unsorted_states<<" Descents="<<descents<<" MaxDescents="<<maximum<<" Pattern231States="<<avoid231_failures<<" WeakPatternStates="<<weak_failures<<" Insertions="<<r7_insertions<<" ForcedInsertionInversion="<<r7_forced_inversion<<" UnionVsInversionTradeoff="<<r7_quality_tradeoff<<"\n";
 auto state=initial;for(int k=0;k<K;k++){string s(n,'.');if(k<int(model.papers.size()))for(int j=0;j<model.papers[k].len;j++)s[model.cols[j]]=char('0'+(((model.papers[k].color>>j)&1)^model.flip[j]));cout<<s<<'\n';}cout.flush();
 for(int r=0;r<R;r++){int product;if(!(cin>>product)||product<0)return 0;vector<pair<int,int>>ops;sim.step(state,product,r,&ops);cout<<ops.size()<<'\n';for(auto[k,d]:ops)cout<<k<<' '<<d<<'\n';cout.flush();}
 cerr<<"FinishCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
}
'''
(p/'round7_invariant.cpp').write_text(s)

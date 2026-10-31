from pathlib import Path
p=Path(__file__).resolve().parent
base=(p.parent.parent/'main.cpp').read_text()
start=base.index('inline std::pair<int,int> best_move(')
opening=base.index('{',start);depth=1;end=opening+1
while depth:
 if base[end]=='{':depth+=1
 elif base[end]=='}':depth-=1
 end+=1
function=base[start:end]
function=function.replace('const std::vector<uint64_t>* access=nullptr)', 'const std::vector<uint64_t>* access, const vector<int>&reference, double weight)')
function=function.replace('double decay=.5','double decay')
function=function.replace('int following_min=suffix[i+1];double quality=initial_quality;', 'int refdelta=0;int following_min=suffix[i+1];double quality=initial_quality;')
function=function.replace('int x=order[d],xl=length[x],old_invk=invk;', 'int x=order[d],xl=length[x],old_invk=invk;refdelta+=direction*((reference[k]<reference[x])-(reference[k]>reference[x]));')
function=function.replace('if(distance!=initial_distance-1)continue;', 'if(distance!=initial_distance-1)continue;double ranked_quality=quality+weight*refdelta;')
function=function.replace('quality<best_quality','ranked_quality<best_quality').replace('std::abs(quality-best_quality)','std::abs(ranked_quality-best_quality)').replace('best_quality=quality;','best_quality=ranked_quality;')
s=base[:base.index('int main()')].replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100').replace('#define AHC_REFINEMENT_BUDGET 1.65','#define AHC_REFINEMENT_BUDGET 100')
s+='\n#include "round7_policy_rollout.h"\nnamespace R7Geometry {\n'+function+'\n}\n'+r'''
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);int n,K,P,R;if(!(cin>>n>>K>>P>>R))return 0;
 vector<U64>original(P);for(int v=0;v<P;v++){string x;cin>>x;for(int j=0;j<n;j++)if(x[j]=='1')original[v]|=1ULL<<j;}
 int mode=argc>1?atoi(argv[1]):0;double weight=argc>2?atof(argv[2]):1000000;
 structure2_training_origin=clock();auto model=train_multi_model(original,12345);exact_state_refit(model.A,model.papers,1,43,32,8,0);auto future=selection_teacher(model.A,model.papers);
 R7Simulator sim(model.A,model.papers);R7State state;state.future=future;state.order.resize(model.papers.size());iota(state.order.begin(),state.order.end(),0);int active=state.order.size();
 auto expected=[&](const vector<int>&order){sim.dp.prepare(order);double total=0;for(U64 target:model.A){int distance;sim.dp.solve_prepared(target,distance,false);total+=distance;}return total/P;};
 double initial=expected(state.order);auto descending=state.order;stable_sort(descending.begin(),descending.end(),[&](int a,int b){return sim.length[a]>sim.length[b];});double descending_cost=expected(descending);
 auto sample=state;vector<long long>position_sum(active);vector<int>best_order;double stationary=0,best=1e100;mt19937 rng(413729);int previous=-1;
 for(int r=0;r<128+16*16;r++){int product=rng()%P;while(product==previous)product=rng()%P;previous=product;sim.step(sample,product,r);if(r>=128&&(r-128)%16==0){for(int i=0;i<active;i++)position_sum[sample.order[i]]+=i;double cost=expected(sample.order);stationary+=cost/16;if(cost<best){best=cost;best_order=sample.order;}}}
 auto median=state.order;stable_sort(median.begin(),median.end(),[&](int x,int y){return position_sum[x]<position_sum[y];});double median_cost=expected(median);
 vector<int>reference(active);if(mode==1)reference=sim.length;else{auto&order=mode==2?median:best_order;for(int i=0;i<active;i++)reference[order[i]]=i;}
 cerr<<"InitialMean="<<initial<<" DescendingMean="<<descending_cost<<" StationaryMean="<<stationary<<" BestMean="<<best<<" MedianMean="<<median_cost<<"\n";
 for(int k=0;k<K;k++){string s(n,'.');if(k<active)for(int j=0;j<model.papers[k].len;j++)s[model.cols[j]]=char('0'+(((model.papers[k].color>>j)&1)^model.flip[j]));cout<<s<<'\n';}cout.flush();
 long long total=0;
 for(int r=0;r<R;r++){
  int product;if(!(cin>>product)||product<0)return 0;vector<pair<int,int>>ops;
  if(mode==0)sim.step(state,product,r,&ops);
  else{
   for(int k=0;k<active;k++)sim.dp.reward[k]=16+2*__builtin_popcountll(state.future[k]);int moves;auto chosen=sim.dp.solve(state.order,model.A[product],moves);vector<char>selected(active);for(int k:chosen)selected[k]=true;
   for(int i=0;i<moves;i++){auto[from,to]=R7Geometry::best_move(state.order,sim.length,selected,.5,&state.future,reference,weight);assert(from>=0);int k=state.order[from];state.order.erase(state.order.begin()+from);state.order.insert(state.order.begin()+to,k);ops.push_back({k,to});}
   U64 bit=1ULL<<(r%64);for(int k=0;k<active;k++)state.future[k]=(state.future[k]&~bit)|(selected[k]?bit:0ULL);
  }
  assert(sim.display(state.order)==model.A[product]);assert(ops.size()<=384);total+=ops.size();cout<<ops.size()<<'\n';for(auto[k,d]:ops)cout<<k<<' '<<d<<'\n';cout.flush();
 }
 cerr<<"GeometryMode="<<mode<<" Weight="<<weight<<" FinalMean="<<expected(state.order)<<" FinishCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
}
'''
(p/'round7_geometry.cpp').write_text(s)

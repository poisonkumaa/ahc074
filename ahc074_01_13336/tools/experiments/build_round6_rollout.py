from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'round5_sampling_adaptive.cpp').read_text()
s=s.replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100').replace('#define AHC_REFINEMENT_BUDGET 1.65','#define AHC_REFINEMENT_BUDGET 100')
mark='inline void exact_state_refit('
helper=r'''
static int rollout_count=512;
static int rollout_cost(const vector<U64>&A,const vector<PrefixPaper>&papers){
 PrefixDP dp(64,papers);dp.public_data(A);vector<int>order(papers.size()),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);vector<U64>future(papers.size());mt19937 rng(3671279);int previous=-1,total=0;
 for(int r=0;r<128+rollout_count;r++){int target=rng()%A.size();while(target==previous)target=rng()%A.size();previous=target;for(int k=0;k<int(papers.size());k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);int moves;auto goal=dp.solve(order,A[target],moves);vector<char>selected(papers.size());for(int k:goal)selected[k]=true;order=direct_final(order,length,selected,&future);U64 bit=1ULL<<(r%64);for(int k=0;k<int(papers.size());k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);if(r>=128)total+=moves;}
 return total;
}
'''
s=s.replace(mark,helper+mark,1)
s=s.replace(' int accepted=0;long long total_gain=0;auto began=clock();double forward_time=0;size_t finite_states=0;', ' int accepted=0;long long total_gain=0;auto began=clock();double forward_time=0;size_t finite_states=0;int rollout_best=rollout_cost(A,papers),rollout_reject=0;')
s=s.replace('   int Q=level[l].size();','   int Q=level[l].size();auto before_level=papers;int before_accepted=accepted;long long before_gain=total_gain;')
marker='  for(int e=0;e<M;e++){int v=scenarios[e].target;auto&t=*tables[e];for(int q=0;q<Q;q++){int z=level[l][q],best=INF;'
assert marker in s
s=s.replace(marker,r'''
  if(accepted>before_accepted){int score=rollout_cost(A,papers);if(score<=rollout_best)rollout_best=score;else{rollout_reject++;papers=before_level;accepted=before_accepted;total_gain=before_gain;for(int z:level[l])update(z);for(int e=0;e<M;e++)for(int q=0;q<Q;q++)through[e*Q+q]=route[e*Q+q][start[scenarios[e].target][level[l][q]]];}}
'''+marker)
s=s.replace('cerr<<"exact_refit accepted="','cerr<<"RolloutBest="<<rollout_best<<" Rejected="<<rollout_reject<<"\\n";cerr<<"exact_refit accepted="')
s=s.replace('int main(){','int main(int argc,char**argv){rollout_count=argc>1?atoi(argv[1]):512;')
s=s.replace('exact_state_refit(A,papers,1,43,32,-1,0);','exact_state_refit(A,papers,1,43,32,argc>2?atoi(argv[2]):8,0);')
(d/'round6_rollout.cpp').write_text(s)

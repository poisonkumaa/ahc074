#pragma once
#include "round7_lengthlocal.h"
inline void source_product_swap(const vector<U64>&A,vector<PrefixPaper>&papers,int passes,int pair_count,int snapshot_count,int validation_states=0){
 const int P=A.size(),K=papers.size();mt19937 rng(47123691);auto began=clock();int accepted=0;
 auto stationary_cost=[&](){
  assert(validation_states>0&&validation_states<=1024);PrefixDP dp(64,papers);dp.public_data(A);auto future=selection_teacher(A,papers);vector<int>order(K),length; iota(order.begin(),order.end(),0);for(auto p:papers)length.push_back(p.len);mt19937 history_rng(512273);int previous=-1,next_sample=0;vector<Round7LengthLocal::Snapshot>states;
  for(int r=0;r<128+1024;r++){
   int target=history_rng()%P;while(target==previous)target=history_rng()%P;
   for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);dp.prepare(order);
   if(next_sample<validation_states&&r-128==(2*next_sample+1)*1024/(2*validation_states)){
    Round7LengthLocal::Snapshot s;s.target=target;for(int k=0;k<K;k++)s.pos[k]=dp.pos[k];for(int l=0;l<=65;l++)for(int h=0;h<=K;h++)s.cnt[l][h]=dp.cnt[l][h];states.push_back(s);next_sample++;
   }
   int moves;auto chosen=dp.solve_prepared(A[target],moves);vector<char>selected(K);for(int k:chosen)selected[k]=true;order=direct_final(order,length,selected,&future);U64 bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);previous=target;
  }
  assert(int(states.size())==validation_states);Round7LengthLocal::Evaluator evaluator(A,papers);long long total=0;for(auto&s:states)for(int v=0;v<P;v++){s.target=v;total+=evaluator.distance(s,0,0,0);}return total;
 };
 for(int pass=0;pass<passes;pass++){
  auto snapshots=Round7LengthLocal::capture(A,papers,snapshot_count);Round7LengthLocal::Evaluator evaluator(A,papers);
  vector<int>source(K,-1),eligible;for(int k=0;k<K;k++)if(papers[k].len>=43){for(int v=0;v<P;v++)if((A[v]&PrefixDP::low(papers[k].len))==(papers[k].color&PrefixDP::low(papers[k].len))){source[k]=v;break;}if(source[k]>=0)eligible.push_back(k);}
  auto feasible=[&](int x,int y,PrefixPaper nx,PrefixPaper ny){
   for(int v=0;v<P;v++){
    int sx=evaluator.matching_start(v,nx),sy=evaluator.matching_start(v,ny);
    for(int k:evaluator.level[nx.len])if(k!=x&&k!=y)sx=min(sx,evaluator.matching_start(v,papers[k]));
    for(int k:evaluator.level[ny.len])if(k!=x&&k!=y)sy=min(sy,evaluator.matching_start(v,papers[k]));
    int reached=0;for(int l=1;l<=64;l++){int start=l==nx.len?sx:l==ny.len?sy:evaluator.minstart[v][l];if(start<=reached)reached=l;}if(reached!=64)return false;
   }return true;
  };
  auto evaluate=[&](){long long score=0;for(const auto&s:snapshots){int d=evaluator.distance(s,0,0,0);assert(d<1000000);score+=d;}return score;};
  long long before=evaluate(),best=before;int bx=-1,by=-1,tried=0,valid=0;vector<pair<int,int>>pairs;
  for(int i=0;i<int(eligible.size());i++)for(int j=i+1;j<int(eligible.size());j++){int x=eligible[i],y=eligible[j];if(papers[x].len!=papers[y].len&&source[x]!=source[y])pairs.push_back({x,y});}
  shuffle(pairs.begin(),pairs.end(),rng);if(int(pairs.size())>pair_count)pairs.resize(pair_count);
  for(auto[x,y]:pairs){
   tried++;auto ox=papers[x],oy=papers[y];PrefixPaper nx{ox.len,A[source[y]]&PrefixDP::low(ox.len)},ny{oy.len,A[source[x]]&PrefixDP::low(oy.len)};
   if(!feasible(x,y,nx,ny))continue;valid++;
   evaluator.update_candidate(x,ox,nx);evaluator.update_candidate(y,oy,ny);papers[x]=nx;papers[y]=ny;
   long long score=evaluate();papers[x]=ox;papers[y]=oy;evaluator.update_candidate(y,ny,oy);evaluator.update_candidate(x,nx,ox);
   if(score<best){best=score;bx=x;by=y;}
  }
  cerr<<"SourceSwap pass="<<pass<<" before="<<before<<" after="<<best<<" samples="<<snapshots.size()<<" tried="<<tried<<" feasible="<<valid;
  if(bx>=0){
   auto nx=PrefixPaper{papers[bx].len,A[source[by]]&PrefixDP::low(papers[bx].len)},ny=PrefixPaper{papers[by].len,A[source[bx]]&PrefixDP::low(papers[by].len)};
   cerr<<" length="<<papers[bx].len<<","<<papers[by].len<<" source="<<source[bx]<<","<<source[by];auto ox=papers[bx],oy=papers[by];long long original_validation=validation_states?stationary_cost():0;papers[bx]=nx;papers[by]=ny;
#ifdef SOURCESWAP_CHECK
   PrefixDP check(64,papers);check.public_data(A);long long sum=0;for(const auto&s:snapshots){vector<int>order(K);for(int k=0;k<K;k++)order[s.pos[k]]=k;int moves;check.solve(order,A[s.target],moves,false);sum+=moves;}assert(sum==best);
#endif
   if(validation_states){long long changed_validation=stationary_cost();cerr<<" validation="<<original_validation<<","<<changed_validation;if(changed_validation>=original_validation){papers[bx]=ox;papers[by]=oy;bx=-1;cerr<<" veto=1";}}
   if(bx>=0)accepted++;
  }
  cerr<<" CPU="<<double(clock()-began)/CLOCKS_PER_SEC<<"\n";if(bx<0)break;
 }
 cerr<<"SourceAccepted="<<accepted<<" CPU="<<double(clock()-began)/CLOCKS_PER_SEC<<"\n";
}

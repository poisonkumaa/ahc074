#pragma once
inline vector<uint64_t> selection_teacher(const vector<U64>&A,const vector<PrefixPaper>&papers){
 int P=A.size();PrefixDP dp(64,papers);vector<int>order(papers.size()),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);
 vector<uint64_t>future(papers.size());mt19937 freq_rng(912367);vector<int>train_targets(P);iota(train_targets.begin(),train_targets.end(),0);shuffle(train_targets.begin(),train_targets.end(),freq_rng);
 for(int r=0;r<192;r++){
  int target=r<128?freq_rng()%P:train_targets[r-128],moves;auto chosen=dp.solve(order,A[target],moves);vector<char>selected(papers.size());for(int k:chosen){selected[k]=true;if(r>=128)future[k]|=1ULL<<(r-128);}
  for(int t=0;t<moves;t++){auto[from,to]=GlobalPrefix::best_move(order,length,selected);if(from<0)abort();int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);}
 }return future;
}
inline int selection_evaluate(const vector<U64>&A,const vector<PrefixPaper>&papers,vector<uint64_t>future,const vector<int>&targets,double guard,vector<int>&partial){
 PrefixDP dp(64,papers);vector<int>order(papers.size()),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);int total=0;partial.clear();
 for(int r=0;r<int(targets.size());r++){
  if((r&15)==0&&double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>guard)return INT_MAX;
  for(int k=0;k<int(papers.size());k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);
  int moves;auto chosen=dp.solve(order,A[targets[r]],moves);vector<char>selected(papers.size());for(int k:chosen)selected[k]=true;
  for(int i=0;i<moves;i++){auto[from,to]=GlobalPrefix::best_move(order,length,selected,.5,&future);if(from<0)abort();int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);}
  uint64_t bit=1ULL<<(r%64);for(int k=0;k<int(papers.size());k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
  total+=moves;partial.push_back(total);
 }return total;
}
inline vector<uint64_t> selection_design(const vector<U64>&A,vector<PrefixPaper>&papers,int count,double guard,int extra){
 auto base=papers;int P=A.size();vector<int>targets;mt19937 rng(377321);vector<int>batch(P);iota(batch.begin(),batch.end(),0);while(int(targets.size())<count){shuffle(batch.begin(),batch.end(),rng);if(!targets.empty()&&batch[0]==targets.back())swap(batch[0],batch[1]);for(int v:batch)if(int(targets.size())<count)targets.push_back(v);}
 refit_all_prefix_papers(A,papers,4,32);auto bestpapers=papers;auto bestfuture=selection_teacher(A,papers);vector<int>part;int best=selection_evaluate(A,papers,bestfuture,targets,guard,part),winner=0;
 cerr<<"Selection strong="<<best<<" CPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
 for(int c=1;c<=1+extra;c++){
  if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>guard-.05)break;
  papers=base;refit_all_prefix_papers(A,papers,c==1?1:2,c==1?8:16);auto future=selection_teacher(A,papers);
  int score=selection_evaluate(A,papers,future,targets,guard,part);
  cerr<<"Selection "<<c<<"="<<score<<" CPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
  if(score<best){best=score;bestpapers=papers;bestfuture=future;winner=c;}
 }
 papers=bestpapers;cerr<<"Selection winner="<<winner<<"\n";return bestfuture;
}

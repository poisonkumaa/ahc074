#pragma once
inline U64 column_swap_bits(U64 x,int j){U64 d=((x>>j)^(x>>(j+1)))&1;return x^(d<<j)^(d<<(j+1));}
inline bool column_reachable(const vector<U64>&A,const vector<PrefixPaper>&papers){
 for(U64 a:A){bool reachable[65]{};reachable[0]=true;for(int l=1;l<=64;l++)for(auto p:papers)if(p.len==l){U64 d=(a^p.color)&PrefixDP::low(l);int start=d?64-__builtin_clzll(d):0;for(int j=start;j<l;j++)if(reachable[j]){reachable[l]=true;break;}}if(!reachable[64])return false;}return true;
}
inline void column_local_search(vector<U64>&A,vector<PrefixPaper>&papers,vector<int>&cols,vector<int>&flip,int passes,int repetitions){
 int K=papers.size(),P=A.size();vector<int>length;for(auto p:papers)length.push_back(p.len);auto began=clock();int accepted=0;
 mt19937 rng(73219);vector<int>targets(P);iota(targets.begin(),targets.end(),0);
 for(int pass=0;pass<passes;pass++){
  vector<vector<int>>states;vector<int>request;PrefixDP dp(64,papers);dp.public_data(A);vector<int>order(K);iota(order.begin(),order.end(),0);auto future=selection_teacher(A,papers);
  for(int r=0;r<128+P*repetitions;r++){
   if(r>=128&&(r-128)%P==0)shuffle(targets.begin(),targets.end(),rng);int t=r<128?rng()%P:targets[(r-128)%P];
   if(r>=128){states.push_back(order);request.push_back(t);}for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);int moves;auto chosen=dp.solve(order,A[t],moves);vector<char>selected(K);for(int k:chosen)selected[k]=true;order=direct_final(order,length,selected,&future);U64 bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
  }
  auto evaluate=[&](const vector<U64>&data,const vector<PrefixPaper>&p){PrefixDP d(64,p);d.public_data(data);int score=0;for(int i=0;i<int(states.size());i++){int moves;d.solve(states[i],data[request[i]],moves,false);score+=moves;}return score;};
  int current=evaluate(A,papers),best=current,bestj=-1;vector<U64>bestA;vector<PrefixPaper>bestP;
  for(int j=0;j<63;j++){
   auto data=A;auto p=papers;for(U64&v:data)v=column_swap_bits(v,j);for(auto&x:p)if(x.len>j+1)x.color=column_swap_bits(x.color,j);
   column_level=j+1;refit_all_prefix_papers(data,p,1,32);column_level=-1;if(!column_reachable(data,p))continue;
   int score=evaluate(data,p);if(score<best){best=score;bestj=j;bestA=move(data);bestP=move(p);}
  }
  cerr<<"ColumnLocal pass="<<pass<<" gain="<<double(current-best)/states.size()<<" boundary="<<bestj+1<<" CPU="<<double(clock()-began)/CLOCKS_PER_SEC<<"\n";
  if(bestj<0)break;A=move(bestA);papers=move(bestP);swap(cols[bestj],cols[bestj+1]);swap(flip[bestj],flip[bestj+1]);accepted++;
 }
 cerr<<"ColumnAccepted="<<accepted<<"\n";
}

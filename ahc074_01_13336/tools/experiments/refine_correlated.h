#pragma once
inline std::vector<PrefixPaper> select_refined_correlated(const std::vector<uint64_t>&A,const std::vector<PrefixPaper>&base,double shrink,int count){
 std::mt19937_64 rng(A[0]^A[97]^0x671ee779abcULL);std::vector<int>orders;
 for(int i=0;i<16;i++)orders.push_back(rng()%A.size());
 std::vector<int> shuffled(A.size());std::iota(shuffled.begin(),shuffled.end(),0);std::shuffle(shuffled.begin(),shuffled.end(),rng);orders.insert(orders.end(),shuffled.begin(),shuffled.end());
 double best=1e100;std::vector<PrefixPaper>answer;int picked=-1;
 for(int c=0;c<count;c++){
  if(c&&double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>.85)break;
  auto paper=base;
  if(c==0)refit_all_prefix_papers(A,paper,1,8);
  if(c==1)refit_all_prefix_papers(A,paper,2,32);
  if(c==2)refit_cost_papers(A,paper,1,1,2);
  if(c==3){refit_all_prefix_papers(A,paper,1,8);refit_cost_papers(A,paper,1,1,2);}
  int K=paper.size();std::vector<int>order(K),length;std::iota(order.begin(),order.end(),0);for(auto p:paper)length.push_back(p.len);
  PrefixDP dp(64,paper);int total=0;
  std::vector<std::array<uint64_t,4>>future(K);std::mt19937 freq_rng(912367);std::vector<int>tt(A.size());std::iota(tt.begin(),tt.end(),0);std::shuffle(tt.begin(),tt.end(),freq_rng);
  for(int r=0;r<192;r++){
   int v=r<128?freq_rng()%A.size():tt[r-128],dist;auto chosen=dp.solve(order,A[v],dist);std::vector<char>sel(K);for(int k:chosen){sel[k]=true;if(r>=128)future[k][0]|=1ULL<<(r-128);}
   for(int m=0;m<dist;m++){auto[from,to]=GlobalPrefix::best_move(order,length,sel);if(from<0)std::abort();int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);}
  }
  std::iota(order.begin(),order.end(),0);

  for(int r=0;r<int(orders.size());r++){
   int distance;auto chosen=dp.solve(order,A[orders[r]],distance);if(r>=16)total+=distance;
   std::vector<char>selected(K);for(int k:chosen)selected[k]=true;
   for(int m=0;m<distance;m++){auto[from,to]=GlobalPrefix::best_move(order,length,selected,.5,&future,1);if(from<0)std::abort();int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);}
  }
  int intrinsic=0;for(auto a:A){int f[65];std::fill(f,f+65,1000);f[0]=0;for(auto p:paper){uint64_t d=(a^p.color)&PrefixDP::low(p.len);int st=d?64-__builtin_clzll(d):0;for(int j=st;j<p.len;j++)f[p.len]=std::min(f[p.len],f[j]+1);}intrinsic+=f[64];}
  double value=(total+shrink*intrinsic)/A.size();std::cerr<<"refine_choice="<<c<<" estimate="<<double(total)/A.size()<<" records="<<double(intrinsic)/A.size()<<" CPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
  if(value<best)best=value,answer=paper,picked=c;
 }
 std::cerr<<"refine_selected="<<picked<<"\n";return answer;
}

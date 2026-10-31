#pragma once
inline void round7_report_product_cost(const vector<U64>&A,const vector<PrefixPaper>&papers){
 const int P=A.size(),K=papers.size();PrefixDP dp(64,papers);dp.public_data(A);vector<int>order(K),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);
 auto future=selection_teacher(A,papers);vector<long long>sum(P),count(P),record(P),visits(P),ownuse(P),paperuse(K);vector<int>ownlength(P),ownpaper(P,-1);
 for(int v=0;v<P;v++)for(int k=0;k<K;k++)if(papers[k].len>ownlength[v]&&((A[v]^papers[k].color)&PrefixDP::low(papers[k].len))==0)ownlength[v]=papers[k].len,ownpaper[v]=k;
 mt19937 rng(0x7152a91);vector<int>targets(P);iota(targets.begin(),targets.end(),0);int previous=-1;
 for(int r=0;r<128+1024;r++){
  if(r>=128&&(r-128)%P==0)shuffle(targets.begin(),targets.end(),rng);
  int target=r<128?rng()%P:targets[(r-128)%P];for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);
  if(r>=128&&(r-128)%32==0){dp.prepare(order);for(int v=0;v<P;v++)if(v!=previous){int moves;dp.solve_prepared(A[v],moves,false);sum[v]+=moves;count[v]++;}}
  int moves;auto chosen=dp.solve(order,A[target],moves);vector<char>selected(K);for(int k:chosen)selected[k]=true;
  if(r>=128){visits[target]++;record[target]+=chosen.size();for(int k:chosen)paperuse[k]++;if(ownpaper[target]>=0&&selected[ownpaper[target]])ownuse[target]++;}
  order=direct_final(order,length,selected,&future);U64 bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);previous=target;
 }
 cerr<<"FairMetrics {\"products\":[";for(int v=0;v<P;v++){if(v)cerr<<",";cerr<<"{\"id\":"<<v<<",\"cost\":"<<double(sum[v])/count[v]<<",\"own_length\":"<<ownlength[v]<<",\"records\":"<<double(record[v])/visits[v]<<",\"own_probability\":"<<double(ownuse[v])/visits[v]<<",\"own_paper_usage\":"<<(ownpaper[v]<0?0:paperuse[ownpaper[v]])<<"}";}cerr<<"],\"paper_usage\":[";for(int k=0;k<K;k++){if(k)cerr<<",";cerr<<paperuse[k];}cerr<<"]}\n";
}

#pragma once
namespace Round7LengthLocal {
struct Snapshot {
 int target;
 std::array<uint8_t,192>pos;
 std::array<std::array<uint8_t,193>,66>cnt;
};
struct Evaluator {
 const std::vector<U64>&A;
 std::vector<PrefixPaper>&papers;
 std::vector<std::array<std::vector<uint8_t>,64>>candidate;
 std::vector<std::array<int,65>>minstart;
 std::array<std::vector<int>,65>level;
 int stamp[65][193]{},work[65][193]{},epoch=0;
 Evaluator(const std::vector<U64>&a,std::vector<PrefixPaper>&p):A(a),papers(p),candidate(A.size()),minstart(A.size()){
  for(int k=0;k<int(papers.size());k++)level[papers[k].len].push_back(k);
  for(int v=0;v<int(A.size());v++){
   minstart[v].fill(1000);
   for(int k=0;k<int(papers.size());k++){
    auto paper=papers[k];int start=matching_start(v,paper);
    minstart[v][paper.len]=std::min(minstart[v][paper.len],start);
    for(int j=start;j<paper.len;j++)candidate[v][j].push_back(k);
   }
  }
 }
 int matching_start(int v,PrefixPaper paper)const {U64 diff=(A[v]^paper.color)&PrefixDP::low(paper.len);return diff?64-__builtin_clzll(diff):0;}
 bool feasible(int changed,PrefixPaper replacement)const {
  int oldlen=papers[changed].len,newlen=replacement.len;
  for(int v=0;v<int(A.size());v++){
   int os=1000,ns=matching_start(v,replacement);
   for(int k:level[oldlen])if(k!=changed)os=std::min(os,matching_start(v,papers[k]));
   for(int k:level[newlen])if(k!=changed)ns=std::min(ns,matching_start(v,papers[k]));
   int reached=0;for(int l=1;l<=64;l++){int start=l==oldlen?os:l==newlen?ns:minstart[v][l];if(start<=reached)reached=l;}
   if(reached!=64)return false;
  }return true;
 }
 void update_candidate(int k,PrefixPaper oldpaper,PrefixPaper newpaper){
  for(int v=0;v<int(A.size());v++){
   int oldstart=matching_start(v,oldpaper),newstart=matching_start(v,newpaper);
   for(int j=std::min(oldstart,newstart);j<std::max(oldpaper.len,newpaper.len);j++){
    bool was=oldstart<=j&&j<oldpaper.len,now=newstart<=j&&j<newpaper.len;if(was==now)continue;
    auto&c=candidate[v][j];auto at=std::lower_bound(c.begin(),c.end(),k);
    if(was){assert(at!=c.end()&&*at==k);c.erase(at);}else c.insert(at,k);
   }
  }
 }
 int distance(const Snapshot&s,int changed,int boundary,int sign){
  epoch++;int active[65][193],count[65]{};work[0][0]=0;stamp[0][0]=epoch;active[0][count[0]++]=0;int cp=s.pos[changed];
  for(int j=0;j<64;j++){
   if(!count[j])continue;
   for(int z:candidate[s.target][j]){
    int l=papers[z].len,q=s.pos[z];
    for(int i=0;i<count[j];i++){
     int h=active[j][i],add=0;
     if(q>=h){int correction=sign*(h<=cp&&cp<q),moved=int(s.cnt[l][q])-s.cnt[l][h]+(l==boundary?correction:0),upper=int(s.cnt[j+1][q])-s.cnt[j+1][h]+(j+1==boundary?correction:0);add=moved+(upper>moved);}
     int nh=std::max(q+1,h),value=work[j][h]+add;
     if(stamp[l][nh]!=epoch){stamp[l][nh]=epoch;work[l][nh]=1000000;active[l][count[l]++]=nh;}
     if(value<work[l][nh])work[l][nh]=value;
    }
   }
  }
  int best=1000000;for(int i=0;i<count[64];i++)best=std::min(best,work[64][active[64][i]]);return best;
 }
 long long evaluate(const std::vector<Snapshot>&snapshots,int changed,PrefixPaper replacement){
  auto original=papers[changed];int boundary=std::max(original.len,replacement.len),sign=replacement.len>original.len?1:replacement.len<original.len?-1:0;
  update_candidate(changed,original,replacement);papers[changed]=replacement;
  long long total=0;
  for(const auto&s:snapshots){int d=distance(s,changed,boundary,sign);assert(d<1000000);total+=d;
#ifdef LENGTHLOCAL_CHECK
   PrefixDP check(64,papers);check.public_data({A[s.target]});std::vector<int>order(papers.size());for(int k=0;k<int(papers.size());k++)order[s.pos[k]]=k;int exact;check.solve(order,A[s.target],exact,false);assert(d==exact);
#endif
  }
  papers[changed]=original;update_candidate(changed,replacement,original);return total;
 }
};
std::vector<Snapshot>capture(const std::vector<U64>&A,const std::vector<PrefixPaper>&papers,int count){
 int P=A.size(),K=papers.size();PrefixDP dp(64,papers);dp.public_data(A);std::vector<int>order(K),length;std::iota(order.begin(),order.end(),0);for(auto p:papers)length.push_back(p.len);
 auto future=selection_teacher(A,papers);std::mt19937 rng(745813);std::vector<int>targets(P);std::iota(targets.begin(),targets.end(),0);std::shuffle(targets.begin(),targets.end(),rng);std::vector<Snapshot>out;out.reserve(count);
 int previous=-1;
 for(int r=0;r<128+count;r++){
  int target=r<128?int(rng()%P):targets[(r-128)%P];if(target==previous)target=(target+1)%P;
  for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);
  dp.prepare(order);
  if(r>=128){Snapshot s;s.target=target;for(int k=0;k<K;k++)s.pos[k]=dp.pos[k];for(int l=0;l<=65;l++)for(int h=0;h<=K;h++)s.cnt[l][h]=dp.cnt[l][h];out.push_back(s);}
  int moves;auto chosen=dp.solve_prepared(A[target],moves);std::vector<char>selected(K);for(int k:chosen)selected[k]=true;order=direct_final(order,length,selected,&future);
  U64 bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);previous=target;
 }
 return out;
}
void search(const std::vector<U64>&A,std::vector<PrefixPaper>&papers,int passes,int samples){
 clock_t started=clock();int accepted=0;
 for(int pass=0;pass<passes;pass++){
  auto snapshots=capture(A,papers,samples);Evaluator evaluate(A,papers);long long before=evaluate.evaluate(snapshots,0,papers[0]),best=before;int chosen=-1,feasible=0;PrefixPaper replacement{};
  for(int k=0;k<int(papers.size());k++){
   auto old=papers[k];std::vector<PrefixPaper>options;
   if(old.len>1)options.push_back({old.len-1,old.color&PrefixDP::low(old.len-1)});
   if(old.len<64)for(int b=0;b<2;b++)options.push_back({old.len+1,(old.color&PrefixDP::low(old.len))|(U64(b)<<old.len)});
   for(auto p:options)if(evaluate.feasible(k,p)){
    feasible++;long long value=evaluate.evaluate(snapshots,k,p);if(value<best){best=value;chosen=k;replacement=p;}
   }
  }
  std::cerr<<"LengthPass="<<pass<<" before="<<before<<" after="<<best<<" candidates="<<feasible;
  if(chosen>=0){std::cerr<<" paper="<<chosen<<" length="<<papers[chosen].len<<"->"<<replacement.len;papers[chosen]=replacement;accepted++;}
  std::cerr<<" CPU="<<double(clock()-started)/CLOCKS_PER_SEC<<"\n";
  if(chosen<0)break;
 }
 std::cerr<<"LengthAccepted="<<accepted<<"\n";
}
}

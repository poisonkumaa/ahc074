#pragma once
inline void simulation_refit_prefix(const std::vector<uint64_t>&A,std::vector<PrefixPaper>&papers,int importance=8,int minimum_length=44,int margin=12,clock_t time_origin=0,double cpu_limit=1e100,unsigned seed=173234){
 const int P=A.size(),N=64,K=papers.size(),INF=1000;std::vector<std::vector<int>>level(65),start(P,std::vector<int>(K));
 for(int k=0;k<K;k++)level[papers[k].len].push_back(k);
 for(auto &v:level)if(v.size()>8)return;
 auto update=[&](int k){for(int v=0;v<P;v++){uint64_t d=(A[v]^papers[k].color)&PrefixDP::low(papers[k].len);start[v][k]=d?64-__builtin_clzll(d):0;}};for(int k=0;k<K;k++)update(k);
 std::vector<std::array<int,65>>F(P),B(P);
 auto distances=[&](){double total=0;for(int v=0;v<P;v++){auto&f=F[v];auto&b=B[v];f.fill(INF);b.fill(INF);f[0]=0;b[N]=0;for(int l=1;l<=N;l++)for(int k:level[l])for(int j=start[v][k];j<l;j++)f[l]=std::min(f[l],f[j]+1);for(int l=N;l>0;l--)for(int k:level[l])for(int j=start[v][k];j<l;j++)b[j]=std::min(b[j],b[l]+1);total+=double(f[N])/P;}return total;};
 double initial=distances();int accepted=0,rejected=0;
 std::vector<int>sequence(P),length;std::iota(sequence.begin(),sequence.end(),0);std::mt19937 rng(seed);std::shuffle(sequence.begin(),sequence.end(),rng);for(auto p:papers)length.push_back(p.len);
 int evaluations=0;
 auto simulate=[&](int bound){
  PrefixDP machine(N,papers);std::vector<int>order(K);std::iota(order.begin(),order.end(),0);int total=0;
  for(int r=0;r<P;r++){
   if((r&7)==0&&double(clock()-time_origin)/CLOCKS_PER_SEC>cpu_limit)return 1000000000;
   int moves;auto chosen=machine.solve(order,A[sequence[r]],moves);std::vector<char>selected(K);for(int k:chosen)selected[k]=true;
   total+=moves;if(total>=bound)return total;
   for(int i=0;i<moves;i++){auto[from,to]=GlobalPrefix::best_move(order,length,selected);if(from<0)throw std::runtime_error("simrefit internal operation stalled");int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);}
  }
  return total;
 };
 int best_score=simulate(1000000000),original_score=best_score;
 if(best_score>=1000000000)return;
 struct Node{int child[2]{-1,-1},weight=0,sample=0;std::array<int,9>dp,split;Node(){dp.fill(-1000000000);dp[0]=0;split.fill(0);}};
 for(int l=N;l>=minimum_length;l--){
  if(double(clock()-time_origin)/CLOCKS_PER_SEC>cpu_limit)break;
  int amount=level[l].size();if(!amount)continue;distances();std::vector<Node>trie(1);
  for(int v=0;v<P;v++){
   auto f=F[v];f[l]=INF;for(int upper=l+1;upper<=N;upper++){f[upper]=INF;for(int k:level[upper])for(int j=start[v][k];j<upper;j++)f[upper]=std::min(f[upper],f[j]+1);}
   int bypass=f[N],bestprefix=INF,previous=0,x=0;
   for(int d=1;d<=l;d++){
    int bit=A[v]>>(l-d)&1;if(trie[x].child[bit]<0){int ni=trie.size();trie[x].child[bit]=ni;trie.emplace_back();}x=trie[x].child[bit];trie[x].sample=v;
    bestprefix=std::min(bestprefix,F[v][l-d]);int route=bestprefix+1+B[v][l];int benefit=0;
    if(route<INF){if(bypass>=INF)benefit=1000000+importance*(N+1-route);else benefit=importance*std::max(0,bypass-route);}
    trie[x].weight+=benefit-previous+1;previous=benefit;
   }
  }
  for(int x=int(trie.size())-1;x>=0;x--){auto&p=trie[x];int a=p.child[0],b=p.child[1];if(a<0&&b<0){p.dp[1]=p.weight;continue;}
   for(int q=1;q<=amount;q++)for(int left=0;left<=q;left++){int right=q-left;if((a<0&&left)||(b<0&&right))continue;int value=(a<0?0:trie[a].dp[left])+(b<0?0:trie[b].dp[right])+p.weight;if(value>p.dp[q])p.dp[q]=value,p.split[q]=left;}
  }
  if(trie[0].dp[amount]<0){rejected++;continue;}
  std::vector<uint64_t>pattern;std::function<void(int,int)>collect=[&](int x,int q){if(!q)return;auto&p=trie[x];if(p.child[0]<0&&p.child[1]<0){pattern.push_back(A[p.sample]&PrefixDP::low(l));return;}int left=p.split[q];if(left)collect(p.child[0],left);if(q-left)collect(p.child[1],q-left);};collect(0,amount);
  bool valid=true;for(int v=0;v<P&&valid;v++){auto f=F[v];f[l]=INF;for(uint64_t c:pattern){uint64_t d=(c^A[v])&PrefixDP::low(l);int st=d?64-__builtin_clzll(d):0;for(int j=st;j<l;j++)f[l]=std::min(f[l],f[j]+1);}for(int upper=l+1;upper<=N;upper++){f[upper]=INF;for(int k:level[upper])for(int j=start[v][k];j<upper;j++)f[upper]=std::min(f[upper],f[j]+1);}if(f[N]>=INF)valid=false;}
  if(!valid){rejected++;continue;}
  std::vector<uint64_t>old;bool changed=false;for(int q=0;q<amount;q++){int k=level[l][q];old.push_back(papers[k].color);changed|=old.back()!=pattern[q];papers[k].color=pattern[q];}
  if(!changed)continue;
  int score=simulate(best_score-margin+1);evaluations++;
  if(score+margin<=best_score&&score<best_score){best_score=score;for(int k:level[l])update(k);accepted++;}
  else{for(int q=0;q<amount;q++)papers[level[l][q]].color=old[q];rejected++;}
 }
 std::cerr<<"simrefit intrinsic="<<initial<<"->"<<distances()<<" rollout="<<original_score<<"->"<<best_score<<" evaluations="<<evaluations<<" accepted="<<accepted<<" rejected="<<rejected<<" CPU="<<double(clock()-time_origin)/CLOCKS_PER_SEC<<"\n";
}

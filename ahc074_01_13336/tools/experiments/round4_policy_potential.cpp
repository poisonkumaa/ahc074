// AHC074 v7: input-specific portfolio and all-catalogue access model.
// Standalone C++17. Only initial barcodes are used in preprocessing.
// Real orders are read one at a time, with an explicit flush after each answer.
// The attached main(2).cpp supplies the prefix-DP and legal shortest-path core.
// New: four design variants, 256-product access masks, variance-reduced
// Monte Carlo model selection, and a portable inlined population count.
// No external files, future-order reads, or command-line arguments are used.
#ifdef LOCAL_DIAGNOSTICS
static constexpr bool DIAGNOSTICS=true;
#else
static constexpr bool DIAGNOSTICS=false;
#endif
// AHC074: public-data prefix dictionaries, exact legal reactive paths.
// Train only on the initial 256 barcodes. The 64-bit access model is initialized
// with internally generated requests, then updated only from processed orders.
// Each query minimizes its move count exactly; both goal and move tie-breakers
// favor papers that are likely to be useful again. No future order is read.
#pragma GCC optimize("O3")
#include <bits/stdc++.h>
using namespace std;using U64=uint64_t;
int BASE_MODE=0;


#ifndef WORDS
#define WORDS 4
#endif
struct Access {
 array<uint64_t,WORDS> w{};
 Access(uint64_t v=0){w[0]=v;}
 static Access bit(int i){Access x;x.w[(i>>6)%WORDS]=uint64_t(1)<<(i&63);return x;}
 Access& operator|=(const Access&a){for(int i=0;i<WORDS;i++)w[i]|=a.w[i];return *this;}
 friend Access operator|(Access a,const Access&b){return a|=b;}
 friend Access operator&(const Access&a,const Access&b){Access c;for(int i=0;i<WORDS;i++)c.w[i]=a.w[i]&b.w[i];return c;}
 friend Access operator~(Access a){for(auto&x:a.w)x=~x;return a;}
};
inline int maskpop(uint64_t x){x-= (x>>1)&0x5555555555555555ULL;x=(x&0x3333333333333333ULL)+((x>>2)&0x3333333333333333ULL);x=(x+(x>>4))&0x0f0f0f0f0f0f0f0fULL;return (x*0x0101010101010101ULL)>>56;}
inline int maskpop(const Access&a){int n=0;for(auto x:a.w)n+=maskpop(x);return n;}

struct PrefixPaper {int len;U64 color;};
struct PrefixDP {
 int n,k,epoch=0,primary=65536;vector<PrefixPaper>a;vector<array<int,193>>cnt,work,parent,which,stamp,opcount;int pos[192];vector<int>reward;
 vector<array<vector<uint8_t>,64>>candidate;unordered_map<U64,int>product_index;
 PrefixDP(int nn,vector<PrefixPaper>aa):n(nn),k(aa.size()),a(move(aa)),cnt(n+2),work(n+1),parent(n+1),which(n+1),stamp(n+1),opcount(n+1),reward(k,16){}
 static U64 low(int n){return n==64?~0ULL:(1ULL<<n)-1;}
 void public_data(const vector<U64>&A){candidate.resize(A.size());for(int v=0;v<int(A.size());v++){product_index[A[v]]=v;for(int z=0;z<k;z++){U64 diff=(A[v]^a[z].color)&low(a[z].len);int start=diff?64-__builtin_clzll(diff):0;for(int j=start;j<a[z].len;j++)candidate[v][j].push_back(z);}}}
 void prepare(const vector<int>&p){for(int i=0;i<k;i++)pos[p[i]]=i;for(int l=0;l<=n+1;l++){cnt[l][0]=0;for(int i=0;i<k;i++)cnt[l][i+1]=cnt[l][i]+(a[p[i]].len>=l);}}
 vector<int>solve(const vector<int>&p,U64 target,int&best,bool reconstruct=true){prepare(p);return solve_prepared(target,best,reconstruct);}
 vector<int>solve_prepared(U64 target,int&best,bool reconstruct=true){
  auto&dp=work;epoch++;int active[65][193],count[65]{};dp[0][0]=0;opcount[0][0]=0;stamp[0][0]=epoch;active[0][count[0]++]=0;const auto&all=candidate[product_index.at(target)];
  for(int j=0;j<n;j++){
   int ac=count[j];if(!ac)continue;if(reconstruct)sort(active[j],active[j]+ac);
   for(int z:all[j]){int l=a[z].len,q=pos[z];
    for(int ai=0;ai<ac;ai++){int h=active[j][ai],add=0;if(q>=h){int moved=cnt[l][q]-cnt[l][h],upper=cnt[j+1][q]-cnt[j+1][h];add=moved+(upper>moved);}int nh=max(q+1,h),value=dp[j][h]+add*primary-reward[z];
     if(stamp[l][nh]!=epoch){stamp[l][nh]=epoch;dp[l][nh]=1000000000;active[l][count[l]++]=nh;}
     if(value<dp[l][nh]){dp[l][nh]=value;opcount[l][nh]=opcount[j][h]+add;if(reconstruct){parent[l][nh]=j*(k+1)+h;which[l][nh]=z;}}
    }
   }
  }
  int h=active[n][0],j=n;for(int i=1;i<count[n];i++){int x=active[n][i];if(dp[n][x]<dp[n][h]||(dp[n][x]==dp[n][h]&&x<h))h=x;}best=opcount[n][h];vector<int>ans;if(!reconstruct)return ans;
  while(j){int z=which[j][h],prev=parent[j][h];ans.push_back(z);j=prev/(k+1);h=prev%(k+1);}reverse(ans.begin(),ans.end());return ans;
 }
};

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>

// All papers have opaque prefix [0,length[k]).  Goal contains exactly one
// visible paper at each chosen length, and includes a maximum-length paper.
// Colors are immaterial after this desired visible-record set is fixed.
namespace GlobalPrefix {

inline int fixed_distance(const std::vector<int>& order,
                          const std::vector<int>& length,
                          const std::vector<char>& selected) {
    const int K = order.size();
    const int N = *std::max_element(length.begin(), length.end());
    std::vector<int> owner(N+2,-1), pos(K), next(K), bad(K);
    std::vector<char> moved(K);
    for (int k=0;k<K;k++) if(selected[k]) owner[length[k]]=k;
    int root=-1;
    for(int l=N;l>=0;l--) { if(owner[l]>=0)root=owner[l]; owner[l]=root; }
    for(int k=0;k<K;k++) next[k]=owner[length[k]];
    int minimum=N+1, answer=0;
    for(int i=K-1;i>=0;i--) {
        int k=order[i]; pos[k]=i;
        moved[k]=length[k]>=minimum; answer+=moved[k];
        if(selected[k]) minimum=std::min(minimum,length[k]);
    }
    for(int k=0;k<K;k++) if(!selected[k]&&!moved[k]&&pos[k]<pos[next[k]]) bad[next[k]]++;
    for(int k=0;k<K;k++) if(selected[k]&&!moved[k]&&bad[k]) answer++;
    return answer;
}

// Returns (old position, insertion position after removal), or (-1,-1) if
// the goal is already reached.  Every returned move is legal and lowers
// fixed_distance by exactly one.  Enumerating all positions is O(K * V),
// where V is the number of currently visible papers; distance updates are O(1).
// Among shortest-path moves minimize the number of mandatory removals averaged
// over 64 previous goals. A suffix union counts correlated choices exactly.
// Length inversions break remaining ties and initialize the access model.
inline std::pair<int,int> best_move(const std::vector<int>& order,
                                  const std::vector<int>& length,
                                  const std::vector<char>& selected,
                                  double decay=.5, const std::vector<Access>* access=nullptr) {
    const int K=order.size();
    const int N=*std::max_element(length.begin(),length.end());
    std::array<int,66> owner;owner.fill(-1);std::array<int,192> next,pos;std::array<int,193> suffix;suffix.fill(N+1);
    std::array<int,192> initial_bad{},inversions{};
    std::array<char,192> initial_moved{},initial_before{};
    std::array<double,193> power;power[0]=1.;
    for(int i=1;i<=K;i++)power[i]=power[i-1]*decay;
    for(int k=0;k<K;k++)if(selected[k])owner[length[k]]=k;
    int root=-1;
    for(int l=N;l>=0;l--){if(owner[l]>=0)root=owner[l];owner[l]=root;}
    for(int k=0;k<K;k++)next[k]=owner[length[k]];
    for(int i=0;i<K;i++)pos[order[i]]=i;
    int initial_distance=0;
    for(int i=K-1;i>=0;i--){
        int k=order[i];initial_moved[k]=length[k]>=suffix[i+1];
        initial_distance+=initial_moved[k];
        suffix[i]=std::min(suffix[i+1],selected[k]?length[k]:N+1);
    }
    for(int k=0;k<K;k++)if(!selected[k]){
        initial_before[k]=pos[k]<pos[next[k]];
        if(initial_before[k]&&!initial_moved[k])initial_bad[next[k]]++;
    }
    for(int k=0;k<K;k++)if(selected[k]&&!initial_moved[k]&&initial_bad[k])initial_distance++;
    if(initial_distance==0)return {-1,-1};
    Access below[192]{}, mask_tree[65]{};
    if(access){for(int i=K-1;i>=0;i--){int k=order[i];for(int x=length[k];x>0;x-=x&-x)below[k]|=mask_tree[x];for(int x=length[k];x<=N;x+=x&-x)mask_tree[x]|=(*access)[k];}}
    double initial_quality=0;
    int initial_inversions=0;
    std::array<int,66> fenwick{};
    for(int i=K-1;i>=0;i--){
        int k=order[i];
        for(int x=length[k]-1;x>0;x-=x&-x)inversions[k]+=fenwick[x];
        for(int x=length[k];x<=N;x+=x&-x)fenwick[x]++;
        if(access)initial_quality+=maskpop(below[k]);
        else initial_quality+=1-power[inversions[k]];
        initial_inversions+=inversions[k];
    }
    std::pair<int,int> answer{-1,-1};double best_quality=std::numeric_limits<double>::infinity();
    int best_inversions=std::numeric_limits<int>::max(), high=0;
    std::array<char,192> moved,before;std::array<int,192>bad;
    for(int i=0;i<K;i++){
        int k=order[i],kl=length[k];if(kl<=high)continue;high=kl;
        if(!initial_moved[k]) {
            if(selected[k]) { if(!initial_bad[k])continue; }
            else if(!initial_before[k]||initial_moved[next[k]]||initial_bad[next[k]]!=1)continue;
        }
        // Mandatory papers must move downward.  An optional chosen record
        // moves upward over its uncovered interval; the sole optional leaf
        // can instead move downward below that chosen record.
        int direction=initial_moved[k]||!selected[k]?1:-1;
        Access remaining[193];if(access&&direction==1){remaining[K]=0;for(int d=K-1;d>i;d--){int x=order[d];remaining[d]=remaining[d+1]|(length[x]<=kl?(*access)[x]:Access(0));}}
        {
            moved=initial_moved;before=initial_before;bad=initial_bad;
            int distance=initial_distance, invk=inversions[k], invsum=initial_inversions;
            int following_min=suffix[i+1];double quality=initial_quality;Access own=0;int own_count=0;
            if(access){own=below[k];own_count=maskpop(own);}
            auto extra=[&](int group){return !moved[group]&&bad[group]>0;};
            auto change_moved=[&](int x,bool value){
                if(bool(moved[x])==value)return;
                int group=next[x];distance-=extra(group);
                distance+=int(value)-int(moved[x]);
                if(!selected[x]&&before[x])bad[group]+=int(moved[x])-int(value);
                moved[x]=value;distance+=extra(group);
            };
            auto change_before=[&](int x,bool value){
                if(bool(before[x])==value)return;
                int group=next[x];distance-=extra(group);
                if(!moved[x])bad[group]+=int(value)-int(before[x]);
                before[x]=value;distance+=extra(group);
            };
            for(int d=i+direction;d>=0&&d<K;d+=direction){
                int x=order[d],xl=length[x],old_invk=invk;
                if(direction==1){
                    if(selected[k]&&xl>=kl)change_moved(x,true);
                    change_moved(k,kl>=suffix[d+1]);
                    if(!selected[x]&&next[x]==k)change_before(x,true);
                    if(!selected[k]&&next[k]==x)change_before(k,false);
                    if(xl<kl){invk--;invsum--;}else if(xl>kl){if(!access)quality+=power[inversions[x]]-power[inversions[x]+1];invsum++;}
                    if(access){
                     int next_count=maskpop(remaining[d+1]);quality+=next_count-own_count;own_count=next_count;
                     if(xl>=kl)quality+=maskpop((*access)[k]&~below[x]);
                    }
                }else{
                    if(selected[k]&&xl>=kl)change_moved(x,xl>=following_min);
                    if(selected[x]&&kl>=xl)change_moved(k,true);
                    if(!selected[x]&&next[x]==k)change_before(x,false);
                    if(!selected[k]&&next[k]==x)change_before(k,true);
                    if(selected[x])following_min=std::min(following_min,xl);
                    if(xl<kl){invk++;invsum++;}else if(xl>kl){if(!access)quality-=power[inversions[x]-1]-power[inversions[x]];invsum--;}
                    if(access){
                     own|=(*access)[x];int next_count=maskpop(own);quality+=next_count-own_count;own_count=next_count;
                    }
                }
                if(!access)quality+=power[old_invk]-power[invk];
                if(distance!=initial_distance-1)continue;
                if(quality<best_quality-1e-10 ||
                   (std::abs(quality-best_quality)<=1e-10&&invsum<best_inversions)){
                    best_quality=quality;best_inversions=invsum;answer={i,d};
                }
            }
        }
        if(answer.first>=0)break;
    }
    return answer;
}
}


#ifndef STRUCTURE2_TRAINING_CPU_LIMIT
#define STRUCTURE2_TRAINING_CPU_LIMIT 0.75
#endif
static clock_t structure2_training_origin=0;

inline void train_backbone_dictionary(const std::vector<uint64_t>& A,std::vector<PrefixPaper>&papers,int n,int perlevel,int passes,int importance){
 const int P=A.size(),K=192,INF=1000;
 U64 background=~0ULL;
 if(BASE_MODE==1)background=0;
 if(BASE_MODE==2){background=0;for(int j=0;j<n;j++){int cnt=0;for(auto v:A)cnt+=v>>j&1;if(cnt>=P/2)background|=1ULL<<j;}}
 if(BASE_MODE>=3){mt19937_64 br(BASE_MODE*19237);background=br();}
 papers.clear();for(int l=1;l<=n;l++)papers.push_back({l,(background&PrefixDP::low(l))^(1ULL<<(l-1))});papers.push_back({n,background&PrefixDP::low(n)});int fixed=papers.size();papers.resize(K,{0,0});
 std::vector<std::vector<int>>level(n+1),extra(n+1);std::vector<int>spare;
 for(int k=0;k<K;k++)if(k<fixed)level[papers[k].len].push_back(k);else spare.push_back(k);
 std::vector<std::array<int,192>> start(P);std::vector<std::array<int,65>>fw(P),bw(P);
 auto update=[&](int k){int l=papers[k].len;for(int v=0;v<P;v++){uint64_t d=(A[v]^papers[k].color)&PrefixDP::low(l);start[v][k]=d?64-__builtin_clzll(d):0;}};
 for(int k=0;k<K;k++)update(k);
 auto distances=[&](){
  int total=0;for(int v=0;v<P;v++){
   auto&f=fw[v];auto&b=bw[v];f.fill(INF);b.fill(INF);f[0]=0;b[n]=0;
   for(int l=1;l<=n;l++)for(int k:level[l])for(int j=start[v][k];j<l;j++)f[l]=std::min(f[l],f[j]+1);
   for(int l=n;l>0;l--)for(int k:level[l])for(int j=start[v][k];j<l;j++)b[j]=std::min(b[j],b[l]+1);
   total+=f[n];
  }return double(total)/P;
 };
 double initial=distances();
 struct Node{int child[2]{-1,-1},weight=0,sample=0;std::array<int,17>dp,split;Node(){dp.fill(-100000000);dp[0]=0;split.fill(0);}};
 for(int pass=0;pass<passes;pass++)for(int l=n;l>0;l--){if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>STRUCTURE2_TRAINING_CPU_LIMIT)break;
  int amount=std::min(perlevel,int(extra[l].size()+spare.size()));if(!amount)continue;distances();std::vector<Node>trie(1);
  for(int v=0;v<P;v++){
   // A route avoiding l crosses it once, along an edge j < l < upper.
   // The prefix and suffix distances on that edge cannot use level l.
   int prefix_min[65];prefix_min[l]=INF;for(int j=l-1;j>=0;j--)prefix_min[j]=std::min(prefix_min[j+1],fw[v][j]);
   int bypass=INF;for(int u=l+1;u<=n;u++)for(int k:level[u])if(start[v][k]<l)bypass=std::min(bypass,prefix_min[start[v][k]]+1+bw[v][u]);
   for(int k:level[l])if(k<fixed&&start[v][k]<l)bypass=std::min(bypass,prefix_min[start[v][k]]+1+bw[v][l]);
   int node=0,bestprefix=INF,prev=0;
   for(int d=1;d<=l;d++){
    int b=A[v]>>(l-d)&1;if(trie[node].child[b]<0){int next=trie.size();trie[node].child[b]=next;trie.emplace_back();}node=trie[node].child[b];trie[node].sample=v;
    bestprefix=std::min(bestprefix,fw[v][l-d]);int benefit=std::max(0,bypass-(bestprefix+1+bw[v][l]));trie[node].weight+=importance*(benefit-prev)+1;prev=benefit;
   }
  }
  for(int node=int(trie.size())-1;node>=0;node--){auto&v=trie[node];int a=v.child[0],b=v.child[1];if(a<0&&b<0){v.dp[1]=v.weight;continue;}
   for(int q=1;q<=amount;q++)for(int left=0;left<=q;left++){int right=q-left;if((a<0&&left)||(b<0&&right))continue;int value=(a<0?0:trie[a].dp[left])+(b<0?0:trie[b].dp[right])+v.weight;if(value>v.dp[q])v.dp[q]=value,v.split[q]=left;}
  }
  int use=amount;while(use>1&&trie[0].dp[use]<0)use--;std::vector<uint64_t>pattern;
  std::function<void(int,int)>collect=[&](int node,int count){if(!count)return;auto&v=trie[node];if(v.child[0]<0&&v.child[1]<0){pattern.push_back(A[v.sample]&PrefixDP::low(l));return;}int left=v.split[count];if(left)collect(v.child[0],left);if(count-left)collect(v.child[1],count-left);};collect(0,use);
  while(extra[l].size()<pattern.size()){int k=spare.back();spare.pop_back();extra[l].push_back(k);level[l].push_back(k);papers[k].len=l;}
  for(int q=0;q<int(pattern.size());q++){int k=extra[l][q];papers[k].color=pattern[q];update(k);}
 }
 if(DIAGNOSTICS)std::cerr<<"backbone "<<initial<<" -> "<<distances()<<" papers="<<K-spare.size()<<" CPU="<<double(clock())/CLOCKS_PER_SEC<<"\n";
 std::vector<PrefixPaper>p;for(auto x:papers)if(x.len)p.push_back(x);std::stable_sort(p.begin(),p.end(),[](auto a,auto b){return a.len<b.len;});papers=p;
}


inline void refit_all_prefix_papers(const std::vector<uint64_t>&A,std::vector<PrefixPaper>&papers,int passes,int importance){
 const int P=A.size(),N=64,K=papers.size(),INF=1000;std::vector<std::vector<int>>level(65),start(P,std::vector<int>(K));
 for(int k=0;k<K;k++)level[papers[k].len].push_back(k);
 auto update=[&](int k){for(int v=0;v<P;v++){uint64_t d=(A[v]^papers[k].color)&PrefixDP::low(papers[k].len);start[v][k]=d?64-__builtin_clzll(d):0;}};for(int k=0;k<K;k++)update(k);
 std::vector<std::array<int,65>>F(P),B(P);
 auto distances=[&](){double total=0;for(int v=0;v<P;v++){auto&f=F[v];auto&b=B[v];f.fill(INF);b.fill(INF);f[0]=0;b[N]=0;for(int l=1;l<=N;l++)for(int k:level[l])for(int j=start[v][k];j<l;j++)f[l]=std::min(f[l],f[j]+1);for(int l=N;l>0;l--)for(int k:level[l])for(int j=start[v][k];j<l;j++)b[j]=std::min(b[j],b[l]+1);total+=double(f[N])/P;}return total;};
 double initial=distances();int accepted=0,rejected=0;bool dirty=false;
 struct Node{int child[2]{-1,-1},weight=0,sample=0;std::array<int,17>dp,split;Node(){dp.fill(-1000000000);dp[0]=0;split.fill(0);}};
 for(int pass=0;pass<passes;pass++)for(int l=N;l>0;l--){if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>STRUCTURE2_TRAINING_CPU_LIMIT)break;
  int amount=level[l].size();if(!amount)continue;if(dirty){distances();dirty=false;}std::vector<Node>trie(1);std::vector<int>without_level(P);
  for(int v=0;v<P;v++){
   // A route avoiding l crosses it once, along an edge j < l < upper.
   // The prefix and suffix distances on that edge cannot use level l.
   int prefix_min[65];prefix_min[l]=INF;for(int j=l-1;j>=0;j--)prefix_min[j]=std::min(prefix_min[j+1],F[v][j]);
   int bypass=INF;for(int upper=l+1;upper<=N;upper++)for(int k:level[upper])if(start[v][k]<l)bypass=std::min(bypass,prefix_min[start[v][k]]+1+B[v][upper]);
   without_level[v]=bypass;int bestprefix=INF,previous=0,x=0;
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
  bool valid=true;for(int v=0;v<P&&valid;v++)if(without_level[v]>=INF){int route=INF;for(uint64_t c:pattern){uint64_t d=(c^A[v])&PrefixDP::low(l);int st=d?64-__builtin_clzll(d):0;for(int j=st;j<l;j++)route=std::min(route,F[v][j]+1+B[v][l]);}if(route>=INF)valid=false;}
  if(!valid){rejected++;continue;}vector<U64>oldpattern;for(int k:level[l])oldpattern.push_back(papers[k].color);auto sortedpattern=pattern;sort(oldpattern.begin(),oldpattern.end());sort(sortedpattern.begin(),sortedpattern.end());dirty=(oldpattern!=sortedpattern);for(int q=0;q<amount;q++){int k=level[l][q];papers[k].color=pattern[q];update(k);}accepted++;
 }
 if(DIAGNOSTICS)std::cerr<<"refit "<<initial<<" -> "<<distances()<<" accepted="<<accepted<<" rejected="<<rejected<<"\n";
}

#ifndef ENTROPY_WIDTH
#define ENTROPY_WIDTH 6
#endif
#ifndef ENTROPY_STEPS
#define ENTROPY_STEPS 10000
#endif
#ifndef ENTROPY_PAIR
#define ENTROPY_PAIR 2
#endif
// Optimize concentrations of 6-window patterns using reversal moves.
// Reversal preserves the scores of interior windows; only boundaries change.
vector<int> refine_columns(vector<int>cols,const vector<U64>&input,int diff[64][64],int steps=10000,int pair_weight=2){
 constexpr int W=ENTROPY_WIDTH;constexpr int STARTS=65-W;
 uint8_t bit[64][256];for(int j=0;j<64;j++)for(int a=0;a<256;a++)bit[j][a]=input[a]>>j&1;
 auto value=[&](int start){int freq[1<<W]{};for(int a=0;a<256;a++){int q=0;for(int d=0;d<W;d++)q|=int(bit[cols[start+d]][a])<<d;freq[q]++;}int z=0;for(int f:freq)z+=f*f;return z;};
 int score[STARTS],total=0;for(int j=0;j<STARTS;j++)total+=score[j]=value(j);
 for(int j=1;j<64;j++)total-=pair_weight*min(diff[cols[j-1]][cols[j]],256-diff[cols[j-1]][cols[j]]);
 auto best=cols;int bestscore=total;mt19937 rng(812347+cols[0]);
 for(int it=0;it<steps;it++){
  int x=rng()%64,y=rng()%64;if(x>y)swap(x,y);if(x==y)continue;
  vector<int>affected;
  for(int j=max(0,x-W+1);j<min(x,STARTS);j++)affected.push_back(j);
  for(int j=max(0,y-W+2);j<min(y+1,STARTS);j++)affected.push_back(j);
  sort(affected.begin(),affected.end());affected.erase(unique(affected.begin(),affected.end()),affected.end());
  int delta=0;for(int j:affected)delta-=score[j];
  auto edge=[&](int a,int b){return min(diff[a][b],256-diff[a][b]);};
  if(x)delta+=pair_weight*(edge(cols[x-1],cols[x])-edge(cols[x-1],cols[y]));
  if(y<63)delta+=pair_weight*(edge(cols[y],cols[y+1])-edge(cols[x],cols[y+1]));
  reverse(cols.begin()+x,cols.begin()+y+1);vector<int>newscore;
  for(int j:affected){int v=value(j);delta+=v;newscore.push_back(v);}
  double temp=80*pow(.003,double(it)/steps);
  if(delta>=0||double(rng())/4294967296.<exp(delta/temp)){
   // Windows entirely inside the reversed range have the same multiset of
   // patterns, but their positions are mirrored.
   if(y-x+1>=W)reverse(score+x,score+y-W+2);
   for(int i=0;i<(int)affected.size();i++)score[affected[i]]=newscore[i];
   total+=delta;if(total>bestscore){bestscore=total;best=cols;}
  }else reverse(cols.begin()+x,cols.begin()+y+1);
 }
 return best;
}



namespace GlobalPrefix {
inline std::vector<int> final_order(const std::vector<int>&order,const std::vector<int>&length,const std::vector<char>&selected,const std::vector<Access>*future=nullptr){
 using namespace std;
 int K=order.size(),N=*max_element(length.begin(),length.end()),owner[65],root=-1;fill(owner,owner+65,-1);for(int k=0;k<K;k++)if(selected[k])owner[length[k]]=k;for(int l=N;l>=0;l--){if(owner[l]>=0)root=owner[l];owner[l]=root;}
 vector<char>mandatory(K);int suffix=N+1;for(int i=K-1;i>=0;i--){int k=order[i];mandatory[k]=length[k]>=suffix;if(selected[k])suffix=min(suffix,length[k]);}
 vector<vector<int>>children(K);vector<int>insert;int pos[192];for(int i=0;i<K;i++){int k=order[i];pos[k]=i;if(selected[k])continue;if(mandatory[k])insert.push_back(k);else children[owner[length[k]]].push_back(k);}
 stable_sort(insert.begin(),insert.end(),[&](int a,int b){return length[a]<length[b];});
 double power[193];power[0]=1;for(int i=1;i<=K;i++)power[i]=power[i-1]*.5;
 for(int k:insert){auto&v=children[owner[length[k]]];int M=v.size(),kl=length[k];Access below[192]{},remain[193]{};int inversions[192]{},lower_after[193]{};
  for(int i=M-1;i>=0;i--){int x=v[i];for(int j=i+1;j<M;j++){int y=v[j];if(length[y]<length[x])inversions[x]++;if(future&&length[y]<=length[x])below[x]|=(*future)[y];}remain[i]=remain[i+1]|(future&&length[x]<=kl?(*future)[x]:Access(0));lower_after[i]=lower_after[i+1]+(length[x]<kl);}
  double gain=0,best=1e100;int larger=0,bestinv=INT_MAX,at=0;
  for(int i=0;i<=M;i++){double q=gain+(future?double(maskpop(remain[i])):1-power[lower_after[i]]);int inv=larger+lower_after[i];if(q<best-1e-10||(abs(q-best)<=1e-10&&inv<bestinv)){best=q;bestinv=inv;at=i;}
   if(i<M){int x=v[i];if(length[x]>kl)larger++;if(future){if(length[x]>=kl)gain+=maskpop((*future)[k]&~below[x]);}else if(length[x]>kl)gain+=power[inversions[x]]*.5;}
  }
  v.insert(v.begin()+at,k);
 }
 vector<int>out;out.reserve(K);for(int l=1;l<=N;l++){int k=owner[l];if(k>=0&&selected[k]&&length[k]==l){out.push_back(k);for(int x:children[k])out.push_back(x);}}return out;
}
}


#ifndef R4_MAXC
#define R4_MAXC 4
#endif
#ifndef R4_PASSES
#define R4_PASSES 1
#endif
#ifndef R4_PILOT
#define R4_PILOT 544
#endif
#ifndef R4_BUDGET
#define R4_BUDGET 1.15
#endif
#ifndef R4_FINALIST
#define R4_FINALIST 800
#endif

struct Candidate {
 vector<int>cols,flip;vector<U64>target;vector<PrefixPaper>paper;vector<int>order,length;vector<Access>future;unique_ptr<PrefixDP>dp;
 
 vector<double>f1,f2;double mean1=0,mean2=0,sum1=0,sum2=0,sq1=0,sq2=0,cross=0,m1=0,m2=0;
 int count=0,score=0,variant=0;
 double estimate()const{
  double n=max(1,count-32),y=score/n,x1=sum1/n,x2=sum2/n;
  double c1=m1/n-y*x1,c2=m2/n-y*x2;
  double v1=sq1/n-x1*x1+.2,v2=sq2/n-x2*x2+.2,co=cross/n-x1*x2,det=v1*v2-co*co;
  double b1=(c1*v2-c2*co)/det,b2=(c2*v1-c1*co)/det;
  return y+b1*(mean1-x1)+b2*(mean2-x2);
 }

};
struct PotentialPlanner {
 PrefixDP&base;const vector<PrefixPaper>&paper;int N,K,epoch=0,cache_epoch=0,last_phi=0;
 vector<array<long long,193>>work;vector<array<int,193>>stamp,parent,which,moves_sum,phi_sum;
 vector<vector<vector<int>>>interval;int cache_stamp[193*193]{},cache_phi[193*193]{};
 Access below[192];int inv[192];
 PotentialPlanner(PrefixDP&b):base(b),paper(b.a),N(b.n),K(b.k),work(N+1),stamp(N+1),parent(N+1),which(N+1),moves_sum(N+1),phi_sum(N+1),interval(N+1,vector<vector<int>>(N+1)){
  for(int j=0;j<N;j++)for(int l=j+1;l<=N;l++)for(int k=0;k<K;k++)if(j<paper[k].len&&paper[k].len<=l)interval[j][l].push_back(k);
 }
 int group_cost(const vector<int>&members,int length,int h,int q,const vector<Access>&future){
  int size=members.size(),v[192],missing[192],vc=0,mc=0;Access all;
  for(int i=0;i<size;i++){int k=members[i];below[i]=0;inv[i]=0;all|=future[k];bool moved=base.pos[k]<(paper[k].len==length?max(h,q+1):h);if(moved)missing[mc++]=i;else v[vc++]=i;}
  int total=maskpop(all);
  for(int i=vc-1;i>=0;i--){int x=v[i],k=members[x];for(int j=i+1;j<vc;j++){int y=v[j],z=members[y];if(paper[z].len<=paper[k].len)below[x]|=future[z];if(paper[z].len<paper[k].len)inv[x]++;}total+=maskpop(below[x]);}
  stable_sort(missing,missing+mc,[&](int a,int b){return paper[members[a]].len<paper[members[b]].len;});
  for(int mi=0;mi<mc;mi++){
   int x=missing[mi],k=members[x],kl=paper[k].len;Access remaining[193];int lower[193];remaining[vc]=0;lower[vc]=0;
   for(int i=vc-1;i>=0;i--){int z=members[v[i]];remaining[i]=remaining[i+1]|(paper[z].len<=kl?future[z]:Access(0));lower[i]=lower[i+1]+(paper[z].len<kl);}
   int gain=0,larger=0,best=INT_MAX,bestinv=INT_MAX,at=0;
   for(int i=0;i<=vc;i++){int cost=gain+maskpop(remaining[i]),inversions=larger+lower[i];if(cost<best||(cost==best&&inversions<bestinv)){best=cost;bestinv=inversions;at=i;}
    if(i<vc){int y=v[i],z=members[y];if(paper[z].len>kl)larger++;if(paper[z].len>=kl)gain+=maskpop(future[k]&~below[y]);}
   }
   total+=best;below[x]=remaining[at];inv[x]=lower[at];for(int i=0;i<at;i++){int y=v[i],z=members[y];if(paper[z].len>=kl)below[y]|=future[k];if(paper[z].len>kl)inv[y]++;}
   for(int i=vc;i>at;i--)v[i]=v[i-1];v[at]=x;vc++;
  }
  return total;
 }
 vector<int>solve(const vector<int>&order,U64 target,const vector<Access>&future,int weight,int&moves){
  base.prepare(order);epoch++;int active[65][193],count[65]{};work[0][0]=0;stamp[0][0]=epoch;moves_sum[0][0]=phi_sum[0][0]=0;active[0][count[0]++]=0;
  const auto&all=base.candidate[base.product_index.at(target)];
  for(int j=0;j<N;j++){
   int ac=count[j];if(!ac)continue;sort(active[j],active[j]+ac);
   for(int z:all[j]){
    int l=paper[z].len,q=base.pos[z],lower_pos[192],same_pos[192],lc=0,sc=0;vector<int>members;members.reserve(interval[j][l].size());
    for(int k:interval[j][l])if(k!=z)members.push_back(k);sort(members.begin(),members.end(),[&](int a,int b){return base.pos[a]<base.pos[b];});
    for(int k:members)if(paper[k].len==l)same_pos[sc++]=base.pos[k];else lower_pos[lc++]=base.pos[k];
    int tag=++cache_epoch;
    for(int ai=0;ai<ac;ai++){
     int h=active[j][ai],add=0;if(q>=h){int removed=base.cnt[l][q]-base.cnt[l][h],upper=base.cnt[j+1][q]-base.cnt[j+1][h];add=removed+(upper>removed);}
     int nh=max(q+1,h),lowcut=lower_bound(lower_pos,lower_pos+lc,h)-lower_pos,samecut=lower_bound(same_pos,same_pos+sc,nh)-same_pos,key=lowcut*193+samecut;
     int phi;if(cache_stamp[key]==tag)phi=cache_phi[key];else{phi=group_cost(members,l,h,q,future);cache_stamp[key]=tag;cache_phi[key]=phi;}
     long long value=work[j][h]+add*1000000000000LL+1LL*weight*phi-base.reward[z];
     if(stamp[l][nh]!=epoch){stamp[l][nh]=epoch;work[l][nh]=LLONG_MAX;active[l][count[l]++]=nh;}
     if(value<work[l][nh]){work[l][nh]=value;moves_sum[l][nh]=moves_sum[j][h]+add;phi_sum[l][nh]=phi_sum[j][h]+phi;parent[l][nh]=j*193+h;which[l][nh]=z;}
    }
   }
  }
  int h=active[N][0],j=N;for(int i=1;i<count[N];i++){int x=active[N][i];if(work[N][x]<work[N][h]||(work[N][x]==work[N][h]&&x<h))h=x;}
  moves=moves_sum[N][h];last_phi=phi_sum[N][h];vector<int>goal;while(j){goal.push_back(which[j][h]);int prev=parent[j][h];j=prev/193;h=prev%193;}reverse(goal.begin(),goal.end());return goal;
 }
};
int total_potential(const vector<int>&order,const vector<int>&length,const vector<Access>&future){Access tree[65];int total=0;for(int i=int(order.size())-1;i>=0;i--){int k=order[i];Access u;for(int x=length[k];x>0;x-=x&-x)u|=tree[x];total+=maskpop(u);for(int x=length[k];x<=64;x+=x&-x)tree[x]|=future[k];}return total;}

int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int N,K,P,R;if(!(cin>>N>>K>>P>>R)||N!=64||K!=192||P!=256)return 0;
 vector<U64>input(P);for(auto&v:input){string row;cin>>row;if(row.size()!=64)return 0;for(int j=0;j<N;j++)if(row[j]=='1')v|=1ULL<<j;}
 clock_t total_origin=clock();auto elapsed=[&](){return double(clock()-total_origin)/CLOCKS_PER_SEC;};
 constexpr int MAXC=1, PASSES=4, PILOT=544;
 constexpr double BUDGET=R4_BUDGET;
 int correlation[64][64]{};for(int x=0;x<N;x++)for(int y=0;y<N;y++)for(auto v:input)correlation[x][y]+=((v>>x)^(v>>y))&1;
 vector<int>cols(64);iota(cols.begin(),cols.end(),0);mt19937_64 rng(12345);
 auto edge=[&](int x,int y){int d=correlation[x][y];return min(d,P-d);};
 int cur=0;for(int j=1;j<N;j++)cur+=edge(cols[j-1],cols[j]);int best=cur;auto bestcols=cols;
 for(int it=0;it<300000;it++){
  int x=rng()%64,y=rng()%64;if(x>y)swap(x,y);(void)(rng()%3);int next=cur;if(x)next+=edge(cols[x-1],cols[y])-edge(cols[x-1],cols[x]);if(y<63)next+=edge(cols[x],cols[y+1])-edge(cols[y],cols[y+1]);
  reverse(cols.begin()+x,cols.begin()+y+1);double temp=8*pow(.002,double(it)/300000);
  if(next<=cur||double(rng()>>11)*0x1p-53<exp((cur-next)/temp)){cur=next;if(cur<best)best=cur,bestcols=cols;}else reverse(cols.begin()+x,cols.begin()+y+1);
 }
 vector<vector<int>>column_variants;column_variants.push_back(bestcols);
 auto refined=bestcols;column_variants.push_back(refined);
 reverse(bestcols.begin(),bestcols.end());column_variants.push_back(bestcols);
 reverse(refined.begin(),refined.end());column_variants.push_back(refined);
 mt19937 trng(983212);vector<int>training;
 for(int r=0;r<4096;r++){int a=trng()%(r?255:256);if(r&&a>=training.back())a++;training.push_back(a);}
 auto simulate=[&](Candidate&c,int until){
  for(;c.count<until;c.count++){
   int a=training[c.count];for(int k=0;k<int(c.paper.size());k++)c.dp->reward[k]=16+2*maskpop(c.future[k]);int m;auto chosen=c.dp->solve(c.order,c.target[a],m);vector<char>selected(c.paper.size());for(int k:chosen)selected[k]=true;
   c.order=GlobalPrefix::final_order(c.order,c.length,selected,&c.future);
   Access bit=Access::bit(a);for(int k=0;k<int(c.paper.size());k++)c.future[k]=(c.future[k]&~bit)|(selected[k]?bit:Access(0));
   if(c.count>=32){c.score+=m;double x=c.f1[a],y=c.f2[a];c.sum1+=x;c.sum2+=y;c.sq1+=x*x;c.sq2+=y*y;c.cross+=x*y;c.m1+=m*x;c.m2+=m*y;}
  }
 };
 auto build=[&](int variant){
  Candidate c;c.variant=variant;c.cols=column_variants[(variant/2)%column_variants.size()];c.flip.assign(N,0);for(int j=1;j<N;j++)c.flip[j]=c.flip[j-1]^(correlation[c.cols[j-1]][c.cols[j]]>P/2);
  c.target.assign(P,0);for(int a=0;a<P;a++)for(int j=0;j<N;j++)c.target[a]|=(((input[a]>>c.cols[j])&1)^c.flip[j])<<j;
  BASE_MODE=variant%2;structure2_training_origin=clock();train_backbone_dictionary(c.target,c.paper,N,6,1,8);refit_all_prefix_papers(c.target,c.paper,PASSES,32);
  c.dp=make_unique<PrefixDP>(N,c.paper);c.dp->public_data(c.target);c.order.resize(c.paper.size());iota(c.order.begin(),c.order.end(),0);for(auto p:c.paper)c.length.push_back(p.len);c.future.resize(c.paper.size());
  c.f1.resize(P);c.f2.resize(P);
  for(int a=0;a<P;a++){int m;auto chosen=c.dp->solve(c.order,c.target[a],m);for(int k:chosen)c.future[k]|=Access::bit(a);c.f2[a]=m;c.mean2+=m/double(P);
   int f[65];fill(f,f+65,1000);f[0]=0;for(auto p:c.paper){U64 d=(p.color^c.target[a])&PrefixDP::low(p.len);int start=d?64-__builtin_clzll(d):0;for(int j=start;j<p.len;j++)f[p.len]=min(f[p.len],f[j]+1);}c.f1[a]=f[64];c.mean1+=f[64]/double(P);
  }
  return c;
 };
 Candidate c=build(0);simulate(c,544);cerr<<"Fixed design variant=0 PrepCPU="<<elapsed()<<"\n";
 iota(c.order.begin(),c.order.end(),0);
 for(int k=0;k<K;k++){string s(N,'.');if(k<int(c.paper.size()))for(int j=0;j<c.paper[k].len;j++)s[c.cols[j]]=char('0'+(((c.paper[k].color>>j)&1)^c.flip[j]));cout<<s<<'\n';}cout.flush();
 PotentialPlanner potential(*c.dp);int weight=argc>1?atoi(argv[1]):65536;
 for(int r=0;r<R;r++){
  int a;if(!(cin>>a)||a<0||a>=P)return 0;
  for(int k=0;k<int(c.paper.size());k++)c.dp->reward[k]=16+2*maskpop(c.future[k]);int m;auto chosen=weight?potential.solve(c.order,c.target[a],c.future,weight,m):c.dp->solve(c.order,c.target[a],m);if(m<0||m>2*K)return 0;vector<char>selected(c.paper.size());for(int k:chosen)selected[k]=true;
  vector<pair<int,int>>ops;ops.reserve(m);
  for(int i=0;i<m;i++){auto[from,to]=GlobalPrefix::best_move(c.order,c.length,selected,.5,&c.future);if(from<0)return 0;int k=c.order[from];c.order.erase(c.order.begin()+from);c.order.insert(c.order.begin()+to,k);ops.push_back({k,to});}
  if(weight){int actual=total_potential(c.order,c.length,c.future);if(actual!=potential.last_phi){cerr<<"POTENTIAL MISMATCH r="<<r<<" expected="<<potential.last_phi<<" actual="<<actual<<"\n";abort();}}
  Access bit=Access::bit(a);for(int k=0;k<int(c.paper.size());k++)c.future[k]=(c.future[k]&~bit)|(selected[k]?bit:Access(0));
  cout<<ops.size()<<'\n';for(auto[k,d]:ops)cout<<k<<' '<<d<<'\n';cout.flush();
 }
 if(DIAGNOSTICS)cerr<<"FinishCPU="<<elapsed()<<"\n";
}

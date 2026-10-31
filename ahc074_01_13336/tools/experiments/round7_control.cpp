// AHC074: prefix dictionaries fitted to exact legal move counts.
// Initial learning and all simulated requests use only the public 256 products.
// Real requests are read one at a time after flushing the previous response.
// CPU guards preserve a feasible dictionary at every completed update.
#pragma GCC optimize("O3")
#include <bits/stdc++.h>
using namespace std;using U64=uint64_t;
struct PrefixPaper {int len;U64 color;};
struct PrefixDP {
 int n,k,epoch=0;vector<PrefixPaper>a;vector<array<int,193>>cnt,work,parent,which,stamp;int pos[192];vector<int>reward;
 vector<array<vector<uint8_t>,64>>candidate;unordered_map<U64,int>product_index;
 PrefixDP(int nn,vector<PrefixPaper>aa):n(nn),k(aa.size()),a(move(aa)),cnt(n+2),work(n+1),parent(n+1),which(n+1),stamp(n+1),reward(k,16){}
 static U64 low(int n){return n==64?~0ULL:(1ULL<<n)-1;}
 void public_data(const vector<U64>&A){candidate.resize(A.size());for(int v=0;v<int(A.size());v++){product_index[A[v]]=v;for(int z=0;z<k;z++){U64 diff=(A[v]^a[z].color)&low(a[z].len);int start=diff?64-__builtin_clzll(diff):0;for(int j=start;j<a[z].len;j++)candidate[v][j].push_back(z);}}}
 void prepare(const vector<int>&p){for(int i=0;i<k;i++)pos[p[i]]=i;for(int l=0;l<=n+1;l++){cnt[l][0]=0;for(int i=0;i<k;i++)cnt[l][i+1]=cnt[l][i]+(a[p[i]].len>=l);}}
 vector<int>solve(const vector<int>&p,U64 target,int&best,bool reconstruct=true){prepare(p);return solve_prepared(target,best,reconstruct);}
 vector<int>solve_prepared(U64 target,int&best,bool reconstruct=true){
  auto&dp=work;epoch++;int active[65][193],count[65]{};dp[0][0]=0;stamp[0][0]=epoch;active[0][count[0]++]=0;const auto&all=candidate[product_index.at(target)];
  for(int j=0;j<n;j++){
   int ac=count[j];if(!ac)continue;if(reconstruct)sort(active[j],active[j]+ac);
   for(int z:all[j]){int l=a[z].len,q=pos[z];
    for(int ai=0;ai<ac;ai++){int h=active[j][ai],add=0;if(q>=h){int moved=cnt[l][q]-cnt[l][h],upper=cnt[j+1][q]-cnt[j+1][h];add=moved+(upper>moved);}int nh=max(q+1,h),value=dp[j][h]+add*65536-reward[z];
     if(stamp[l][nh]!=epoch){stamp[l][nh]=epoch;dp[l][nh]=1000000000;active[l][count[l]++]=nh;}
     if(value<dp[l][nh]){dp[l][nh]=value;if(reconstruct){parent[l][nh]=j*(k+1)+h;which[l][nh]=z;}}
    }
   }
  }
  int h=active[n][0],j=n;for(int i=1;i<count[n];i++){int x=active[n][i];if(dp[n][x]<dp[n][h]||(dp[n][x]==dp[n][h]&&x<h))h=x;}best=(dp[n][h]+65535)/65536;vector<int>ans;if(!reconstruct)return ans;
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
// Take the first visible paper admitting a shortest-path move. Among its
// insertion positions, minimize mandatory removals over 64 previous goals.
// A suffix union counts correlated choices exactly.
// Length inversions break remaining ties and initialize the access model.
inline std::pair<int,int> best_move(const std::vector<int>& order,
                                  const std::vector<int>& length,
                                  const std::vector<char>& selected,
                                  double decay=.5, const std::vector<uint64_t>* access=nullptr) {
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
    uint64_t below[192]{}, mask_tree[65]{};
    if(access){for(int i=K-1;i>=0;i--){int k=order[i];for(int x=length[k];x>0;x-=x&-x)below[k]|=mask_tree[x];for(int x=length[k];x<=N;x+=x&-x)mask_tree[x]|=(*access)[k];}}
    double initial_quality=0;
    int initial_inversions=0;
    std::array<int,66> fenwick{};
    for(int i=K-1;i>=0;i--){
        int k=order[i];
        for(int x=length[k]-1;x>0;x-=x&-x)inversions[k]+=fenwick[x];
        for(int x=length[k];x<=N;x+=x&-x)fenwick[x]++;
        if(access)initial_quality+=__builtin_popcountll(below[k]);
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
        uint64_t remaining[193];if(access&&direction==1){remaining[K]=0;for(int d=K-1;d>i;d--){int x=order[d];remaining[d]=remaining[d+1]|(length[x]<=kl?(*access)[x]:0ULL);}}
        {
            moved=initial_moved;before=initial_before;bad=initial_bad;
            int distance=initial_distance, invk=inversions[k], invsum=initial_inversions;
            int following_min=suffix[i+1];double quality=initial_quality;uint64_t own=0;int own_count=0;
            if(access){own=below[k];own_count=__builtin_popcountll(own);}
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
                     int next_count=__builtin_popcountll(remaining[d+1]);quality+=next_count-own_count;own_count=next_count;
                     if(xl>=kl)quality+=__builtin_popcountll((*access)[k]&~below[x]);
                    }
                }else{
                    if(selected[k]&&xl>=kl)change_moved(x,xl>=following_min);
                    if(selected[x]&&kl>=xl)change_moved(k,true);
                    if(!selected[x]&&next[x]==k)change_before(x,false);
                    if(!selected[k]&&next[k]==x)change_before(k,true);
                    if(selected[x])following_min=std::min(following_min,xl);
                    if(xl<kl){invk++;invsum++;}else if(xl>kl){if(!access)quality-=power[inversions[x]-1]-power[inversions[x]];invsum--;}
                    if(access){
                     own|=(*access)[x];int next_count=__builtin_popcountll(own);quality+=next_count-own_count;own_count=next_count;
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


#ifndef AHC_LEARNING_BUDGET
#define AHC_LEARNING_BUDGET 100
#endif
#ifndef AHC_REFINEMENT_BUDGET
#define AHC_REFINEMENT_BUDGET 100
#endif
static double multi_training_budget=AHC_LEARNING_BUDGET;static bool multi_training_finished=true;
#ifndef STRUCTURE2_TRAINING_CPU_LIMIT
#define STRUCTURE2_TRAINING_CPU_LIMIT multi_training_budget
#endif
static clock_t structure2_training_origin=0;

#ifdef INITIAL_TRIE_CHECK
#define ROUND6_TRIE_CHECK
#endif
#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

// Unary paths contribute their weight whenever at least one descendant leaf
// is selected. Mapping those nodes to the first retained descendant preserves
// every leaf/path weight and every k-leaf DP value. Retained nodes keep their
// original insertion order, which preserves the original leaf tie order.
struct Round6TrieTopology {
 struct Node {int child[2]{-1,-1};int sample=0;};
 std::vector<Node>nodes;
 std::vector<std::array<uint16_t,65>>path;
#ifdef ROUND6_TRIE_CHECK
 std::vector<Node>raw_nodes;
 std::vector<std::array<uint16_t,65>>raw_path;
#endif
};
inline const Round6TrieTopology& round6_trie_topology(const std::vector<uint64_t>&A,int length){
 struct Cache {std::vector<uint64_t>data;std::array<std::unique_ptr<Round6TrieTopology>,65>topology;};
 static Cache cache;
 if(cache.data!=A){cache.data=A;for(auto&entry:cache.topology)entry.reset();}
 assert(length>=1&&length<=64);
 if(cache.topology[length])return *cache.topology[length];
 auto result=std::make_unique<Round6TrieTopology>();
 std::vector<Round6TrieTopology::Node>raw(1);
 std::vector<std::array<uint16_t,65>>raw_path(A.size());
 for(int v=0;v<int(A.size());v++){
  int x=0;raw_path[v][0]=0;
  for(int d=1;d<=length;d++){
   int bit=(A[v]>>(length-d))&1;
   if(raw[x].child[bit]<0){int next=raw.size();raw[x].child[bit]=next;raw.emplace_back();}
   x=raw[x].child[bit];raw[x].sample=v;assert(x<65536);raw_path[v][d]=x;
  }
 }
 std::vector<int>map(raw.size(),-1);
 for(int x=0;x<int(raw.size());x++){
  bool left=raw[x].child[0]>=0,right=raw[x].child[1]>=0;
  if(left==right){map[x]=result->nodes.size();result->nodes.emplace_back();}
 }
 for(int x=int(raw.size())-1;x>=0;x--)if(map[x]<0){int child=raw[x].child[0]>=0?raw[x].child[0]:raw[x].child[1];map[x]=map[child];}
 assert(map[0]==0&&result->nodes.size()<=2*A.size()-1);
 for(int x=0;x<int(raw.size());x++){
  bool left=raw[x].child[0]>=0,right=raw[x].child[1]>=0;if(left!=right)continue;
  auto&node=result->nodes[map[x]];node.sample=raw[x].sample;
  for(int bit=0;bit<2;bit++)if(raw[x].child[bit]>=0){node.child[bit]=map[raw[x].child[bit]];assert(node.child[bit]>map[x]);}
 }
 result->path.resize(A.size());
 for(int v=0;v<int(A.size());v++)for(int d=0;d<=length;d++)result->path[v][d]=map[raw_path[v][d]];
#ifdef ROUND6_TRIE_CHECK
 result->raw_nodes=std::move(raw);result->raw_path=std::move(raw_path);
#endif
 cache.topology[length]=std::move(result);return *cache.topology[length];
}

inline void train_backbone_dictionary(const std::vector<uint64_t>& A,std::vector<PrefixPaper>&papers,int n,int perlevel,int passes,int importance){
 const int P=A.size(),K=192,INF=1000;
 papers.clear();for(int l=1;l<=n;l++)papers.push_back({l,PrefixDP::low(l-1)});papers.push_back({n,PrefixDP::low(n)});int fixed=papers.size();papers.resize(K,{0,0});
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

 std::vector<std::array<int,65>>cross(P);
 auto advance=[&](int l){for(int v=0;v<P;v++)for(int k:level[l]){int best=INF;for(int j=start[v][k];j<l;j++){
  bw[v][j]=std::min(bw[v][j],1+bw[v][l]);best=std::min(best,fw[v][j]);if(j+1<l)cross[v][j+1]=std::min(cross[v][j+1],best+1+bw[v][l]);
 }}};
 auto check=[&](int l){
#ifdef INITIAL_DP_CHECK
  auto savedF=fw,savedB=bw;distances();for(int v=0;v<P;v++){for(int j=0;j<l;j++)assert(savedF[v][j]==fw[v][j]);for(int j=l;j<=n;j++)assert(savedB[v][j]==bw[v][j]);
   int prefix_min[65];prefix_min[l]=INF;for(int j=l-1;j>=0;j--)prefix_min[j]=std::min(prefix_min[j+1],fw[v][j]);int reference=INF;
   for(int upper=l+1;upper<=n;upper++)for(int k:level[upper])if(start[v][k]<l)reference=std::min(reference,prefix_min[start[v][k]]+1+bw[v][upper]);assert(reference==cross[v][l]);
  }fw=std::move(savedF);bw=std::move(savedB);
#else
  (void)l;
#endif
 };
 struct Node{int child[2]{-1,-1},weight=0,sample=0,available=1;std::array<int,17>dp,split;Node(){dp.fill(-100000000);dp[0]=0;split.fill(0);}};
 for(int pass=0;pass<passes;pass++){
  if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>STRUCTURE2_TRAINING_CPU_LIMIT){multi_training_finished=false;break;}
  distances();for(int v=0;v<P;v++){bw[v].fill(INF);bw[v][n]=0;cross[v].fill(INF);}
  for(int l=n;l>0;l--){if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>STRUCTURE2_TRAINING_CPU_LIMIT){multi_training_finished=false;break;}
  int amount=std::min(l>=53?8:4,int(extra[l].size()+spare.size()));if(!amount){advance(l);continue;}check(l);const auto&topology=round6_trie_topology(A,l);std::vector<Node>trie(topology.nodes.size());
  for(int x=0;x<int(trie.size());x++){trie[x].child[0]=topology.nodes[x].child[0];trie[x].child[1]=topology.nodes[x].child[1];trie[x].sample=topology.nodes[x].sample;}
#ifdef INITIAL_TRIE_CHECK
  std::vector<Node>rawtrie(topology.raw_nodes.size());for(int x=0;x<int(rawtrie.size());x++){rawtrie[x].child[0]=topology.raw_nodes[x].child[0];rawtrie[x].child[1]=topology.raw_nodes[x].child[1];rawtrie[x].sample=topology.raw_nodes[x].sample;}
#endif

  for(int v=0;v<P;v++){
   // A route avoiding l crosses it once, along an edge j < l < upper.
   // The prefix and suffix distances on that edge cannot use level l.
   int prefix_min[65];prefix_min[l]=INF;for(int j=l-1;j>=0;j--)prefix_min[j]=std::min(prefix_min[j+1],fw[v][j]);
   int bypass=cross[v][l];
   for(int k:level[l])if(k<fixed&&start[v][k]<l)bypass=std::min(bypass,prefix_min[start[v][k]]+1+bw[v][l]);
   int node=0,bestprefix=INF,prev=0;
   for(int d=1;d<=l;d++){
    node=topology.path[v][d];
    bestprefix=std::min(bestprefix,fw[v][l-d]);int benefit=std::max(0,bypass-(bestprefix+1+bw[v][l]));int delta=importance*(benefit-prev)+1;trie[node].weight+=delta;
#ifdef INITIAL_TRIE_CHECK
    rawtrie[topology.raw_path[v][d]].weight+=delta;
#endif
    prev=benefit;
   }
  }
  for(int node=int(trie.size())-1;node>=0;node--){auto&v=trie[node];int a=v.child[0],b=v.child[1];if(a<0&&b<0){v.dp[1]=v.weight;continue;}
   int ca=a<0?0:trie[a].available,cb=b<0?0:trie[b].available;v.available=std::min(amount,ca+cb);for(int q=1;q<=v.available;q++)for(int left=std::max(0,q-cb);left<=std::min(q,ca);left++){int right=q-left;if((a<0&&left)||(b<0&&right))continue;int value=(a<0?0:trie[a].dp[left])+(b<0?0:trie[b].dp[right])+v.weight;if(value>v.dp[q])v.dp[q]=value,v.split[q]=left;}
  }
#ifdef INITIAL_TRIE_CHECK
  for(int node=int(rawtrie.size())-1;node>=0;node--){auto&v=rawtrie[node];int a=v.child[0],b=v.child[1];if(a<0&&b<0){v.dp[1]=v.weight;continue;}
   int ca=a<0?0:rawtrie[a].available,cb=b<0?0:rawtrie[b].available;v.available=std::min(amount,ca+cb);for(int q=1;q<=v.available;q++)for(int left=std::max(0,q-cb);left<=std::min(q,ca);left++){int right=q-left;if((a<0&&left)||(b<0&&right))continue;int value=(a<0?0:rawtrie[a].dp[left])+(b<0?0:rawtrie[b].dp[right])+v.weight;if(value>v.dp[q])v.dp[q]=value,v.split[q]=left;}
  }

  auto leaf_sequence=[&](const std::vector<Node>&tree,int count){std::vector<int>out;std::function<void(int,int)>walk=[&](int x,int q){if(!q)return;const auto&node=tree[x];if(node.child[0]<0&&node.child[1]<0){out.push_back(node.sample);return;}int left=node.split[q];if(left)walk(node.child[0],left);if(q-left)walk(node.child[1],q-left);};walk(0,count);return out;};
  for(int q=0;q<=amount;q++){assert(trie[0].dp[q]==rawtrie[0].dp[q]);if(q<=trie[0].available)assert(leaf_sequence(trie,q)==leaf_sequence(rawtrie,q));}
#endif
  int use=amount;while(use>1&&trie[0].dp[use]<0)use--;std::vector<uint64_t>pattern;
  std::function<void(int,int)>collect=[&](int node,int count){if(!count)return;auto&v=trie[node];if(v.child[0]<0&&v.child[1]<0){pattern.push_back(A[v.sample]&PrefixDP::low(l));return;}int left=v.split[count];if(left)collect(v.child[0],left);if(count-left)collect(v.child[1],count-left);};collect(0,use);
  while(extra[l].size()<pattern.size()){int k=spare.back();spare.pop_back();extra[l].push_back(k);level[l].push_back(k);papers[k].len=l;}
  for(int q=0;q<int(pattern.size());q++){int k=extra[l][q];papers[k].color=pattern[q];update(k);}
  advance(l);
 }
 }
 std::cerr<<"backbone "<<initial<<" -> "<<distances()<<" papers="<<K-spare.size()<<" CPU="<<double(clock())/CLOCKS_PER_SEC<<"\n";
 std::vector<PrefixPaper>p;for(auto x:papers)if(x.len)p.push_back(x);std::stable_sort(p.begin(),p.end(),[](auto a,auto b){return a.len<b.len;});papers=p;
}


inline void refit_all_prefix_papers(const std::vector<uint64_t>&A,std::vector<PrefixPaper>&papers,int passes,int importance){
 const int P=A.size(),N=64,K=papers.size(),INF=1000;std::vector<std::vector<int>>level(65),start(P,std::vector<int>(K));
 for(int k=0;k<K;k++)level[papers[k].len].push_back(k);
 auto update=[&](int k){for(int v=0;v<P;v++){uint64_t d=(A[v]^papers[k].color)&PrefixDP::low(papers[k].len);start[v][k]=d?64-__builtin_clzll(d):0;}};for(int k=0;k<K;k++)update(k);
 std::vector<std::array<int,65>>F(P),B(P);
 auto distances=[&](){double total=0;for(int v=0;v<P;v++){auto&f=F[v];auto&b=B[v];f.fill(INF);b.fill(INF);f[0]=0;b[N]=0;for(int l=1;l<=N;l++)for(int k:level[l])for(int j=start[v][k];j<l;j++)f[l]=std::min(f[l],f[j]+1);for(int l=N;l>0;l--)for(int k:level[l])for(int j=start[v][k];j<l;j++)b[j]=std::min(b[j],b[l]+1);total+=double(f[N])/P;}return total;};
 double initial=distances();int accepted=0,rejected=0;

 std::vector<std::array<int,65>>cross(P);
 auto advance=[&](int l){for(int v=0;v<P;v++)for(int k:level[l]){int best=INF;for(int j=start[v][k];j<l;j++){
  B[v][j]=std::min(B[v][j],1+B[v][l]);best=std::min(best,F[v][j]);if(j+1<l)cross[v][j+1]=std::min(cross[v][j+1],best+1+B[v][l]);
 }}};
 auto check=[&](int l){
#ifdef INITIAL_DP_CHECK
  auto savedF=F,savedB=B;distances();for(int v=0;v<P;v++){for(int j=0;j<l;j++)assert(savedF[v][j]==F[v][j]);for(int j=l;j<=N;j++)assert(savedB[v][j]==B[v][j]);
   int prefix_min[65];prefix_min[l]=INF;for(int j=l-1;j>=0;j--)prefix_min[j]=std::min(prefix_min[j+1],F[v][j]);int reference=INF;
   for(int upper=l+1;upper<=N;upper++)for(int k:level[upper])if(start[v][k]<l)reference=std::min(reference,prefix_min[start[v][k]]+1+B[v][upper]);assert(reference==cross[v][l]);
  }F=std::move(savedF);B=std::move(savedB);
#else
  (void)l;
#endif
 };
 struct Node{int child[2]{-1,-1},weight=0,sample=0,available=1;std::array<int,17>dp,split;Node(){dp.fill(-1000000000);dp[0]=0;split.fill(0);}};
 for(int pass=0;pass<passes;pass++){
  if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>STRUCTURE2_TRAINING_CPU_LIMIT){multi_training_finished=false;break;}
  distances();for(int v=0;v<P;v++){B[v].fill(INF);B[v][N]=0;cross[v].fill(INF);}
  for(int l=N;l>0;l--){if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>STRUCTURE2_TRAINING_CPU_LIMIT){multi_training_finished=false;break;}
  int amount=level[l].size();if(!amount){advance(l);continue;}check(l);const auto&topology=round6_trie_topology(A,l);std::vector<Node>trie(topology.nodes.size());
  for(int x=0;x<int(trie.size());x++){trie[x].child[0]=topology.nodes[x].child[0];trie[x].child[1]=topology.nodes[x].child[1];trie[x].sample=topology.nodes[x].sample;}
#ifdef INITIAL_TRIE_CHECK
  std::vector<Node>rawtrie(topology.raw_nodes.size());for(int x=0;x<int(rawtrie.size());x++){rawtrie[x].child[0]=topology.raw_nodes[x].child[0];rawtrie[x].child[1]=topology.raw_nodes[x].child[1];rawtrie[x].sample=topology.raw_nodes[x].sample;}
#endif
std::vector<int>without_level(P);
  for(int v=0;v<P;v++){
   // A route avoiding l crosses it once, along an edge j < l < upper.
   // The prefix and suffix distances on that edge cannot use level l.
   int prefix_min[65];prefix_min[l]=INF;for(int j=l-1;j>=0;j--)prefix_min[j]=std::min(prefix_min[j+1],F[v][j]);
   int bypass=cross[v][l];
   without_level[v]=bypass;int bestprefix=INF,previous=0,x=0;
   for(int d=1;d<=l;d++){
    x=topology.path[v][d];
    bestprefix=std::min(bestprefix,F[v][l-d]);int route=bestprefix+1+B[v][l];int benefit=0;
    if(route<INF){if(bypass>=INF)benefit=1000000+importance*(N+1-route);else benefit=importance*std::max(0,bypass-route);}
    int delta=benefit-previous+1;trie[x].weight+=delta;
#ifdef INITIAL_TRIE_CHECK
    rawtrie[topology.raw_path[v][d]].weight+=delta;
#endif
    previous=benefit;
   }
  }
  for(int x=int(trie.size())-1;x>=0;x--){auto&p=trie[x];int a=p.child[0],b=p.child[1];if(a<0&&b<0){p.dp[1]=p.weight;continue;}
   int ca=a<0?0:trie[a].available,cb=b<0?0:trie[b].available;p.available=std::min(amount,ca+cb);for(int q=1;q<=p.available;q++)for(int left=std::max(0,q-cb);left<=std::min(q,ca);left++){int right=q-left;if((a<0&&left)||(b<0&&right))continue;int value=(a<0?0:trie[a].dp[left])+(b<0?0:trie[b].dp[right])+p.weight;if(value>p.dp[q])p.dp[q]=value,p.split[q]=left;}
  }
#ifdef INITIAL_TRIE_CHECK
  for(int x=int(rawtrie.size())-1;x>=0;x--){auto&p=rawtrie[x];int a=p.child[0],b=p.child[1];if(a<0&&b<0){p.dp[1]=p.weight;continue;}
   int ca=a<0?0:rawtrie[a].available,cb=b<0?0:rawtrie[b].available;p.available=std::min(amount,ca+cb);for(int q=1;q<=p.available;q++)for(int left=std::max(0,q-cb);left<=std::min(q,ca);left++){int right=q-left;if((a<0&&left)||(b<0&&right))continue;int value=(a<0?0:rawtrie[a].dp[left])+(b<0?0:rawtrie[b].dp[right])+p.weight;if(value>p.dp[q])p.dp[q]=value,p.split[q]=left;}
  }

  auto leaf_sequence=[&](const std::vector<Node>&tree,int count){std::vector<int>out;std::function<void(int,int)>walk=[&](int x,int q){if(!q)return;const auto&node=tree[x];if(node.child[0]<0&&node.child[1]<0){out.push_back(node.sample);return;}int left=node.split[q];if(left)walk(node.child[0],left);if(q-left)walk(node.child[1],q-left);};walk(0,count);return out;};
  for(int q=0;q<=amount;q++){assert(trie[0].dp[q]==rawtrie[0].dp[q]);if(q<=trie[0].available)assert(leaf_sequence(trie,q)==leaf_sequence(rawtrie,q));}
#endif
  if(trie[0].dp[amount]<0){rejected++;advance(l);continue;}
  std::vector<uint64_t>pattern;std::function<void(int,int)>collect=[&](int x,int q){if(!q)return;auto&p=trie[x];if(p.child[0]<0&&p.child[1]<0){pattern.push_back(A[p.sample]&PrefixDP::low(l));return;}int left=p.split[q];if(left)collect(p.child[0],left);if(q-left)collect(p.child[1],q-left);};collect(0,amount);
  bool valid=true;for(int v=0;v<P&&valid;v++)if(without_level[v]>=INF){int route=INF;for(uint64_t c:pattern){uint64_t d=(c^A[v])&PrefixDP::low(l);int st=d?64-__builtin_clzll(d):0;for(int j=st;j<l;j++)route=std::min(route,F[v][j]+1+B[v][l]);}if(route>=INF)valid=false;}
  if(!valid){rejected++;advance(l);continue;}for(int q=0;q<amount;q++){int k=level[l][q];if(papers[k].color!=pattern[q]){papers[k].color=pattern[q];update(k);}}accepted++;
  advance(l);
 }
 }
 std::cerr<<"refit "<<initial<<" -> "<<distances()<<" accepted="<<accepted<<" rejected="<<rejected<<"\n";
}


// Construct the final permutation for simulations and actual responses.
// Actual responses also generate and check the legal moves below.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <climits>
#include <vector>
namespace GlobalPrefix {
inline std::vector<int> final_order_incremental(const std::vector<int>&order,const std::vector<int>&length,const std::vector<char>&selected,const std::vector<uint64_t>*future=nullptr){
 using namespace std;
 int K=order.size(),N=*max_element(length.begin(),length.end()),owner[65],root=-1;fill(owner,owner+65,-1);for(int k=0;k<K;k++)if(selected[k])owner[length[k]]=k;for(int l=N;l>=0;l--){if(owner[l]>=0)root=owner[l];owner[l]=root;}
 vector<char>mandatory(K);int suffix=N+1;for(int i=K-1;i>=0;i--){int k=order[i];mandatory[k]=length[k]>=suffix;if(selected[k])suffix=min(suffix,length[k]);}
 vector<vector<int>>children(K);vector<int>insert;int pos[192];for(int i=0;i<K;i++){int k=order[i];pos[k]=i;if(selected[k])continue;if(mandatory[k])insert.push_back(k);else children[owner[length[k]]].push_back(k);}
 stable_sort(insert.begin(),insert.end(),[&](int a,int b){return length[a]<length[b];});
 double power[193];power[0]=1;for(int i=1;i<=K;i++)power[i]=power[i-1]*.5;
 uint64_t below[192]{};int inversions[192]{};
 for(auto&v:children)for(int i=int(v.size())-1;i>=0;i--){int x=v[i];for(int j=i+1;j<int(v.size());j++){int y=v[j];if(length[y]<length[x])inversions[x]++;if(future&&length[y]<=length[x])below[x]|=(*future)[y];}}
 for(int k:insert){auto&v=children[owner[length[k]]];int M=v.size(),kl=length[k];uint64_t remain[193];int lower_after[193];remain[M]=0;lower_after[M]=0;
  for(int i=M-1;i>=0;i--){int x=v[i];remain[i]=remain[i+1]|(future&&length[x]<=kl?(*future)[x]:0ULL);lower_after[i]=lower_after[i+1]+(length[x]<kl);}
  double gain=0,best=1e100;int larger=0,bestinv=INT_MAX,at=0;
  for(int i=0;i<=M;i++){double q=gain+(future?double(__builtin_popcountll(remain[i])):1-power[lower_after[i]]);int inv=larger+lower_after[i];if(q<best-1e-10||(abs(q-best)<=1e-10&&inv<bestinv)){best=q;bestinv=inv;at=i;}
   if(i<M){int x=v[i];if(length[x]>kl)larger++;if(future){if(length[x]>=kl)gain+=__builtin_popcountll((*future)[k]&~below[x]);}else if(length[x]>kl)gain+=power[inversions[x]]*.5;}
  }
  below[k]=remain[at];inversions[k]=lower_after[at];for(int i=0;i<at;i++){int x=v[i];if(future&&length[x]>=kl)below[x]|=(*future)[k];if(length[x]>kl)inversions[x]++;}
  v.insert(v.begin()+at,k);
 }
 vector<int>out;out.reserve(K);for(int l=1;l<=N;l++){int k=owner[l];if(k>=0&&selected[k]&&length[k]==l){out.push_back(k);for(int x:children[k])out.push_back(x);}}return out;
}
}

inline vector<int> direct_final(const vector<int>&order,const vector<int>&length,const vector<char>&selected,const vector<uint64_t>*future=nullptr){return GlobalPrefix::final_order_incremental(order,length,selected,future);}

inline vector<uint64_t> selection_teacher(const vector<U64>&A,const vector<PrefixPaper>&papers){
 int P=A.size();PrefixDP dp(64,papers);dp.public_data(A);vector<int>order(papers.size()),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);
 vector<uint64_t>future(papers.size());mt19937 freq_rng(912367);vector<int>train_targets(P);iota(train_targets.begin(),train_targets.end(),0);shuffle(train_targets.begin(),train_targets.end(),freq_rng);
 for(int r=0;r<192;r++){
  int target=r<128?freq_rng()%P:train_targets[r-128],moves;auto chosen=dp.solve(order,A[target],moves);vector<char>selected(papers.size());for(int k:chosen){selected[k]=true;if(r>=128)future[k]|=1ULL<<(r-128);}
  order=direct_final(order,length,selected);
 }return future;
}
struct MultiModel {vector<U64>A;vector<int>cols,flip;vector<PrefixPaper>papers;vector<uint64_t>future;};
MultiModel train_multi_model(const vector<U64>&original,U64 random_seed){
 int n=64,K=192,P=256;vector<U64>A=original;
 mt19937_64 rng(random_seed==32||random_seed==1?12345:random_seed);vector<int>cols(n),flip(n);iota(cols.begin(),cols.end(),0);
 int correlation[64][64]{};for(int x=0;x<n;x++)for(int y=0;y<n;y++)for(U64 v:A)correlation[x][y]+=((v>>x)^(v>>y))&1;
 auto cost=[&](){int s=0;for(int j=1;j<n;j++){int d=correlation[cols[j-1]][cols[j]];s+=min(d,P-d);}return s;};
 int current=cost(),best=current;auto bestcols=cols;
 static vector<int>saved_columns;if(saved_columns.empty()||(random_seed!=12345&&random_seed!=32&&random_seed!=1)){
 for(int it=0;it<300000;it++){
  int x=rng()%n,y=rng()%n;if(x>y)swap(x,y);(void)(rng()%3);
  auto edge=[&](int a,int b){int d=correlation[a][b];return min(d,P-d);};int next=current;
  if(x)next+=edge(cols[x-1],cols[y])-edge(cols[x-1],cols[x]);
  if(y<63)next+=edge(cols[x],cols[y+1])-edge(cols[y],cols[y+1]);
  reverse(cols.begin()+x,cols.begin()+y+1);double temperature=8*pow(.002,double(it)/300000);
  if(next<=current||double(rng()>>11)*0x1p-53<exp((current-next)/temperature)){current=next;if(current<best)best=current,bestcols=cols;}
  else reverse(cols.begin()+x,cols.begin()+y+1);
 }
 if(random_seed==12345||random_seed==32||random_seed==1)saved_columns=bestcols;}else bestcols=saved_columns;
 cols=bestcols;if(random_seed==32)rotate(cols.begin(),cols.begin()+32,cols.end());if(random_seed==1)reverse(cols.begin(),cols.end());for(int j=1;j<n;j++)flip[j]=flip[j-1]^(correlation[cols[j-1]][cols[j]]>P/2);
 for(U64&v:A){U64 old=v;v=0;for(int j=0;j<n;j++)v|=(((old>>cols[j])&1)^flip[j])<<j;}
 vector<PrefixPaper>papers;
 train_backbone_dictionary(A,papers,n,6,1,8);
 refit_all_prefix_papers(A,papers,4,32);

 return {move(A),move(cols),move(flip),move(papers),{}};
}



// Coordinate refit against exact legal move costs on public-data snapshots.
// Snapshots and their target products are generated internally before real orders.
inline void exact_state_refit(const vector<U64>&A,vector<PrefixPaper>&papers,int passes,int bottom,int strength,int repetitions=1,int regularizer=0){
 auto expired=[](){return double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>=AHC_REFINEMENT_BUDGET;};
 if(expired())return;
 assert(regularizer==0);const auto entry=clock();
 const bool adaptive=repetitions<0;
 if(adaptive)repetitions=32;
 const int N=64,K=papers.size(),P=A.size(),INF=255,unit=1;
 int M=P*repetitions;clock_t calibration_start=0;
 vector<int>length;vector<vector<int>>level(N+1);for(int k=0;k<K;k++){length.push_back(papers[k].len);level[length[k]].push_back(k);}
 struct Scenario{int target;array<uint8_t,192>pos;array<array<uint8_t,193>,65>cnt;};
 vector<Scenario>scenarios(M);
 {
  PrefixDP dp(N,papers);dp.public_data(A);vector<int>order(K);iota(order.begin(),order.end(),0);
  auto future=selection_teacher(A,papers);mt19937 rng(1237189);vector<int>targets(P);iota(targets.begin(),targets.end(),0);shuffle(targets.begin(),targets.end(),rng);
  for(int r=0;r<128+M;r++){
   if((r&63)==0&&expired())return;
   if(r>128&&(r-128)%P==0)shuffle(targets.begin(),targets.end(),rng);
   int t=r<128?rng()%P:targets[(r-128)%P];
   if(r==128)calibration_start=clock();
   if(r>=128){auto&s=scenarios[r-128];s.target=t;for(int i=0;i<K;i++)s.pos[order[i]]=i;for(int l=0;l<=N;l++){s.cnt[l][0]=0;for(int i=0;i<K;i++)s.cnt[l][i+1]=s.cnt[l][i]+(length[order[i]]>=l);}}
   for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);
   int moves;auto chosen=dp.solve(order,A[t],moves);vector<char>selected(K);for(int k:chosen)selected[k]=true;
   order=direct_final(order,length,selected,&future);U64 bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
   if(adaptive&&r==128+4*P-1){
    double per_repetition=double(clock()-calibration_start)/CLOCKS_PER_SEC/4;
    double before_capture=double(calibration_start-structure2_training_origin)/CLOCKS_PER_SEC;
    double estimate=(AHC_REFINEMENT_BUDGET-before_capture)/(5.0*max(per_repetition,1e-6));
    repetitions=8;for(int option:{8,12,16,24,32})if(option<=estimate)repetitions=option;
    M=P*repetitions;
    cerr<<"AdaptiveR="<<repetitions<<" capture_per_repetition="<<per_repetition<<" before="<<before_capture<<"\n";
   }
  }
 }
 scenarios.resize(M);
 vector<array<int,192>>start(P);auto update=[&](int k){for(int v=0;v<P;v++){U64 d=(A[v]^papers[k].color)&PrefixDP::low(length[k]);start[v][k]=d?64-__builtin_clzll(d):0;}};for(int k=0;k<K;k++)update(k);
 int accepted=0;long long total_gain=0;auto began=clock();double forward_time=0;size_t finite_states=0;

 struct Table{vector<uint16_t>F;array<uint16_t,65>offset;vector<array<uint8_t,193>>B;array<uint8_t,65>bypass;explicit Table(int rows):B(rows){}};
 vector<unique_ptr<Table>>tables;for(int e=0;e<M;e++)tables.push_back(make_unique<Table>(N-bottom+1));
 for(int pass=0;pass<passes;pass++){
  // Higher lengths are updated first. Prefix distances below the current
  // length remain unchanged, and suffix distances are finalized once each.
  const auto forward_start=clock();
  for(int e=0;e<M;e++){
   if((e&63)==0&&expired())return;
   auto&s=scenarios[e];int v=s.target;auto&t=*tables[e];t.B[N-bottom].fill(0);t.bypass.fill(INF);
   uint8_t fw[65][193],ah[65][193];int ac[65]{};memset(fw,INF,sizeof(fw));fw[0][0]=0;ah[0][ac[0]++]=0;
   vector<int>cand[64];for(int z=0;z<K;z++)for(int j=start[v][z];j<length[z];j++)cand[j].push_back(z);
   for(int j=0;j<N;j++)for(int z:cand[j]){int len=length[z],q=s.pos[z];for(int ix=0;ix<ac[j];ix++){int h=ah[j][ix],cost=0;if(q>=h){int moved=s.cnt[len][q]-s.cnt[len][h],upper=s.cnt[j+1][q]-s.cnt[j+1][h];cost=moved+(upper>moved);}int nh=max(q+1,h),val=fw[j][h]+cost;assert(val<=K);if(fw[len][nh]==INF)ah[len][ac[len]++]=nh;if(val<fw[len][nh])fw[len][nh]=val;}}
   int size=0;for(int j=0;j<N;j++){t.offset[j]=size;size+=ac[j];}t.offset[N]=size;t.F.resize(size);finite_states+=size;
   for(int j=0;j<N;j++)for(int ix=0;ix<ac[j];ix++){int h=ah[j][ix];t.F[t.offset[j]+ix]=(h<<8)|fw[j][h];}
  }
  forward_time+=double(clock()-forward_start)/CLOCKS_PER_SEC;
  for(int l=N;l>=bottom;l--){
   if(expired())break;
   int Q=level[l].size();
   vector<int>bypass(M,INF),through(M*Q);vector<array<uint8_t,65>>route(M*Q),raw(M*Q);vector<int>change_index(M*Q+1);vector<uint16_t>changes;
   for(int e=0;e<M;e++){
    if((e&63)==0&&expired())return;
    auto&s=scenarios[e];int v=s.target;auto&t=*tables[e];auto&B=t.B;
    auto add=[&](int j,int h,int z){int q=s.pos[z],len=length[z];if(q<h)return regularizer;int moved=s.cnt[len][q]-s.cnt[len][h],upper=s.cnt[j+1][q]-s.cnt[j+1][h];return unit*(moved+(upper>moved))+regularizer;};
    if(l<N){auto&out=B[l-bottom];out.fill(INF);for(int len=l+1;len<=N;len++){
     int at[193];fill(at,at+K+1,INF*2);int first=K+1;bool any=false;
     for(int z:level[len])if(start[v][z]<=l){int q=s.pos[z];first=min(first,q);any=true;at[q]=s.cnt[len][q]+B[len-bottom][q+1];}
     if(!any)continue;int same=INF*2,far=INF*2,last=-1;
     for(int h=K;h>=0;h--){int d=s.cnt[l+1][h]-s.cnt[len][h];if(last!=d){far=min(far,same);same=INF*2;last=d;}same=min(same,at[h]);int val=min(same,far+1)-s.cnt[len][h];if(first<h)val=min(val,int(B[len-bottom][h]));if(val<out[h])out[h]=val;}
    }
#ifdef SAMPLING_CHECK_B
    array<int,193>reference;reference.fill(INF);for(int z=0;z<K;z++)if(length[z]>l&&start[v][z]<=l){int len=length[z],q=s.pos[z];for(int h=0;h<=K;h++)reference[h]=min(reference[h],add(l,h,z)+B[len-bottom][max(q+1,h)]);}for(int h=0;h<=K;h++)assert(reference[h]==out[h]);
#endif
    }
    bypass[e]=t.bypass[l];
#ifdef SAMPLING_CHECK_B
    int reference_bypass=INF;
    for(int z=0;z<K;z++)if(length[z]>l&&start[v][z]<l){int len=length[z],q=s.pos[z];for(int j=start[v][z];j<l;j++)for(int fi=t.offset[j];fi<t.offset[j+1];fi++){int packed=t.F[fi],h=packed>>8,f=packed&255;reference_bypass=min(reference_bypass,f+add(j,h,z)+B[len-bottom][max(q+1,h)]);}}
    assert(reference_bypass==bypass[e]);
#endif
    for(int q=0;q<Q;q++){int z=level[l][q],pos=s.pos[z];auto&rr=route[e*Q+q];rr.fill(INF);change_index[e*Q+q]=changes.size();
     for(int j=l-1;j>=0;j--){int value=INF;for(int fi=t.offset[j];fi<t.offset[j+1];fi++){int packed=t.F[fi],h=packed>>8,f=packed&255;value=min(value,f+add(j,h,z)+B[l-bottom][max(pos+1,h)]);}raw[e*Q+q][j]=value;if(value<rr[j+1])changes.push_back(((l-j)<<8)|value);rr[j]=min(value,int(rr[j+1]));}
     through[e*Q+q]=rr[start[v][z]];
    }
   }
  change_index[M*Q]=changes.size();
  struct Node{int child[2]{-1,-1},sample=0;long long weight=0;};
  const auto&topology=round6_trie_topology(A,l);const auto&path=topology.path;vector<Node>trie(topology.nodes.size());
  for(int x=0;x<int(trie.size());x++){trie[x].child[0]=topology.nodes[x].child[0];trie[x].child[1]=topology.nodes[x].child[1];trie[x].sample=topology.nodes[x].sample;}
#ifdef EXACT_TRIE_CHECK
  vector<Node>reference_trie(1);vector<array<int,65>>reference_path(P);
  for(int v=0;v<P;v++){int x=0;for(int d=1;d<=l;d++){int b=A[v]>>(l-d)&1;if(reference_trie[x].child[b]<0){int ni=reference_trie.size();reference_trie[x].child[b]=ni;reference_trie.emplace_back();}x=reference_trie[x].child[b];reference_path[v][d]=x;reference_trie[x].sample=v;}}
#endif
  for(int q=0;q<Q;q++){
   if(expired())return;
   int z=level[l][q];for(auto&node:trie)node.weight=0;long long oldcost=0;
#ifdef EXACT_TRIE_CHECK
   for(auto&node:reference_trie)node.weight=0;
#endif
   for(int e=0;e<M;e++){int v=scenarios[e].target,base=bypass[e];for(int other=0;other<Q;other++)if(other!=q)base=min(base,through[e*Q+other]);oldcost+=min(base,through[e*Q+q]);long long previous=0;
    for(int ix=change_index[e*Q+q];ix<change_index[e*Q+q+1];ix++){int d=changes[ix]>>8,value=changes[ix]&255;long long gain=value>=INF?0:(base>=INF?1000000000000LL+strength*(10000-value):strength*max(0,base-value));trie[path[v][d]].weight+=gain-previous;
#ifdef EXACT_TRIE_CHECK
    reference_trie[reference_path[v][d]].weight+=gain-previous;
#endif
    previous=gain;}
   }
   long long best=LLONG_MIN;int sample=-1;
   for(int x=0;x<int(trie.size());x++){auto&node=trie[x];bool leaf=true;for(int b=0;b<2;b++)if(node.child[b]>=0){trie[node.child[b]].weight+=node.weight;leaf=false;}if(leaf&&node.weight>best)best=node.weight,sample=node.sample;}
#ifdef EXACT_TRIE_CHECK
   long long reference_best=LLONG_MIN;int reference_sample=-1;
   for(int x=0;x<int(reference_trie.size());x++){auto&node=reference_trie[x];bool leaf=true;for(int bit=0;bit<2;bit++)if(node.child[bit]>=0){reference_trie[node.child[bit]].weight+=node.weight;leaf=false;}if(leaf&&node.weight>reference_best)reference_best=node.weight,reference_sample=node.sample;}
   assert(best==reference_best&&sample==reference_sample);
   for(int v=0;v<P;v++)for(int d=1;d<=l;d++)if(d==l||path[v][d]!=path[v][d+1])assert(trie[path[v][d]].weight==reference_trie[reference_path[v][d]].weight);
#endif
   U64 candidate=A[sample]&PrefixDP::low(l);long long newcost=0;bool valid=true;
   for(int e=0;e<M;e++){int v=scenarios[e].target,base=bypass[e];for(int other=0;other<Q;other++)if(other!=q)base=min(base,through[e*Q+other]);U64 d=(A[v]^candidate)&PrefixDP::low(l);int st=d?64-__builtin_clzll(d):0,value=min(base,int(route[e*Q+q][st]));newcost+=value;valid&=value<INF;}
   if(valid&&newcost<oldcost){papers[z].color=candidate;update(z);accepted++;total_gain+=oldcost-newcost;for(int e=0;e<M;e++)through[e*Q+q]=route[e*Q+q][start[scenarios[e].target][z]];}
  }
  for(int e=0;e<M;e++){int v=scenarios[e].target;auto&t=*tables[e];for(int q=0;q<Q;q++){int z=level[l][q],best=INF;for(int j=start[v][z];j+1<l;j++){best=min(best,int(raw[e*Q+q][j]));if(best<t.bypass[j+1])t.bypass[j+1]=best;}}}
 }
 }
 cerr<<"exact_refit accepted="<<accepted<<" gain="<<double(total_gain)/M<<" snapshot="<<double(began-entry)/CLOCKS_PER_SEC<<" forward="<<forward_time<<" finite="<<double(finite_states)/M<<" CPU="<<double(clock()-began)/CLOCKS_PER_SEC<<"\n";
}

#include <algorithm>
#include <array>
#include <utility>
#include <vector>
// The target is the permutation returned by direct_final for the same goal.
inline bool direct_moves(std::vector<int>&order,const std::vector<int>&length,const std::vector<char>&selected,const std::vector<int>&target,std::vector<std::pair<int,int>>&operations){
 const int K=order.size(),N=*std::max_element(length.begin(),length.end());int owner[65],pos[192],rank[192],bad[192]{};bool moving[192]{};std::fill(owner,owner+65,-1);
 for(int k=0;k<K;k++)if(selected[k])owner[length[k]]=k;int root=-1;for(int l=N;l>=0;l--){if(owner[l]>=0)root=owner[l];owner[l]=root;}
 for(int i=0;i<K;i++){pos[order[i]]=i;rank[target[i]]=i;}
 int suffix=N+1,remaining=0;for(int i=K-1;i>=0;i--){int k=order[i];moving[k]=length[k]>=suffix;remaining+=moving[k];if(selected[k])suffix=std::min(suffix,length[k]);}
 for(int k=0;k<K;k++)if(!selected[k]&&!moving[k]&&pos[k]<pos[owner[length[k]]])bad[owner[length[k]]]++;
 for(int k=0;k<K;k++)if(selected[k]&&!moving[k]&&bad[k]){moving[k]=true;remaining++;}
 int previous=-1;for(int k:order)if(!moving[k]){if(rank[k]<=previous)return false;previous=rank[k];}
 auto result=order;std::vector<std::pair<int,int>>ops;ops.reserve(remaining);
 while(remaining--){
  int from=-1,highest=0;for(int i=0;i<K;i++){int k=result[i];if(length[k]>highest){highest=length[k];if(moving[k]){from=i;break;}}}
  if(from<0)return false;int k=result[from],next=-1;for(int r=rank[k]+1;r<K;r++)if(!moving[target[r]]){next=target[r];break;}
  int to=K-1;if(next>=0){to=std::find(result.begin(),result.end(),next)-result.begin();if(from<to)to--;}
  if(to==from)return false;result.erase(result.begin()+from);result.insert(result.begin()+to,k);moving[k]=false;ops.push_back({k,to});
 }
 if(result!=target)return false;order=std::move(result);operations=std::move(ops);return true;
}



static double control_beta=1;static int control_reward=0;
static int ownerFreq[192],productOwner[256];static U64 ownerMask[192];
inline void control_init(const vector<U64>&A,const vector<PrefixPaper>&papers){
 for(int v=0;v<int(A.size());v++){int best=-1;for(int k=0;k<int(papers.size());k++)if((best<0||papers[k].len>papers[best].len)&&!((papers[k].color^A[v])&PrefixDP::low(papers[k].len)))best=k;assert(best>=0);productOwner[v]=best;ownerFreq[best]++;}
 vector<int>targets(A.size());iota(targets.begin(),targets.end(),0);mt19937 rng(912367);shuffle(targets.begin(),targets.end(),rng);for(int r=0;r<64;r++)ownerMask[productOwner[targets[r]]]|=1ULL<<r;
}
inline std::vector<int> control_final(const std::vector<int>&order,const std::vector<int>&length,const std::vector<char>&selected,const std::vector<uint64_t>*future=nullptr){
 using namespace std;
 int K=order.size(),N=*max_element(length.begin(),length.end()),owner[65],root=-1;fill(owner,owner+65,-1);for(int k=0;k<K;k++)if(selected[k])owner[length[k]]=k;for(int l=N;l>=0;l--){if(owner[l]>=0)root=owner[l];owner[l]=root;}
 vector<char>mandatory(K);int suffix=N+1;for(int i=K-1;i>=0;i--){int k=order[i];mandatory[k]=length[k]>=suffix;if(selected[k])suffix=min(suffix,length[k]);}
 vector<vector<int>>children(K);vector<int>insert;int pos[192];for(int i=0;i<K;i++){int k=order[i];pos[k]=i;if(selected[k])continue;if(mandatory[k])insert.push_back(k);else children[owner[length[k]]].push_back(k);}
 stable_sort(insert.begin(),insert.end(),[&](int a,int b){return length[a]<length[b];});
 double power[193];power[0]=1;for(int i=1;i<=K;i++)power[i]=power[i-1]*.5;
 uint64_t below[192]{},belowOwner[192]{};int inversions[192]{},exactOwner[192]{};
 for(auto&v:children)for(int i=int(v.size())-1;i>=0;i--){int x=v[i];for(int j=i+1;j<int(v.size());j++){int y=v[j];if(length[y]<length[x])inversions[x]++;if(future&&length[y]<=length[x]){below[x]|=(*future)[y];belowOwner[x]|=ownerMask[y];exactOwner[x]+=ownerFreq[y];}}}
 for(int k:insert){auto&v=children[owner[length[k]]];int M=v.size(),kl=length[k];uint64_t remain[193],remainOwner[193];int lower_after[193],afterOwner[193];remain[M]=remainOwner[M]=0;lower_after[M]=afterOwner[M]=0;
  for(int i=M-1;i>=0;i--){int x=v[i];remain[i]=remain[i+1]|(future&&length[x]<=kl?(*future)[x]:0ULL);lower_after[i]=lower_after[i+1]+(length[x]<kl);remainOwner[i]=remainOwner[i+1]|(length[x]<=kl?ownerMask[x]:0ULL);afterOwner[i]=afterOwner[i+1]+(length[x]<=kl?ownerFreq[x]:0);}
  double gain=0,best=1e100;int larger=0,bestinv=INT_MAX,at=0;
  for(int i=0;i<=M;i++){double q=gain+(future?(4.0*__builtin_popcountll(remain[i])+control_beta*(afterOwner[i]-4*__builtin_popcountll(remainOwner[i]))):1-power[lower_after[i]]);int inv=larger+lower_after[i];if(q<best-1e-10||(abs(q-best)<=1e-10&&inv<bestinv)){best=q;bestinv=inv;at=i;}
   if(i<M){int x=v[i];if(length[x]>kl)larger++;if(future){if(length[x]>=kl)gain+=4.0*__builtin_popcountll((*future)[k]&~below[x])+control_beta*(ownerFreq[k]-4*__builtin_popcountll(ownerMask[k]&~belowOwner[x]));}else if(length[x]>kl)gain+=power[inversions[x]]*.5;}
  }
  below[k]=remain[at];belowOwner[k]=remainOwner[at];exactOwner[k]=afterOwner[at];inversions[k]=lower_after[at];for(int i=0;i<at;i++){int x=v[i];if(future&&length[x]>=kl){below[x]|=(*future)[k];belowOwner[x]|=ownerMask[k];exactOwner[x]+=ownerFreq[k];}if(length[x]>kl)inversions[x]++;}
  v.insert(v.begin()+at,k);
 }
 vector<int>out;out.reserve(K);for(int l=1;l<=N;l++){int k=owner[l];if(k>=0&&selected[k]&&length[k]==l){out.push_back(k);for(int x:children[k])out.push_back(x);}}return out;
}
int main(int argc,char**argv){control_beta=argc>1?atof(argv[1]):1;control_reward=argc>2?atoi(argv[2]):0;int reps=argc>3?atoi(argv[3]):8;
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int n,K,P,R;if(!(cin>>n>>K>>P>>R)||n!=64||K!=192||P!=256)return 0;
 vector<U64>original(P);for(int v=0;v<P;v++){string x;cin>>x;if(int(x.size())!=n)return 0;for(int j=0;j<n;j++)if(x[j]=='1')original[v]|=1ULL<<j;}
 structure2_training_origin=clock();multi_training_budget=AHC_LEARNING_BUDGET;
 auto model=train_multi_model(original,12345);auto A=move(model.A);auto cols=move(model.cols);auto flip=move(model.flip);auto papers=move(model.papers);
 exact_state_refit(A,papers,1,43,32,reps,0);
 auto future=selection_teacher(A,papers);
 PrefixDP dp(n,papers);dp.public_data(A);vector<int>order(papers.size()),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);
 cerr<<"PrepCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
 for(int k=0;k<K;k++){
  string pattern(n,'.');if(k<int(papers.size()))for(int j=0;j<papers[k].len;j++)pattern[cols[j]]=char('0'+(((papers[k].color>>j)&1)^flip[j]));
  cout<<pattern<<'\n';
 }
 cout.flush();int fallbacks=0,missingOwner=0;control_init(A,papers);
 for(int r=0;r<R;r++){
  int product;if(!(cin>>product)||product<0||product>=P)return 0;
  for(int k=0;k<int(papers.size());k++)dp.reward[k]=16+2*__builtin_popcountll(future[k])+(control_reward?lround(control_beta*(ownerFreq[k]*.5-2*__builtin_popcountll(ownerMask[k]))):0);
  int moves;auto chosen=dp.solve(order,A[product],moves);if(moves<0||moves>2*K)return 0;
  vector<char>selected(papers.size());for(int k:chosen)selected[k]=true;
  vector<pair<int,int>>operations;auto before=order;
  auto target=control_final(order,length,selected,&future);
  if(!direct_moves(order,length,selected,target,operations)||int(operations.size())!=moves){
   fallbacks++;order=move(before);operations.clear();
   for(int i=0;i<moves;i++){
    auto [from,to]=GlobalPrefix::best_move(order,length,selected,.5,&future);if(from<0)return 0;
    int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);operations.push_back({k,to});
   }
  }
  missingOwner+=!selected[productOwner[product]];uint64_t ob=1ULL<<(r%64);for(int k=0;k<int(papers.size());k++)ownerMask[k]=(ownerMask[k]&~ob)|(k==productOwner[product]?ob:0ULL);
  uint64_t bit=1ULL<<(r%64);for(int k=0;k<int(papers.size());k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
  cout<<operations.size()<<'\n';for(auto [k,d]:operations)cout<<k<<' '<<d<<'\n';cout.flush();
 }
 cerr<<"MissingOwner="<<missingOwner<<"\n";cerr<<"PathFallbacks="<<fallbacks<<" FinishCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
}

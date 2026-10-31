// AHC074: public-data prefix dictionaries, exact legal reactive paths.
#pragma GCC optimize("O3")
#include <bits/stdc++.h>
using namespace std;using U64=uint64_t;
struct PrefixPaper {int len;U64 color;};
struct PrefixDP {
 int n,k;vector<PrefixPaper>a;
 vector<array<int,193>> cnt;
 PrefixDP(int n_,vector<PrefixPaper>a_):n(n_),k(a_.size()),a(move(a_)),cnt(n+2){}
 static U64 low(int n){return n==64?~0ULL:(1ULL<<n)-1;}
 vector<int> solve(const vector<int>&p,U64 target,int &best){
  int pos[192];for(int i=0;i<k;i++)pos[p[i]]=i;
  for(int l=0;l<=n+1;l++){cnt[l][0]=0;for(int i=0;i<k;i++)cnt[l][i+1]=cnt[l][i]+(a[p[i]].len>=l);}
  vector<array<int,193>>dp(n+1),parent(n+1),which(n+1);
  for(auto&r:dp)r.fill(1000000);dp[0][0]=0;
  // A secondary preference keeps more visible records after a shortest transition.
  for(int j=0;j<n;j++){
   vector<int>active;for(int h=0;h<=k;h++)if(dp[j][h]<1000000)active.push_back(h);if(active.empty())continue;
   vector<int>cand;for(int z=0;z<k;z++){int l=a[z].len;if(l>j&&!((a[z].color^target)&(low(l)^low(j))))cand.push_back(z);}
   for(int z:cand){int l=a[z].len,q=pos[z];
    for(int h:active){
     int add=0;if(q>=h){int moved=cnt[l][q]-cnt[l][h],upper=cnt[j+1][q]-cnt[j+1][h];add=moved+(upper>moved);}
     int nh=max(q+1,h),value=dp[j][h]+add*256-1;
     if(value<dp[l][nh]){dp[l][nh]=value;parent[l][nh]=j*(k+1)+h;which[l][nh]=z;}
    }
   }
  }
  int h=min_element(dp[n].begin(),dp[n].begin()+k+1)-dp[n].begin(),j=n;
  best=(dp[n][h]+255)/256;vector<int>ans;
  while(j){int z=which[j][h],prev=parent[j][h];ans.push_back(z);j=prev/(k+1);h=prev%(k+1);}
  reverse(ans.begin(),ans.end());return ans;
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
// Among shortest-path moves prefer low inversions by prefix length, which
// makes papers easier to expose on future requests.
inline std::pair<int,int> best_move(const std::vector<int>& order,
                                  const std::vector<int>& length,
                                  const std::vector<char>& selected,
                                  double decay=.5, const std::vector<uint64_t>* access=nullptr) {
    const int K=order.size();
    const int N=*std::max_element(length.begin(),length.end());
    std::vector<int> owner(N+2,-1), next(K), pos(K), suffix(K+1,N+1);
    std::vector<int> initial_bad(K), inversions(K);
    std::vector<char> initial_moved(K), initial_before(K);
    std::vector<double> power(K+1,1.);
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
    uint64_t unions[65][193];
    if(access){
      for(int l=1;l<=N;l++){unions[l][K]=0;
       for(int i=K-1;i>=0;i--){int k=order[i];unions[l][i]=unions[l][i+1]|(length[k]<=l?(*access)[k]:0ULL);}
      }
    }
    double initial_quality=0;
    int initial_inversions=0;
    std::vector<int> fenwick(N+2);
    for(int i=K-1;i>=0;i--){
        int k=order[i];
        for(int x=length[k]-1;x>0;x-=x&-x)inversions[k]+=fenwick[x];
        for(int x=length[k];x<=N;x+=x&-x)fenwick[x]++;
        if(access)initial_quality+=__builtin_popcountll(unions[length[k]][i+1]);
        else initial_quality+=1-power[inversions[k]];
        initial_inversions+=inversions[k];
    }
    std::pair<int,int> answer{-1,-1};double best_quality=std::numeric_limits<double>::infinity();
    int best_inversions=std::numeric_limits<int>::max(), high=0;
    std::vector<char> moved(K),before(K);std::vector<int>bad(K);
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
        {
            moved=initial_moved;before=initial_before;bad=initial_bad;
            int distance=initial_distance, invk=inversions[k], invsum=initial_inversions;
            int following_min=suffix[i+1];double quality=initial_quality;uint64_t own=0;int own_count=0;
            if(access){own=unions[kl][i+1];own_count=__builtin_popcountll(own);}
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
                     int next_count=__builtin_popcountll(unions[kl][d+1]);quality+=next_count-own_count;own_count=next_count;
                     if(xl>=kl)quality+=__builtin_popcountll((*access)[k]&~unions[xl][d+1]);
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
    }
    return answer;
}
}


#ifndef STRUCTURE2_TRAINING_CPU_LIMIT
#define STRUCTURE2_TRAINING_CPU_LIMIT 0.9
#endif
static clock_t structure2_training_origin=0;

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
 struct Node{int child[2]{-1,-1},weight=0,sample=0;std::array<int,9>dp,split;Node(){dp.fill(-100000000);dp[0]=0;split.fill(0);}};
 for(int pass=0;pass<passes;pass++)for(int l=n;l>0;l--){if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>STRUCTURE2_TRAINING_CPU_LIMIT)break;
  int amount=std::min(perlevel,int(extra[l].size()+spare.size()));if(!amount)continue;distances();std::vector<Node>trie(1);
  for(int v=0;v<P;v++){
   auto excluded=fw[v];excluded[l]=INF;
   for(int k:level[l])if(k<fixed)for(int j=start[v][k];j<l;j++)excluded[l]=std::min(excluded[l],excluded[j]+1);
   for(int u=l+1;u<=n;u++){excluded[u]=INF;for(int k:level[u])for(int j=start[v][k];j<u;j++)excluded[u]=std::min(excluded[u],excluded[j]+1);}
   int bypass=excluded[n],node=0,bestprefix=INF,prev=0;
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
 struct Node{int child[2]{-1,-1},weight=0,sample=0;std::array<int,9>dp,split;Node(){dp.fill(-1000000000);dp[0]=0;split.fill(0);}};
 for(int pass=0;pass<passes;pass++)for(int l=N;l>0;l--){if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>STRUCTURE2_TRAINING_CPU_LIMIT)break;
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
  if(!valid){rejected++;continue;}for(int q=0;q<amount;q++){int k=level[l][q];papers[k].color=pattern[q];update(k);}accepted++;
 }
 std::cerr<<"refit "<<initial<<" -> "<<distances()<<" accepted="<<accepted<<" rejected="<<rejected<<"\n";
}

int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int n,K,P,R;if(!(cin>>n>>K>>P>>R)||n!=64||K!=192||P!=256)return 0;
 vector<U64>A(P);for(int i=0;i<P;i++){string s;cin>>s;if(int(s.size())!=n)return 0;for(int j=0;j<n;j++)if(s[j]=='1')A[i]|=1ULL<<j;}
 structure2_training_origin=clock(); mt19937_64 rng(12345);vector<int>cols(n),flip(n);iota(cols.begin(),cols.end(),0);
 int correlation[64][64]{};for(int x=0;x<n;x++)for(int y=0;y<n;y++)for(U64 v:A)correlation[x][y]+=((v>>x)^(v>>y))&1;
 auto cost=[&](){int s=0;for(int j=1;j<n;j++){int d=correlation[cols[j-1]][cols[j]];s+=min(d,P-d);}return s;};
 int current=cost(),best=current;auto bestcols=cols;
 for(int it=0;it<300000;it++){
  int x=rng()%n,y=rng()%n;if(x>y)swap(x,y);(void)(rng()%3);
  auto edge=[&](int a,int b){int d=correlation[a][b];return min(d,P-d);};int next=current;
  if(x)next+=edge(cols[x-1],cols[y])-edge(cols[x-1],cols[x]);
  if(y<63)next+=edge(cols[x],cols[y+1])-edge(cols[y],cols[y+1]);
  reverse(cols.begin()+x,cols.begin()+y+1);double temperature=8*pow(.002,double(it)/300000);
  if(next<=current||double(rng()>>11)*0x1p-53<exp((current-next)/temperature)){current=next;if(current<best)best=current,bestcols=cols;}
  else reverse(cols.begin()+x,cols.begin()+y+1);
 }
 cols=bestcols;for(int j=1;j<n;j++)flip[j]=flip[j-1]^(correlation[cols[j-1]][cols[j]]>P/2);
 for(U64&v:A){U64 old=v;v=0;for(int j=0;j<n;j++)v|=(((old>>cols[j])&1)^flip[j])<<j;}
 vector<PrefixPaper>papers;
 train_backbone_dictionary(A,papers,n,6,1,8);
 refit_all_prefix_papers(A,papers,1,8);
 cerr<<"PrepCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
 if(papers.size()>size_t(K))return 0;
 PrefixDP dp(n,papers);vector<int>order(papers.size()),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);
 int samples=64;vector<uint64_t> future(papers.size());mt19937 freq_rng(912367);
 vector<int>train_targets(P);iota(train_targets.begin(),train_targets.end(),0);shuffle(train_targets.begin(),train_targets.end(),freq_rng);
 for(int r=0;r<128+samples;r++){
  int target=r<128?freq_rng()%P:train_targets[r-128],moves;auto chosen=dp.solve(order,A[target],moves);vector<char>selected(papers.size());for(int k:chosen){selected[k]=true;if(r>=128)future[k]|=1ULL<<(r-128);}
  for(int t=0;t<moves;t++){auto[from,to]=GlobalPrefix::best_move(order,length,selected);if(from<0)abort();int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);}
 }
 cerr<<"PolicyCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<endl;
 iota(order.begin(),order.end(),0);
 for(int k=0;k<K;k++){
  string pattern(n,'.');if(k<int(papers.size()))for(int j=0;j<papers[k].len;j++)pattern[cols[j]]=char('0'+(((papers[k].color>>j)&1)^flip[j]));
  cout<<pattern<<'\n';
 }
 cout.flush();
 for(int r=0;r<R;r++){
  int product;if(!(cin>>product)||product<0||product>=P)return 0;
  int moves;auto chosen=dp.solve(order,A[product],moves);if(moves<0||moves>2*K)return 0;
  vector<char>selected(papers.size());for(int k:chosen)selected[k]=true;
  vector<pair<int,int>>operations;operations.reserve(moves);
  for(int i=0;i<moves;i++){
   auto [from,to]=GlobalPrefix::best_move(order,length,selected,.5,&future);if(from<0)return 0;
   int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);operations.push_back({k,to});
  }
  cout<<operations.size()<<'\n';for(auto [k,d]:operations)cout<<k<<' '<<d<<'\n';cout.flush();
 }
 cerr<<"FinishCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
}


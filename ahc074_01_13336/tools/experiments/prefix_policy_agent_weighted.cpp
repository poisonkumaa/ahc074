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
   vector<int>cand;for(int z=0;z<k;z++){int l=a[z].len;if(l>j&&!((a[z].color^target)&(low(l)^low(j))))cand.push_back(z);}
   for(int z:cand){int l=a[z].len,q=pos[z];
    for(int h=0;h<=k;h++)if(dp[j][h]<1000000){
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
                                  double decay=.5) {
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
    double initial_quality=0;
    int initial_inversions=0;
    std::vector<int> fenwick(N+2);
    for(int i=K-1;i>=0;i--){
        int k=order[i];
        for(int x=length[k]-1;x>0;x-=x&-x)inversions[k]+=fenwick[x];
        for(int x=length[k];x<=N;x+=x&-x)fenwick[x]++;
        initial_quality+=1-power[inversions[k]];
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
            int following_min=suffix[i+1];double quality=initial_quality;
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
                    if(xl<kl){invk--;invsum--;}
                    else if(xl>kl){quality+=power[inversions[x]]-power[inversions[x]+1];invsum++;}
                }else{
                    if(selected[k]&&xl>=kl)change_moved(x,xl>=following_min);
                    if(selected[x]&&kl>=xl)change_moved(k,true);
                    if(!selected[x]&&next[x]==k)change_before(x,false);
                    if(!selected[k]&&next[k]==x)change_before(k,true);
                    if(selected[x])following_min=std::min(following_min,xl);
                    if(xl<kl){invk++;invsum++;}
                    else if(xl>kl){quality-=power[inversions[x]-1]-power[inversions[x]];invsum--;}
                }
                quality+=power[old_invk]-power[invk];
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
vector<U64> trie_patterns(const vector<U64>&A,int l,int cap,int amount){
 struct Node{int child[2]{-1,-1},count=0,sample=0;array<int,5>dp,split;Node(){dp.fill(-1000000);dp[0]=0;split.fill(0);}};
 vector<Node>trie(1);int depth=min(l,cap);
 for(int v=0;v<(int)A.size();v++){int x=0;trie[x].count++;trie[x].sample=v;for(int d=0;d<depth;d++){int b=A[v]>>(l-1-d)&1;if(trie[x].child[b]<0){int y=trie.size();trie[x].child[b]=y;trie.emplace_back();}x=trie[x].child[b];trie[x].count++;trie[x].sample=v;}}
 for(int x=(int)trie.size()-1;x>=0;x--){auto&v=trie[x];int a=v.child[0],b=v.child[1];if(a<0&&b<0){v.dp[1]=v.count;continue;}
  for(int k=1;k<=amount;k++)for(int q=0;q<=k;q++){int r=k-q;if((a<0&&q)||(b<0&&r))continue;int z=(a<0?0:trie[a].dp[q])+(b<0?0:trie[b].dp[r])+v.count;if(z>v.dp[k])v.dp[k]=z,v.split[k]=q;}
 }
 int use=amount;while(use>1&&trie[0].dp[use]<0)use--;
 vector<U64>ans;function<void(int,int)>rec=[&](int x,int k){if(!k)return;auto&v=trie[x];if(v.child[0]<0&&v.child[1]<0){ans.push_back(A[v.sample]&PrefixDP::low(l));return;}int q=v.split[k];if(q)rec(v.child[0],q);if(k-q)rec(v.child[1],k-q);};rec(0,use);return ans;
}

#include <bits/stdc++.h>

// Coordinate optimization of dictionaries against all public products' exact
// minimum segment counts.  No actual order sequence is used.
inline void train_prefix_dictionary(const std::vector<uint64_t>& products,
                                    std::vector<PrefixPaper>& papers,
                                    int n,int cap,int passes,int importance) {
    const int P=products.size(),K=papers.size(),INF=1000;
    std::vector<std::vector<int>> level(n+1),start(P,std::vector<int>(K));
    for(int k=0;k<K;k++)level[papers[k].len].push_back(k);
    auto update_start=[&](int k){
        for(int v=0;v<P;v++){
            uint64_t d=(products[v]^papers[k].color)&PrefixDP::low(papers[k].len);
            start[v][k]=d?64-__builtin_clzll(d):0;
        }
    };
    for(int k=0;k<K;k++)update_start(k);
    std::vector<std::array<int,65>>forward(P),backward(P);
    auto distances=[&](){
        int sum=0;
        for(int v=0;v<P;v++){
            auto&f=forward[v];auto&b=backward[v];f.fill(INF);b.fill(INF);f[0]=0;b[n]=0;
            for(int l=1;l<=n;l++)for(int k:level[l]){
                int best=INF;for(int j=start[v][k];j<l;j++)best=std::min(best,f[j]);
                f[l]=std::min(f[l],best+1);
            }
            for(int l=n;l>0;l--)for(int k:level[l])
                for(int j=start[v][k];j<l;j++)b[j]=std::min(b[j],b[l]+1);
            sum+=f[n];
        }
        return double(sum)/P;
    };
    double initial=distances();
    struct Node {
        int child[2]{-1,-1},weight=0,sample=0;
        std::array<int,5> dp,split;
        Node(){dp.fill(-100000000);dp[0]=0;split.fill(0);}
    };
    for(int pass=0;pass<passes;pass++)for(int z=1;z<=n;z++){
        int l=pass%2?z:n+1-z;
        int amount=level[l].size(),depth=std::min(l,cap);
        if(amount<=1)continue;
        distances();
        std::vector<Node>trie(1);
        for(int v=0;v<P;v++){
            auto excluded=forward[v];excluded[l]=INF;
            for(int upper=l+1;upper<=n;upper++){
                excluded[upper]=INF;
                for(int k:level[upper]){
                    int best=INF;for(int j=start[v][k];j<upper;j++)best=std::min(best,excluded[j]);
                    excluded[upper]=std::min(excluded[upper],best+1);
                }
            }
            int bypass=std::min(n+1,excluded[n]);
            int x=0,best_prefix=INF,previous_benefit=0;
            trie[x].sample=v;
            for(int d=1;d<=depth;d++){
                int bit=products[v]>>(l-d)&1;
                if(trie[x].child[bit]<0){int nx=trie.size();trie[x].child[bit]=nx;trie.emplace_back();}
                x=trie[x].child[bit];trie[x].sample=v;
                best_prefix=std::min(best_prefix,forward[v][l-d]);
                int benefit=std::max(0,bypass-(best_prefix+1+backward[v][l]));
                trie[x].weight+=importance*(benefit-previous_benefit)+1;
                previous_benefit=benefit;
            }
        }
        for(int x=int(trie.size())-1;x>=0;x--){
            auto&node=trie[x];int a=node.child[0],b=node.child[1];
            if(a<0&&b<0){node.dp[1]=node.weight;continue;}
            for(int count=1;count<=amount;count++)for(int left=0;left<=count;left++){
                int right=count-left;if((a<0&&left)||(b<0&&right))continue;
                int value=(a<0?0:trie[a].dp[left])+(b<0?0:trie[b].dp[right])+node.weight;
                if(value>node.dp[count])node.dp[count]=value,node.split[count]=left;
            }
        }
        int use=amount;while(use>1&&trie[0].dp[use]<0)use--;
        std::vector<uint64_t>patterns;
        std::function<void(int,int)>collect=[&](int x,int count){
            if(!count)return;auto&node=trie[x];
            if(node.child[0]<0&&node.child[1]<0){patterns.push_back(products[node.sample]&PrefixDP::low(l));return;}
            int left=node.split[count];if(left)collect(node.child[0],left);if(count-left)collect(node.child[1],count-left);
        };
        collect(0,use);
        if(int(patterns.size())!=amount)continue;
        bool has[2]{};for(auto c:patterns)has[c>>(l-1)&1]=true;
        if(patterns.size()>1&&(!has[0]||!has[1]))patterns[0]^=1ULL<<(l-1);
        for(int q=0;q<amount;q++){int k=level[l][q];papers[k].color=patterns[q];update_start(k);}
    }
    std::cerr<<"intrinsic "<<initial<<" -> "<<distances()<<"\n";
}

struct Candidate{
 array<int,64>cols{},flip{};
 vector<U64>target;
 vector<PrefixPaper>paper;
 vector<int>order,length;
 int score=0,count=0,tsp=0;
};
int main(){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int N,K,P,R;if(!(cin>>N>>K>>P>>R)||N!=64||K!=192||P!=256)return 0;
 vector<U64>input(P);string row;for(auto&v:input){if(!(cin>>row)||row.size()!=64)return 0;for(int j=0;j<64;j++)if(row[j]=='1')v|=U64(1)<<j;}
 clock_t started=clock();mt19937_64 rng(input[0]^input[73]^0xc93b21fe173ULL);
 int diff[64][64]{},weight[64][64]{};
 for(int x=0;x<64;x++)for(int y=0;y<64;y++){for(U64 a:input)diff[x][y]+=((a>>x)^(a>>y))&1;weight[x][y]=min(diff[x][y],P-diff[x][y]);}
 auto ordercols=[&](int attempt){
  array<int,64>cols; iota(cols.begin(),cols.end(),0);if(attempt)shuffle(cols.begin(),cols.end(),rng);
  int value=0;for(int j=1;j<64;j++)value+=weight[cols[j-1]][cols[j]];
  auto bestcols=cols;int best=value;const int STEPS=90000;
  for(int it=0;it<STEPS;it++){
   int x=rng()%64,y=rng()%64;if(x>y)swap(x,y);if(x==y)continue;
   int delta=0;if(x)delta+=weight[cols[x-1]][cols[y]]-weight[cols[x-1]][cols[x]];if(y<63)delta+=weight[cols[x]][cols[y+1]]-weight[cols[y]][cols[y+1]];
   double temp=8*pow(.002,double(it)/STEPS);
   if(delta<=0||double(rng()>>11)*0x1p-53<exp(-delta/temp)){
    reverse(cols.begin()+x,cols.begin()+y+1);value+=delta;if(value<best){best=value;bestcols=cols;}
   }
  }
  return pair{bestcols,best};
 };
 vector<int>training;for(int q=0;q<2;q++){vector<int>block(P);iota(block.begin(),block.end(),0);shuffle(block.begin(),block.end(),rng);if(!training.empty()&&training.back()==block[0])swap(block[0],block[1]);training.insert(training.end(),block.begin(),block.end());}
 auto build=[&](array<int,64>cols,int tsp){
  Candidate c;c.cols=cols;c.tsp=tsp;c.flip[0]=0;
  for(int j=1;j<64;j++)c.flip[j]=c.flip[j-1]^(diff[cols[j-1]][cols[j]]>P/2);
  c.target.assign(P,0);for(int a=0;a<P;a++)for(int j=0;j<64;j++)c.target[a]|=(((input[a]>>cols[j])&1)^c.flip[j])<<j;
  for(int len=1;len<=64;len++){
   auto pat=trie_patterns(c.target,len,16,3);bool zero=false,one=false;
   for(U64 v:pat)if(v>>(len-1)&1)one=true;else zero=true;
   if(pat.size()>1&&(!zero||!one))pat[0]^=U64(1)<<(len-1);
   for(U64 v:pat)c.paper.push_back({len,v});
  }
  train_prefix_dictionary(c.target,c.paper,64,16,1,8);c.order.resize(c.paper.size());iota(c.order.begin(),c.order.end(),0);for(auto a:c.paper)c.length.push_back(a.len);
  return c;
 };
 auto simulate=[&](Candidate&c,int until){
  PrefixDP dp(64,c.paper);
  for(;c.count<until;c.count++){
   int value;auto chosen=dp.solve(c.order,c.target[training[c.count]],value);if(c.count>=16)c.score+=value;
   vector<char>selected(c.paper.size());for(int k:chosen)selected[k]=true;
   for(int it=0;it<value;it++){
    auto[from,to]=GlobalPrefix::best_move(c.order,c.length,selected,.5);if(from<0)abort();int k=c.order[from];c.order.erase(c.order.begin()+from);c.order.insert(c.order.begin()+to,k);
   }
  }
 };
 vector<Candidate>candidates;
 for(int run=0;run<3;run++){
  auto[cols,tsp]=ordercols(run);
  for(int dir=0;dir<2;dir++){
   if(dir)reverse(cols.begin(),cols.end());
   auto c=build(cols,tsp);simulate(c,80);candidates.push_back(move(c));
   if(double(clock()-started)/CLOCKS_PER_SEC>.92)break;
  }
  if(double(clock()-started)/CLOCKS_PER_SEC>.92)break;
 }
 sort(candidates.begin(),candidates.end(),[](const Candidate&a,const Candidate&b){return a.score<b.score;});
 int finalists=min(3,int(candidates.size()));
 for(int i=0;i<finalists;i++){
  if(double(clock()-started)/CLOCKS_PER_SEC>1.05){finalists=i;break;}
  simulate(candidates[i],256);
 }
 int best=0;if(finalists){for(int i=1;i<finalists;i++)if(candidates[i].score<candidates[best].score)best=i;}
 auto c=move(candidates[best]);
 cerr<<"Candidates="<<candidates.size()<<" Finalists="<<finalists<<" Estimate="<<double(c.score)/max(1,c.count-16)<<" PrepCPU="<<double(clock()-started)/CLOCKS_PER_SEC<<endl;
 vector<string>design(192,string(64,'.'));
 for(int k=0;k<(int)c.paper.size();k++)for(int j=0;j<c.paper[k].len;j++)design[k][c.cols[j]]='0'+(((c.paper[k].color>>j)&1)^c.flip[j]);
 for(auto&s:design)cout<<s<<'\n';cout.flush();
 PrefixDP dp(64,c.paper);iota(c.order.begin(),c.order.end(),0);
 for(int r=0;r<R;r++){
  int a;if(!(cin>>a)||a<0||a>=P)return 0;int value;auto chosen=dp.solve(c.order,c.target[a],value);
  vector<char>selected(c.paper.size());for(int k:chosen)selected[k]=true;
  vector<pair<int,int>>ops;ops.reserve(value);
  for(int it=0;it<value;it++){
   auto[from,to]=GlobalPrefix::best_move(c.order,c.length,selected,.5);if(from<0)return 0;int k=c.order[from];ops.emplace_back(k,to);c.order.erase(c.order.begin()+from);c.order.insert(c.order.begin()+to,k);
  }
  if(ops.size()>384)return 0;cout<<ops.size()<<'\n';for(auto[k,d]:ops)cout<<k<<' '<<d<<'\n';cout.flush();
 }
 cerr<<"FinishCPU="<<double(clock()-started)/CLOCKS_PER_SEC<<endl;
}

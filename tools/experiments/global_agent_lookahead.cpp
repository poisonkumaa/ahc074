// AHC074: public-data prefix dictionaries, exact legal reactive paths.
#pragma GCC optimize("O3")
#include <bits/stdc++.h>
using namespace std;using U64=uint64_t;
struct PrefixPaper {int len;U64 color;};
struct PrefixDP {
 int n,k;vector<PrefixPaper>a;
 vector<array<int,193>> cnt,work,parent,which;int pos[192];
 PrefixDP(int n_,vector<PrefixPaper>a_):n(n_),k(a_.size()),a(move(a_)),cnt(n+2),work(n+1),parent(n+1),which(n+1){}
 static U64 low(int n){return n==64?~0ULL:(1ULL<<n)-1;}
 void prepare(const vector<int>&p){
  for(int i=0;i<k;i++)pos[p[i]]=i;
  for(int l=0;l<=n+1;l++){cnt[l][0]=0;for(int i=0;i<k;i++)cnt[l][i+1]=cnt[l][i]+(a[p[i]].len>=l);}
 }
 vector<int> solve(const vector<int>&p,U64 target,int &best,int bias=-1,bool reconstruct=true){prepare(p);return solve_prepared(target,best,bias,reconstruct);}
 vector<int> solve_prepared(U64 target,int &best,int bias=-1,bool reconstruct=true){
  auto&dp=work;
  for(auto&r:dp)r.fill(1000000);dp[0][0]=0;
  // A secondary preference keeps more visible records after a shortest transition.
  for(int j=0;j<n;j++){
   int active[193],ac=0;for(int h=0;h<=k;h++)if(dp[j][h]<1000000)active[ac++]=h;if(!ac)continue;
   int cand[192],cc=0;for(int z=0;z<k;z++){int l=a[z].len;if(l>j&&!((a[z].color^target)&(low(l)^low(j))))cand[cc++]=z;}
   for(int ci=0;ci<cc;ci++){int z=cand[ci],l=a[z].len,q=pos[z];
    for(int ai=0;ai<ac;ai++){int h=active[ai];
     int add=0;if(q>=h){int moved=cnt[l][q]-cnt[l][h],upper=cnt[j+1][q]-cnt[j+1][h];add=moved+(upper>moved);}
     int nh=max(q+1,h),value=dp[j][h]+add*256+bias;
     if(value<dp[l][nh]){dp[l][nh]=value;if(reconstruct){parent[l][nh]=j*(k+1)+h;which[l][nh]=z;}}
    }
   }
  }
  int h=min_element(dp[n].begin(),dp[n].begin()+k+1)-dp[n].begin(),j=n;
  best=(dp[n][h]+128)/256;vector<int>ans;if(!reconstruct)return ans;
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


#include "global_agent_allocate_tune.h"
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int n,K,P,R;if(!(cin>>n>>K>>P>>R)||n!=64||K!=192||P!=256)return 0;
 vector<U64>A(P);for(int i=0;i<P;i++){string s;cin>>s;if(int(s.size())!=n)return 0;for(int j=0;j<n;j++)if(s[j]=='1')A[i]|=1ULL<<j;}
 mt19937_64 rng(12345);vector<int>cols(n),flip(n);iota(cols.begin(),cols.end(),0);
 int correlation[64][64]{};for(int x=0;x<n;x++)for(int y=0;y<n;y++)for(U64 v:A)correlation[x][y]+=((v>>x)^(v>>y))&1;
 auto cost=[&](){int s=0;for(int j=1;j<n;j++){int d=correlation[cols[j-1]][cols[j]];s+=min(d,P-d);}return s;};
 int current=cost(),best=current;auto bestcols=cols;
 for(int it=0;it<300000;it++){
  int x=rng()%n,y=rng()%n;if(x>y)swap(x,y);(void)(rng()%3);
  reverse(cols.begin()+x,cols.begin()+y+1);int next=cost();double temperature=8*pow(.002,double(it)/300000);
  if(next<=current||double(rng()>>11)*0x1p-53<exp((current-next)/temperature)){current=next;if(current<best)best=current,bestcols=cols;}
  else reverse(cols.begin()+x,cols.begin()+y+1);
 }
 cols=bestcols;for(int j=1;j<n;j++)flip[j]=flip[j-1]^(correlation[cols[j-1]][cols[j]]>P/2);
 for(U64&v:A){U64 old=v;v=0;for(int j=0;j<n;j++)v|=(((old>>cols[j])&1)^flip[j])<<j;}
 vector<PrefixPaper>papers;
 for(int length=1;length<=n;length++){
  auto patterns=trie_patterns(A,length,16,3);
  bool has[2]{};for(U64 v:patterns)has[v>>(length-1)&1]=true;
  if(patterns.size()>1&&(!has[0]||!has[1]))patterns[0]^=1ULL<<(length-1);
  for(U64 v:patterns)papers.push_back({length,v});
 }
 train_prefix_dictionary(A,papers,n,argc>6?atoi(argv[6]):16,argc>1?atoi(argv[1]):1,argc>2?atoi(argv[2]):8,argc>3?atoi(argv[3]):0,argc>4?atoi(argv[4]):6,argc>5?atoi(argv[5]):0);
 if(papers.size()>size_t(K))return 0;
 for(int k=0;k<K;k++){
  string pattern(n,'.');if(k<int(papers.size()))for(int j=0;j<papers[k].len;j++)pattern[cols[j]]=char('0'+(((papers[k].color>>j)&1)^flip[j]));
  cout<<pattern<<'\n';
 }
 cout.flush();
 PrefixDP dp(n,papers);vector<int>order(papers.size()),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);
 int samples=argc>7?atoi(argv[7]):8,random_samples=argc>8?atoi(argv[8]):0;
 vector<int>centers{0},closest(P,65),cluster(P),weight(samples);
 auto hamming=[&](int x,int y){return __builtin_popcountll(A[x]^A[y]);};
 while(int(centers.size())<samples){int v=centers.back();for(int x=0;x<P;x++)closest[x]=min(closest[x],hamming(x,v));centers.push_back(max_element(closest.begin(),closest.end())-closest.begin());}
 for(int iter=0;iter<4;iter++){
  fill(weight.begin(),weight.end(),0);for(int v=0;v<P;v++){int b=0;for(int c=1;c<samples;c++)if(hamming(v,centers[c])<hamming(v,centers[b]))b=c;cluster[v]=b;weight[b]++;}
  if(iter==3)break;for(int c=0;c<samples;c++){int best=INT_MAX,medoid=centers[c];for(int u=0;u<P;u++)if(cluster[u]==c){int sum=0;for(int v=0;v<P;v++)if(cluster[v]==c)sum+=hamming(u,v);if(sum<best)best=sum,medoid=u;}centers[c]=medoid;}
 }
 int changed=0,different=0;clock_t lookupCPU=0;mt19937 future_random(92333);
 for(int r=0;r<R;r++){
  int product;if(!(cin>>product)||product<0||product>=P)return 0;
  struct Candidate{vector<int>order;vector<pair<int,int>>operations;};
  auto simulate=[&](int bias){Candidate c{order,{}};int moves;auto chosen=dp.solve(order,A[product],moves,bias);if(moves<0||moves>2*K)exit(1);vector<char>selected(papers.size());for(int k:chosen)selected[k]=true;c.operations.reserve(moves);for(int i=0;i<moves;i++){auto [from,to]=GlobalPrefix::best_move(c.order,length,selected);if(from<0)exit(2);int k=c.order[from];c.order.erase(c.order.begin()+from);c.order.insert(c.order.begin()+to,k);c.operations.push_back({k,to});}return c;};
  auto baseline=simulate(-1),alternative=simulate(1);
  if(baseline.operations.size()!=alternative.operations.size())return 3;
  if(baseline.order!=alternative.order){
   different++;clock_t started=clock();int scores[2]{};
   vector<pair<int,int>>future_targets;
   for(int i=0;i<samples;i++){
    int v=centers[i],w=weight[i]-(cluster[product]==i);
    if(random_samples){v=future_random()%(P-1);v+=v>=product;w=1;}
    else if(v==product){v=-1;for(int j=0;j<P;j++)if(j!=product&&cluster[j]==i&&(v<0||hamming(j,product)<hamming(v,product)))v=j;if(v<0)continue;}
    future_targets.push_back({v,w});
   }
   dp.prepare(baseline.order);for(auto [v,w]:future_targets){int d;dp.solve_prepared(A[v],d,-1,false);scores[0]+=w*d;}
   dp.prepare(alternative.order);for(auto [v,w]:future_targets){int d;dp.solve_prepared(A[v],d,-1,false);scores[1]+=w*d;}
   if(scores[1]<scores[0]){baseline=move(alternative);changed++;}lookupCPU+=clock()-started;
  }
  order=move(baseline.order);auto&operations=baseline.operations;
  cout<<operations.size()<<'\n';for(auto [k,d]:operations)cout<<k<<' '<<d<<'\n';cout.flush();
 }
 cerr<<"lookahead different="<<different<<" changed="<<changed<<" cpu="<<double(lookupCPU)/CLOCKS_PER_SEC<<" totalcpu="<<double(clock())/CLOCKS_PER_SEC<<"\n";
}

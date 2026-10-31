// AHC074: public-data prefix dictionaries, exact legal reactive paths.
#pragma GCC optimize("O3")
#include <bits/stdc++.h>
using namespace std;using U64=uint64_t;
struct PrefixPaper {int len;U64 color;};
struct PrefixDP {
 int n,k;vector<PrefixPaper>a;
 vector<array<int,193>>cnt,work,parent,which,opcount;int pos[192];
 vector<array<int,65>>compatible,penalty;bool learned=false;int primary=1000000;
 static constexpr int F=27;
 PrefixDP(int nn,vector<PrefixPaper>aa):n(nn),k(aa.size()),a(move(aa)),cnt(n+2),work(n+1),parent(n+1),which(n+1),opcount(n+1),compatible(k),penalty(k){}
 static U64 low(int n){return n==64?~0ULL:(1ULL<<n)-1;}
 static int bucket(int span){return span<=4?span-1:span<=8?4:5;}
 int bin(int z,int j)const{return ((a[z].len-1)/16)*6+bucket(a[z].len-j);}
 void public_data(const vector<U64>&products){for(int z=0;z<k;z++){int hist[65]{};for(U64 v:products){U64 d=(v^a[z].color)&low(a[z].len);hist[d?64-__builtin_clzll(d):0]++;}int sum=0;for(int j=0;j<=64;j++)compatible[z][j]=(sum+=hist[j]);}}
 array<double,F>features(const vector<int>&p)const{array<double,F>f{};int j=0;for(int z:p){int l=a[z].len;if(l<=j)continue;f[bin(z,j)]++;double match=compatible[z][j]/256.;f[24]+=match;f[25]+=match*(l-j);j=l;}f[26]=1;return f;}
 void set_weights(const array<double,F>&w,double regular){for(int z=0;z<k;z++)for(int j=0;j<a[z].len;j++){double match=compatible[z][j]/256.;double c=w[bin(z,j)]+w[24]*match+w[25]*match*(a[z].len-j)-regular;penalty[z][j]=lround(1000*c);}learned=true;}
 void prepare(const vector<int>&p){for(int i=0;i<k;i++)pos[p[i]]=i;for(int l=0;l<=n+1;l++){cnt[l][0]=0;for(int i=0;i<k;i++)cnt[l][i+1]=cnt[l][i]+(a[p[i]].len>=l);}}
 vector<int>solve(const vector<int>&p,U64 target,int&best,bool reconstruct=true){prepare(p);return solve_prepared(target,best,reconstruct);}
 vector<int>solve_prepared(U64 target,int&best,bool reconstruct=true){
  auto&dp=work;for(auto&r:dp)r.fill(1000000000);dp[0][0]=0;opcount[0][0]=0;
  for(int j=0;j<n;j++){
   int active[193],ac=0;for(int h=0;h<=k;h++)if(dp[j][h]<1000000000)active[ac++]=h;if(!ac)continue;
   int cand[192],cc=0;for(int z=0;z<k;z++)if(a[z].len>j&&!((a[z].color^target)&(low(a[z].len)^low(j))))cand[cc++]=z;
   for(int ci=0;ci<cc;ci++){int z=cand[ci],l=a[z].len,q=pos[z],quality=learned?penalty[z][j]:-1;
    for(int ai=0;ai<ac;ai++){int h=active[ai],add=0;if(q>=h){int moved=cnt[l][q]-cnt[l][h],upper=cnt[j+1][q]-cnt[j+1][h];add=moved+(upper>moved);}int nh=max(q+1,h),value=dp[j][h]+add*primary+quality;
     if(value<dp[l][nh]){dp[l][nh]=value;opcount[l][nh]=opcount[j][h]+add;if(reconstruct){parent[l][nh]=j*(k+1)+h;which[l][nh]=z;}}
    }
   }
  }
  int h=min_element(dp[n].begin(),dp[n].begin()+k+1)-dp[n].begin(),j=n;best=opcount[n][h];vector<int>ans;if(!reconstruct)return ans;
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
                                    int n,int cap,int passes,int importance,int price,int maxpapers=6,int shape=0) {
    const int P=products.size(),K=papers.size(),INF=1000;
    std::vector<int> spare;
    bool has[65][2]{};
    for(int k=0;k<K;k++){int l=papers[k].len,b=papers[k].color>>(l-1)&1;if(has[l][b]){papers[k].len=0;papers[k].color=0;spare.push_back(k);}else has[l][b]=true;}
    std::vector<std::vector<int>> level(n+1),start(P,std::vector<int>(K));
    for(int k=0;k<K;k++)if(papers[k].len)level[papers[k].len].push_back(k);
    std::vector<int> plan(n+1,2);
    if(shape){std::vector<std::pair<int,int>> rank;for(int l=2;l<=n;l++)for(int q=0;q<maxpapers-2;q++)rank.push_back({l-shape*q,l});std::sort(rank.rbegin(),rank.rend());for(int j=0;j<int(spare.size());j++)plan[rank[j].second]++;}
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
        std::array<int,7> dp,split;
        Node(){dp.fill(-100000000);dp[0]=0;split.fill(0);}
    };
    for(int pass=0;pass<passes;pass++)for(int z=1;z<=n;z++){
        int l=n+1-z;
        int previous_amount=level[l].size(); int amount=std::min(shape?plan[l]:maxpapers,previous_amount+int(spare.size())),depth=std::min(l,cap);
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
        int use=previous_amount;
        double unit_price=price*std::max(.35,double(l-1)/std::max(1,int(spare.size())));
        for(int q=2;q<=amount;q++)if(trie[0].dp[q]-unit_price*q>trie[0].dp[use]-unit_price*use)use=q;
        if(trie[0].dp[use]<0){use=amount;while(use>1&&trie[0].dp[use]<0)use--;}
        std::vector<uint64_t>patterns;
        std::function<void(int,int)>collect=[&](int x,int count){
            if(!count)return;auto&node=trie[x];
            if(node.child[0]<0&&node.child[1]<0){patterns.push_back(products[node.sample]&PrefixDP::low(l));return;}
            int left=node.split[count];if(left)collect(node.child[0],left);if(count-left)collect(node.child[1],count-left);
        };
        collect(0,use);
        amount=patterns.size();
        while(int(level[l].size())>amount){int k=level[l].back();level[l].pop_back();papers[k].len=0;papers[k].color=0;spare.push_back(k);}
        while(int(level[l].size())<amount){int k=spare.back();spare.pop_back();level[l].push_back(k);papers[k].len=l;}
        bool has[2]{};for(auto c:patterns)has[c>>(l-1)&1]=true;
        if(patterns.size()>1&&(!has[0]||!has[1]))patterns[0]^=1ULL<<(l-1);
        for(int q=0;q<amount;q++){int k=level[l][q];papers[k].color=patterns[q];update_start(k);}
    }
    std::cerr<<"intrinsic "<<initial<<" -> "<<distances()<<" papers="<<K-spare.size()<<"\n";
    std::vector<PrefixPaper> compact;for(auto p:papers)if(p.len)compact.push_back(p);
    std::stable_sort(compact.begin(),compact.end(),[](auto a,auto b){return a.len<b.len;});papers=compact;
}

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
 train_prefix_dictionary(A,papers,n,64,3,8,0,6,0);
 if(papers.size()>size_t(K))return 0;
 PrefixDP dp(n,papers);vector<int>order(papers.size()),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);
 dp.public_data(A);vector<array<double,PrefixDP::F>>features;vector<double>value;
 mt19937 vrng(972451);vector<int>sequence(P);iota(sequence.begin(),sequence.end(),0);shuffle(sequence.begin(),sequence.end(),vrng);
 vector<int>future=sequence;shuffle(future.begin(),future.end(),vrng);int samples=argc>2?atoi(argv[2]):16;
 for(int r=0;r<P+32;r++){
  int product=r<32?vrng()%P:sequence[r-32],moves;auto chosen=dp.solve(order,A[product],moves);vector<char>selected(papers.size());for(int k:chosen)selected[k]=true;
  for(int it=0;it<moves;it++){auto[from,to]=GlobalPrefix::best_move(order,length,selected);if(from<0)abort();int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);}
  if(r<32)continue;
  features.push_back(dp.features(order));dp.prepare(order);double avg=0;for(int s=0;s<samples;s++){int cost;dp.solve_prepared(A[future[s]],cost,false);avg+=cost;}value.push_back(avg/samples);
 }
 constexpr int F=PrefixDP::F;double matrix[F][F+1]{};double lambda=argc>1?atof(argv[1]):20;
 for(int v=0;v<(int)value.size();v++)for(int i=0;i<F;i++){matrix[i][F]+=features[v][i]*value[v];for(int j=0;j<F;j++)matrix[i][j]+=features[v][i]*features[v][j];}
 for(int i=0;i<F-1;i++)matrix[i][i]+=lambda;
 for(int i=0;i<F;i++){int p=i;for(int j=i+1;j<F;j++)if(abs(matrix[j][i])>abs(matrix[p][i]))p=j;for(int j=i;j<=F;j++)swap(matrix[i][j],matrix[p][j]);double x=matrix[i][i];for(int j=i;j<=F;j++)matrix[i][j]/=x;for(int r=0;r<F;r++)if(r!=i){double x=matrix[r][i];for(int j=i;j<=F;j++)matrix[r][j]-=x*matrix[i][j];}}
 array<double,F>weights;for(int i=0;i<F;i++)weights[i]=matrix[i][F];double mean=accumulate(value.begin(),value.end(),0.)/value.size(),sse=0,variance=0;
 for(int v=0;v<(int)value.size();v++){double pred=inner_product(weights.begin(),weights.end(),features[v].begin(),0.);sse+=(pred-value[v])*(pred-value[v]);variance+=(value[v]-mean)*(value[v]-mean);}
 cerr<<"Fit R2="<<1-sse/variance<<" mean="<<mean<<" weights=";for(double w:weights)cerr<<w<<",";cerr<<" CPU="<<double(clock())/CLOCKS_PER_SEC<<endl;
 double regular=argc>3?atof(argv[3]):.1;dp.set_weights(weights,regular);if(argc>4)dp.primary=atoi(argv[4]);iota(order.begin(),order.end(),0);
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
   auto [from,to]=GlobalPrefix::best_move(order,length,selected);if(from<0)return 0;
   int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);operations.push_back({k,to});
  }
  cout<<operations.size()<<'\n';for(auto [k,d]:operations)cout<<k<<' '<<d<<'\n';cout.flush();
 }
 cerr<<"FinishCPU="<<double(clock())/CLOCKS_PER_SEC<<endl;
}

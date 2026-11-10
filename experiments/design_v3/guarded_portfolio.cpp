// AHC074 Layered Barcode Display. Build with g++ -std=c++17 -O2.
// v12 blends the relation-count price with the legal mean distance over all
// backing orders for both products; home repair demand is updated from
// finished orders. All live decisions use only the disclosed catalogue
// and orders received so far.
// The design re-scores the leading comparison windows after exchanging product
// partners, exchanges two or three primary templates between compatible slots,
// and can switch paired requests without lifting their shared main.
// The design reserves color-biased columns for anchors and samples physical bank
// heads before choosing prefix holes. A previous main and another visible
// sheet can participate in the backing stack for a paired order.
// Compare layouts with five or six backing sheets. In the window search,
// update pair orientations in bitsets and keep independent cycle certificates.
// When routing an order, try covering a group of final repairs with one donor.
// Only physically legal routes with the target display and a reusable deck pass.
#if defined(__GNUC__)
#pragma GCC optimize("O3")
#endif
// Count the 64-window masks with the hardware instruction on x86 judges.
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC target("popcnt")
#endif
// Exact accelerations: DAG metric memoization, backing route reuse,
// prefix-hole marginal evaluation and prefix-coverage trace compression.
#include <unordered_map>
#ifndef AHC074_PREFIX_HOLE_OPTIMIZER_HPP
#define AHC074_PREFIX_HOLE_OPTIMIZER_HPP
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <string>
#include <vector>


#include <cstdlib>
// All configured times use local experiment units. Work quotas are identical
// on both platforms; only optional-work deadlines are scaled. The Linux
// default keeps the last optional search cutoff at 1.80 native clock seconds.
namespace search_budget {
inline double real(const char* name,double fallback) {
    const char* value=std::getenv(name);return value?std::atof(value):fallback;
}
inline int integer(const char* name,int fallback) {
    const char* value=std::getenv(name);return value?std::atoi(value):fallback;
}
inline double scale() {
#if defined(_WIN32)
    constexpr double maximum=1.0;
#else
    constexpr double maximum=.60;
#endif
    static const double factor=[](){
#if defined(_WIN32)
        constexpr double ceiling=1.0;
#else
        constexpr double ceiling=.60;
#endif
        const double value=real("NOV8_TIME_SCALE",ceiling);
        return std::isfinite(value)?std::max(.1,std::min(ceiling,value)):ceiling;
    }();
    (void)maximum;
    return factor;
}
inline double seconds(double local){return local*scale();}
}

namespace prefix_holes {
using U=std::uint64_t;
struct Options {
    int max_total=16,max_per_paper=1;
    // 0: rank0 oracle; 1: uniform bucket rank; 2: geometric .6; 3: initial rank.
    int rank_mode=1;
    // 0: sum support expectations; 1: max support expectation per product.
    int objective_mode=0;
    const std::vector<std::array<double,32>>*rank_weights=nullptr;
    double own_hole_cost=.5,minimum_gain=0;
    std::clock_t deadline=0; // absolute process CPU; 0 keeps unlimited behavior
};
struct Erasure {int paper,column;double utility_gain,own_cost,net_gain;};
struct Result {std::vector<Erasure>erasures;double utility_gain=0,own_cost=0,net_gain=0;bool timed_out=false;};

inline Result optimize(std::vector<std::string>&S,const std::vector<U>&A,
                       const std::vector<int>&main_of,
                       const std::vector<int>&anchor,
                       int backings,int prefix_size,int first_anchor,
                       const Options&options) {
    const int K=S.size(),P=A.size(),T=prefix_size;
    Result answer;if(options.max_total<=0||options.max_per_paper<=0)return answer;
    auto expired=[&](){if(options.deadline>0 && std::clock()>=options.deadline){answer.timed_out=true;return true;}return false;};
    if(expired())return answer;
    unsigned budget_checks=0;
    U prefix=((U(1)<<T)-1)^((U(1)<<first_anchor)-1);
    std::vector<U>mask(K),black(K),holes(P);
    std::vector<std::vector<int>>products(K),buckets(T);
    std::vector<int>initial_rank(K),erased(K);
    for(int k=0;k<K;k++){
        for(int j=first_anchor;j<T;j++){
            if(S[k][j]!='.')mask[k]|=U(1)<<j;
            if(S[k][j]=='1')black[k]|=U(1)<<j;
        }
        if(k>=backings&&anchor[k]>=first_anchor){initial_rank[k]=buckets[anchor[k]].size();buckets[anchor[k]].push_back(k);}
    }
    for(int a=0;a<P;a++){products[main_of[a]].push_back(a);holes[a]=prefix&~mask[main_of[a]];}
    double expectation[65][32]={};
    for(int g=0;g<=T;g++){
        double probability=std::ldexp(1.0,-g);
        for(int mismatches=0;mismatches<=g;mismatches++){
            for(int r=0;r<32;r++)expectation[g][r]+=probability*std::max(0,mismatches-r-2);
            if(mismatches<g)probability*=double(g-mismatches)/(mismatches+1);
        }
    }
    std::vector<std::array<double,65>>gain(K);
    for(int k=backings;k<K;k++)if(anchor[k]>=first_anchor){
        int size=buckets[anchor[k]].size();
        double weight[32]={},sum=0;
        for(int r=0;r<size&&r<32;r++){
            if(options.rank_mode==0)weight[r]=(r==0);
            else if(options.rank_mode==1)weight[r]=1;
            else if(options.rank_mode==2)weight[r]=std::pow(.6,r);
            else if(options.rank_mode==3)weight[r]=(r==initial_rank[k]);
            else if(options.rank_weights)weight[r]=(*options.rank_weights)[k][r];
            else weight[r]=1;
            sum+=weight[r];
        }
        if(sum==0){for(int r=0;r<size&&r<32;r++)weight[r]=1;sum=std::min(size,32);}
        for(int g=0;g<=T;g++)for(int r=0;r<size&&r<32;r++)gain[k][g]+=weight[r]/sum*expectation[g][r];
    }
    auto value=[&](int k,int a,U opaque,U target_holes)->double {
        int w=anchor[k];if(w<first_anchor||!(target_holes&(U(1)<<w)))return 0;
        U bad=(black[k]^A[a])&opaque&target_holes;
        int cut=bad?63-__builtin_clzll(bad):first_anchor-1;
        if(cut>=w)return 0;
        U cover=opaque&target_holes&(~U(0)<<(cut+1))&~(U(1)<<w);
        return gain[k][__builtin_popcountll(cover)];
    };
    if(options.objective_mode==1){
        std::vector<std::vector<double>>v(K,std::vector<double>(P));
        std::vector<double>best(P),second(P);std::vector<int>owner(P,-1);
        auto rebuild=[&]()->bool{
            std::fill(best.begin(),best.end(),0);std::fill(second.begin(),second.end(),0);std::fill(owner.begin(),owner.end(),-1);
            for(int k=backings;k<K;k++)if(anchor[k]>=first_anchor)for(int a=0;a<P;a++){
                if(a==0 && (k&7)==0 && expired())return false;
                double x=v[k][a]=value(k,a,mask[k],holes[a]);
                if(x>best[a]){second[a]=best[a];best[a]=x;owner[a]=k;}else if(x>second[a])second[a]=x;
            }
            return true;
        };
        if(!rebuild())return answer;
        for(int step=0;step<options.max_total;step++){
            // Accumulate foreign-product changes in the original product order.
            // Removing a non-highest bad cell has no effect. Removing a good
            // covered cell can lower the catalogue maximum only for its owner.
            std::vector<std::array<double,64>>foreign(K);
            for(int k=backings;k<K;k++)if(anchor[k]>=first_anchor&&erased[k]<options.max_per_paper){
                if((k&7)==0 && expired())return answer;
                int w=anchor[k];U low=(U(1)<<w)-1,anchor_bit=U(1)<<w;
                for(int a=0;a<P;a++)if(main_of[a]!=k&&(holes[a]&anchor_bit)){
                    U bad=(black[k]^A[a])&mask[k]&holes[a];
                    int cut=bad?63-__builtin_clzll(bad):first_anchor-1;
                    if(cut>=w)continue;
                    if(cut>=first_anchor){
                        double without=owner[a]==k?second[a]:best[a];
                        double with=value(k,a,mask[k]&~(U(1)<<cut),holes[a]);
                        foreign[k][cut]+=std::max(without,with)-best[a];
                    }
                    if(owner[a]==k && second[a]<best[a]){
                        U cover=mask[k]&holes[a]&(~U(0)<<(cut+1))&~anchor_bit;
                        int g=__builtin_popcountll(cover);
                        if(g){
                            double change=std::max(second[a],gain[k][g-1])-best[a];
                            U bits=cover&low;
                            while(bits){int j=__builtin_ctzll(bits);bits&=bits-1;foreign[k][j]+=change;}
                        }
                    }
                }
            }
            // An added hole contributes at most one further covered cell.
            // Ignore all mismatches to obtain an upper bound, then visit the
            // promising supports first. Max is independent of visit order.
            std::vector<std::vector<std::pair<double,int>>>upper(P);
            for(int a=0;a<P;a++){
                auto&list=upper[a];list.reserve(K-backings);
                for(int q=backings;q<K;q++)if(anchor[q]>=first_anchor){
                    U covered=mask[q]&holes[a]&~(U(1)<<anchor[q]);
                    double bound=gain[q][__builtin_popcountll(covered)+1];
                    if(bound>0)list.push_back({bound,q});
                }
                std::sort(list.begin(),list.end(),[](const auto&x,const auto&y){return x.first>y.first;});
            }
            int chosen=-1,column=-1;double best_delta=options.minimum_gain;
            for(int k=backings;k<K;k++)if(anchor[k]>=first_anchor&&erased[k]<options.max_per_paper)
                for(int j=first_anchor;j<anchor[k];j++)if(mask[k]>>j&1){
                    if((++budget_checks&15)==0 && expired())return answer;
                    U bit=U(1)<<j,new_mask=mask[k]&~bit;double delta=foreign[k][j];
                    for(int a:products[k]){
                        double after=0;U h=holes[a]|bit;
                        for(const auto&entry:upper[a]){
                            if(entry.first<=after)break;
                            int q=entry.second;
                            after=std::max(after,value(q,a,q==k?new_mask:mask[q],h));
                        }
                        delta+=after-best[a];
                    }
                    delta-=options.own_hole_cost*products[k].size();
#ifdef NOV3_HOLES_CHECK
                    double exact=0;
                    for(int a=0;a<P;a++)if(main_of[a]!=k){
                        double without=owner[a]==k?second[a]:best[a];
                        exact+=std::max(without,value(k,a,new_mask,holes[a]))-best[a];
                    }
                    for(int a:products[k]){
                        double after=0;U h=holes[a]|bit;
                        for(int q=backings;q<K;q++)if(anchor[q]>=first_anchor)
                            after=std::max(after,value(q,a,q==k?new_mask:mask[q],h));
                        exact+=after-best[a];
                    }
                    exact-=options.own_hole_cost*products[k].size();
                    assert(exact==delta);
#endif
                    if(delta>best_delta+1e-12){best_delta=delta;chosen=k;column=j;}
                }
            if(expired())return answer;
            if(chosen<0)break;
            double before=0;for(double x:best)before+=x;
            U bit=U(1)<<column;mask[chosen]&=~bit;S[chosen][column]='.';erased[chosen]++;
            for(int a:products[chosen])holes[a]|=bit;
            double own=options.own_hole_cost*products[chosen].size();
            answer.erasures.push_back({chosen,column,best_delta+own,own,best_delta});
            answer.utility_gain+=best_delta+own;answer.own_cost+=own;answer.net_gain+=best_delta;
            if(!rebuild())return answer;
#ifdef PREFIX_HOLES_VERIFY
            double after=0;for(double x:best)after+=x;
            assert(std::abs(after-before-own-best_delta)<1e-8);
#endif
        }
        return answer;
    }
    auto marginal=[&](int k,int j)->double {
        U bit=U(1)<<j,new_mask=mask[k]&~bit;double delta=0;
        for(int a=0;a<P;a++){
            U next_holes=holes[a]|(main_of[a]==k?bit:0);
            delta+=value(k,a,new_mask,next_holes)-value(k,a,mask[k],holes[a]);
        }
        for(int a:products[k])for(int q=backings;q<K;q++)if(q!=k&&anchor[q]>=first_anchor)
            delta+=value(q,a,mask[q],holes[a]|bit)-value(q,a,mask[q],holes[a]);
        return delta-options.own_hole_cost*products[k].size();
    };
    struct Candidate {int paper,column;double net;};
    std::vector<Candidate>candidates;
    for(int k=backings;k<K;k++)if(anchor[k]>=first_anchor)
        for(int j=first_anchor;j<anchor[k];j++)if(mask[k]>>j&1){
            if((++budget_checks&15)==0 && expired())return answer;
            candidates.push_back({k,j,marginal(k,j)});
        }

#ifdef PREFIX_HOLES_VERIFY
    auto objective=[&](){double v=0;for(int k=backings;k<K;k++)if(anchor[k]>=first_anchor)for(int a=0;a<P;a++)v+=value(k,a,mask[k],holes[a]);for(int a=0;a<P;a++)v-=options.own_hole_cost*__builtin_popcountll(holes[a]);return v;};
    double objective_before=objective();
#endif

    for(int step=0;step<options.max_total;step++){
        int best=-1;double best_gain=options.minimum_gain;
        for(int i=0;i<int(candidates.size());i++){
            auto&c=candidates[i];if(erased[c.paper]>=options.max_per_paper||!(mask[c.paper]>>c.column&1))continue;
            if(c.net>best_gain+1e-12){best_gain=c.net;best=i;}
        }
        if(expired())return answer;
        if(best<0)break;
        auto chosen=candidates[best];int q=chosen.paper,t=chosen.column;U bit=U(1)<<t,old_q=mask[q];
#ifdef PREFIX_HOLES_VERIFY
        assert(std::abs(marginal(q,t)-chosen.net)<1e-8);
#endif
        mask[q]&=~bit;S[q][t]='.';erased[q]++;
        for(int a:products[q])holes[a]|=bit;
        double own=options.own_hole_cost*products[q].size();
        answer.erasures.push_back({q,t,chosen.net+own,own,chosen.net});
        answer.utility_gain+=chosen.net+own;answer.own_cost+=own;answer.net_gain+=chosen.net;
        for(auto&c:candidates){
            if(expired())return answer;
            int k=c.paper,j=c.column;
            if(erased[k]>=options.max_per_paper||!(mask[k]>>j&1))continue;
            if(k==q){c.net=marginal(k,j);continue;}
            U candidate_bit=U(1)<<j,new_k=mask[k]&~candidate_bit;double correction=0;
            // The candidate's support row intersects the changed target columns.
            for(int a:products[q]){
                U new_h=holes[a],old_h=new_h&~bit;
                correction+=value(k,a,new_k,new_h)-value(k,a,mask[k],new_h)
                           -value(k,a,new_k,old_h)+value(k,a,mask[k],old_h);
            }
            // The changed support row intersects the candidate's target columns.
            for(int a:products[k]){
                U old_h=holes[a],new_h=old_h|candidate_bit;
                correction+=value(q,a,mask[q],new_h)-value(q,a,mask[q],old_h)
                           -value(q,a,old_q,new_h)+value(q,a,old_q,old_h);
            }
            c.net+=correction;
        }
#ifdef PREFIX_HOLES_VERIFY
        assert(std::abs(objective()-objective_before-answer.net_gain)<1e-7);
        for(auto&c:candidates)if(erased[c.paper]<options.max_per_paper&&(mask[c.paper]>>c.column&1))assert(std::abs(c.net-marginal(c.paper,c.column))<1e-7);
#endif
    }
    return answer;
}
}
#endif

// AHC074: seven comparison sheets, joint product pairing/anchor optimization,
// support-aware main-sheet queues, and shortest backing routes with a
// catalog-based future-value tie-break. Development choices are fixed here.
#ifndef MIGRATE_GAIN
#define MIGRATE_GAIN 0
#endif
#include <optional>
#include <array>
#include <map>
#include <tuple>
#include <memory>
// AHC074 Layered Barcode Display -- comparison backings, 2026-11-02.
// B=7 comparison sheets, balanced endpoint colors, three graph designs.
// Main sheets share product pairs; their holes impose a partial ordering.
// Complete comparison graphs use exact legal permutation distances.
// 4096 independent synthetic requests select the design before any real order.
// A bounded constructor and independent-bit fallback preserve feasibility.
// Input-dependent pairing + shared backings + bucket layout optimization.
// Reposition incompatible supports and expose them with only necessary returns.
// Joint layout evaluation, cached supports, and pre-acquisition before main promotion.
// Weighted queue insertion, exact backing choices, and occlusion-aware supports.
// The paper budget is 7 backings + 71 paired mains + 114 singleton mains.
// No future orders are read before the current response is flushed.
// If construction fails, a 128-sheet independent-bit design guarantees a feasible response.
// Construction and online policy inherited from the user checkpoint.

#ifndef MIN_GAIN
#define MIN_GAIN 0
#endif
#include <cmath>
#ifndef HEAD_PENALTY
#define HEAD_PENALTY 16
#endif
#ifndef NOW_WEIGHT
#define NOW_WEIGHT 64
#endif
#include <ctime>
#define SUPPORT_LIMIT 3
#define RETURN_PENALTY 1
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <cmath>

using namespace std;

constexpr int N = 64;
constexpr int K = 192;
constexpr int P = 256;
constexpr int R = 1024;

struct DeckTracker {
    vector<string> S;
    vector<int> order, position;
    vector<uint64_t> mask_opaque, mask_black;

    void init(const vector<string>& s_init) {
        S = s_init;
        mask_opaque.assign(K,0);mask_black.assign(K,0);
        for(int k=0;k<K;k++)for(int j=0;j<N;j++) {
            if(S[k][j]!='.')mask_opaque[k]|=1ULL<<j;
            if(S[k][j]=='1')mask_black[k]|=1ULL<<j;
        }
        order.resize(K);
        for (int i = 0; i < K; ++i) order[i] = i;
        rebuild_positions();
    }

    void rebuild_positions() {
        position.resize(K);
        for(int d=0;d<K;d++)position[order[d]]=d;
    }
    int depth_of(int k) const { return position[k]; }

    void move_paper(int k, int d_target, vector<pair<int, int>>& moves) {
        int cur_d = depth_of(k);
        if (cur_d == d_target) return;
#ifdef ROUTE_ASSERT
        uint64_t seen=0;for(int d=0;d<cur_d;d++)seen|=mask_opaque[order[d]];
        assert(mask_opaque[k]&~seen);
#endif
        if(cur_d<d_target)
            std::move(order.begin()+cur_d+1,order.begin()+d_target+1,order.begin()+cur_d);
        else
            std::move_backward(order.begin()+d_target,order.begin()+cur_d,order.begin()+cur_d+1);
        order[d_target]=k;
        for(int d=min(cur_d,d_target);d<=max(cur_d,d_target);d++)position[order[d]]=d;
        moves.push_back({k, d_target});
    }

    uint64_t current_display() const {
        uint64_t value=0,seen=0;
        for(int k:order) {
            value|=mask_black[k]&~seen;
            seen|=mask_opaque[k];
            if(seen==~0ULL)break;
        }
        return value;
    }

};

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

// Result[i] corresponds to barcode window 64-W+i.
// First endpoint prints 0, second endpoint prints 1.
// Control c uses (2*c:0,2*c+1:1), and is excluded from this pool.
inline std::vector<std::pair<int,int>> make_comparison_edges(uint64_t a0,int B,int W,bool include_control=false) {
    uint64_t state=0x9e3779b97f4a7c15ULL^a0;
    if(!state)state=0x9e3779b97f4a7c15ULL;
    auto random32=[&](){state^=state<<7;state^=state>>9;return uint32_t(state);};
    std::vector<std::pair<int,int>> base,answer;
    for(int u=0;u<B;u++)for(int v=u+1;v<B;v++)if(include_control||(u^1)!=v)base.push_back({u,v});
    while(int(answer.size())<W) {
        auto pool=base;
        for(int i=int(pool.size())-1;i>0;i--)std::swap(pool[i],pool[random32()%(i+1)]);
        for(auto e:pool){answer.push_back(e);if(int(answer.size())==W)break;}
    }
    return answer;
}

// before[u] is the set of predecessors of backing u.
// Opposite pair endpoint requires the complete reversal of this DAG.
inline bool comparison_pair_dag(uint64_t a,uint64_t b,int B,int W,
        const std::vector<std::pair<int,int>>& edges,uint16_t* before,int controls=-1) {
    std::fill(before,before+B,uint16_t(0));
    uint64_t diff=a^b;
    auto edge=[&](int zero,int one,int bit) {
        int win=bit?one:zero,lose=bit?zero:one;
        before[lose]|=uint16_t(1)<<win;
    };
    if(controls<0)controls=B/2;
    for(int c=0;c<controls;c++)if((diff>>c)&1)edge(2*c,2*c+1,(a>>c)&1);
    int T=64-W;uint64_t need=diff>>T;
    while(need) {
        int i=__builtin_ctzll(need);need&=need-1;
        edge(edges[i].first,edges[i].second,(a>>(T+i))&1);
    }
    uint16_t alive=(uint16_t(1)<<B)-1;
    for(int it=0;it<B;it++) {
        int take=-1;
        for(int k=0;k<B;k++)if((alive&(uint16_t(1)<<k))&&!(before[k]&alive)){take=k;break;}
        if(take<0)return false;
        alive&=~(uint16_t(1)<<take);
    }
    return true;
}

#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
#include <algorithm>

// A pair of products imposes a partial order on seven backings. For a
// sampled starting order, all inverted comparisons must lose an endpoint.
// Precompute minimum vertex cover for every one of the 2^21 edge subsets.
struct SevenCover {
    std::vector<unsigned char> cover;
    uint64_t rowmask[7][128]={};
    SevenCover():cover(1u<<21,0) {
        unsigned incident[7]={};int endpoints[21][2],b=0;
        for(int u=0;u<7;u++)for(int v=u+1;v<7;v++,b++){
            unsigned bit=1u<<b;incident[u]|=bit;incident[v]|=bit;
            endpoints[b][0]=u;endpoints[b][1]=v;
            for(int mask=0;mask<128;mask++){
                if(mask&(1<<v))rowmask[u][mask]|=bit;
                if(mask&(1<<u))rowmask[v][mask]|=uint64_t(bit)<<32;
            }
        }
        for(unsigned m=1;m<cover.size();m++){
            int b=__builtin_ctz(m);
            cover[m]=1+std::min(cover[m&~incident[endpoints[b][0]]],cover[m&~incident[endpoints[b][1]]]);
        }
    }
    static const SevenCover& get(){static SevenCover table;return table;}
    double cost(uint64_t directed,const std::vector<std::array<int,16>>& samples)const {
        unsigned f=unsigned(directed),r=unsigned(directed>>32);int sum=0;
        for(const auto& sample:samples){unsigned invert=sample[7];sum+=cover[(f&invert)|(r&~invert)];}
        return double(sum)/samples.size();
    }
};
inline std::vector<std::array<int,16>> graph_sample_orders(uint64_t a0,int B,int count) {
    uint64_t state=0xd1b54a32d192ed03ULL^a0;
    if(!state)state=0xd1b54a32d192ed03ULL;
    auto rnd=[&](){state^=state<<7;state^=state>>9;return uint32_t(state);};
    std::vector<std::array<int,16>> samples;
    while(int(samples.size())<count) {
        std::array<int,16> p{},rank{},reverse_rank{};
        for(int k=0;k<B;k++)p[k]=k;
        for(int k=B-1;k>0;k--)std::swap(p[k],p[rnd()%(k+1)]);
        for(int k=0;k<B;k++){rank[p[k]]=k;reverse_rank[p[k]]=B-1-k;}
        if(B==7) {
            int bit=0;
            for(int u=0;u<7;u++)for(int v=u+1;v<7;v++,bit++)
                if(rank[u]>rank[v])rank[7]|=1<<bit;
            reverse_rank[7]=((1<<21)-1)^rank[7];
        }
        samples.push_back(rank);
        if(int(samples.size())<count)samples.push_back(reverse_rank);
    }
    return samples;
}

// Exact replacements for the seven-vertex independent-set DP used by kind 3.
namespace exact_small_graph {
inline int independent_set(const uint16_t*bad,unsigned mask) {
    int isolated=0,pivot=-1,max_degree=0;
    unsigned scan=mask;
    while(scan) {
        int v=__builtin_ctz(scan);scan&=scan-1;
        int degree=__builtin_popcount(unsigned(bad[v])&mask);
        if(!degree){mask&=~(1u<<v);isolated++;}
        else if(degree>max_degree){max_degree=degree;pivot=v;}
    }
    if(!mask)return isolated;
    // After deleting isolated vertices, maximum degree 1 means disjoint edges.
    if(max_degree==1)return isolated+__builtin_popcount(mask)/2;
    unsigned without_pivot=mask&~(1u<<pivot);
    int excluded=independent_set(bad,without_pivot);
    unsigned with_pivot=without_pivot&~unsigned(bad[pivot]);
    // Including the pivot cannot beat this bound even if all its non-neighbors
    // were mutually independent. This often eliminates the second branch.
    if(excluded>=1+__builtin_popcount(with_pivot))return isolated+excluded;
    return isolated+max(excluded,1+independent_set(bad,with_pivot));
}

inline void inversion_graph(const uint16_t*closure,int B,const array<int,16>&rank,uint16_t*bad) {
    fill(bad,bad+B,uint16_t(0));
    for(int v=0;v<B;v++) {
        unsigned p=closure[v];
        while(p) {
            int u=__builtin_ctz(p);p&=p-1;
            if(rank[u]>rank[v]){bad[u]|=1u<<v;bad[v]|=1u<<u;}
        }
    }
}

#ifdef COL3_METRIC_FAST_VERIFY
inline int reference_independent_set(const uint16_t*bad,int B) {
    array<unsigned char,128> dp{};int size=1<<B;
    for(int mask=1;mask<size;mask++) {
        int v=__builtin_ctz(unsigned(mask)),rest=mask&(mask-1);
        dp[mask]=max<int>(dp[rest],1+dp[rest&~bad[v]]);
    }
    return dp[size-1];
}
#endif

inline double evaluate(const uint16_t*closure,int B,const vector<array<int,16>>&samples,int solver_mode) {
    assert(B>0 && B<=7);
    bool total_order=solver_mode==1;int closure_edges=0;array<int,7> topo{};unsigned used_depth=0;
    for(int v=0;v<B;v++) {
        int depth=__builtin_popcount(unsigned(closure[v]));closure_edges+=depth;
        if(depth>=B || (closure[v]&(1u<<v)) || (used_depth&(1u<<depth)))total_order=false;
        else {used_depth|=1u<<depth;topo[depth]=v;}
    }
    total_order=total_order && closure_edges==B*(B-1)/2;
    int total=0;
    for(const auto&rank:samples) {
        int independent;
        if(total_order) {
            // The inversion graph of a total order is a permutation graph.
            // Its maximum independent set is the LIS of the sampled ranks in
            // topological order. Bits represent the distinct patience tails.
            unsigned tails=0;
            for(int i=0;i<B;i++) {
                unsigned value=rank[topo[i]],larger=tails&(~0u<<value);
                tails^=larger&-larger;tails|=1u<<value;
            }
            independent=__builtin_popcount(tails);
        }else {
            uint16_t bad[16];inversion_graph(closure,B,rank,bad);
            independent=independent_set(bad,(1u<<B)-1);
        }
#ifdef COL3_METRIC_FAST_VERIFY
        uint16_t bad[16];inversion_graph(closure,B,rank,bad);
        assert(independent==reference_independent_set(bad,B));
#endif
        total+=B-independent;
    }
    // Preserve the integer accumulation and final division of the original.
    return double(total)/samples.size();
}
} // namespace exact_small_graph

inline double graph_dag_metric(const uint16_t* before,int B,int kind,
        const std::vector<std::array<int,16>>& samples) {
    if(kind==0)return 0;
    int direct_edges=0;for(int k=0;k<B;k++)direct_edges+=__builtin_popcount(unsigned(before[k]));
    if(direct_edges==0)return 0;
    if(kind==3 && direct_edges==1)return .5; // Samples come in reversal pairs.
    uint16_t closure[16];std::copy(before,before+B,closure);
    for(int k=0;k<B;k++)for(int v=0;v<B;v++)if(closure[v]&(1u<<k))closure[v]|=closure[k];
    if(kind==1) {
        int count=0;for(int k=0;k<B;k++)count+=__builtin_popcount(unsigned(closure[k]));
        return count;
    }
    const int size=1<<B;
    if(kind==2) {
        std::vector<uint64_t> ways(size,0);ways[0]=1;
        for(int mask=0;mask<size;mask++)if(ways[mask])for(int v=0;v<B;v++)
            if(!(mask&(1<<v)) && !(before[v]&~mask))ways[mask|(1<<v)]+=ways[mask];
        return std::lgamma(B+1)/std::log(2.0)-std::log2(double(ways.back()));
    }
    if(kind==3 && B==7) {
        const auto& lut=SevenCover::get();uint64_t directed=0;
        for(int k=0;k<7;k++)directed|=lut.rowmask[k][closure[k]];
        return lut.cost((directed<<32)|(directed>>32),samples);
    }
    if(kind==3 && B>0 && B<=7)
        return exact_small_graph::evaluate(closure,B,samples,1);
    std::vector<unsigned char> dp(size,0);
    int total=0;
    for(const auto &rank:samples) {
        uint16_t bad[16]={};
        for(int v=0;v<B;v++) {
            uint16_t p=closure[v];
            while(p) {
                int u=__builtin_ctz(unsigned(p));p&=p-1;
                if(rank[u]>rank[v]){bad[u]|=1u<<v;bad[v]|=1u<<u;}
            }
        }
        for(int mask=1;mask<size;mask++) {
            int v=__builtin_ctz(unsigned(mask)),rest=mask&(mask-1);
            dp[mask]=std::max<int>(dp[rest],1+dp[rest&~bad[v]]);
        }
        total+=B-dp.back();
    }
    return double(total)/samples.size();
}

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

// Euler orientation balances the number of printed 0s and 1s at every vertex.
inline void balance_comparison_polarities(std::vector<std::pair<int,int>>& edges,int B) {
    int real=edges.size();auto all=edges;
    std::vector<int> degree(B);
    for(auto [u,v]:all){degree[u]++;degree[v]++;}
    for(int v=0;v<B;v++)if(degree[v]&1)all.push_back({v,B});
    std::vector<std::vector<std::pair<int,int>>> adj(B+1);
    for(int i=0;i<int(all.size());i++) {
        auto [u,v]=all[i];adj[u].push_back({v,i});adj[v].push_back({u,i});
    }
    std::vector<int> pos(B+1);std::vector<bool> used(all.size());
    for(int start=0;start<=B;start++) {
        std::vector<int> stack={start};
        while(!stack.empty()) {
            int u=stack.back();
            while(pos[u]<int(adj[u].size())&&used[adj[u][pos[u]].second])pos[u]++;
            if(pos[u]==int(adj[u].size())){stack.pop_back();continue;}
            auto [v,id]=adj[u][pos[u]++];used[id]=true;
            if(id<real)edges[id]={u,v};
            stack.push_back(v);
        }
    }
}

inline void random_comparison_polarities(std::vector<std::pair<int,int>>& edges,uint64_t seed) {
    uint64_t state=seed^0x94d049bb133111ebULL;
    if(!state)state=0x94d049bb133111ebULL;
    for(auto &e:edges){state^=state<<7;state^=state>>9;if(state&1)std::swap(e.first,e.second);}
}

struct Blossom {
    int n;vector<vector<int>> g;vector<int> match,p,base,q;vector<bool> used,blossom;
    Blossom(int n):n(n),g(n),match(n,-1),p(n),base(n),used(n),blossom(n){}
    int lca(int a,int b) {
        vector<bool> vis(n,false);
        while(true){a=base[a];vis[a]=true;if(match[a]<0)break;a=p[match[a]];}
        while(true){b=base[b];if(vis[b])return b;b=p[match[b]];}
    }
    void mark_path(int v,int b,int child) {
        while(base[v]!=b) {blossom[base[v]]=blossom[base[match[v]]]=true;p[v]=child;child=match[v];v=p[match[v]];}
    }
    bool augment(int root) {
        fill(used.begin(),used.end(),false);fill(p.begin(),p.end(),-1);
        for(int i=0;i<n;i++)base[i]=i;
        q.clear();q.push_back(root);used[root]=true;
        for(size_t head=0;head<q.size();head++) {
            int v=q[head];
            for(int u:g[v]) {
                if(base[v]==base[u] || match[v]==u)continue;
                if(u==root || (match[u]>=0 && p[match[u]]>=0)) {
                    int b=lca(v,u);fill(blossom.begin(),blossom.end(),false);
                    mark_path(v,b,u);mark_path(u,b,v);
                    for(int i=0;i<n;i++)if(blossom[base[i]]) {base[i]=b;if(!used[i])used[i]=true,q.push_back(i);}
                } else if(p[u]<0) {
                    p[u]=v;
                    if(match[u]<0) {
                        while(u>=0) {int pv=p[u],next=pv<0?-1:match[pv];match[u]=pv;if(pv>=0)match[pv]=u;u=next;}
                        return true;
                    }
                    int m=match[u];used[m]=true;q.push_back(m);
                }
            }
        }
        return false;
    }
    int solve(){for(int i=0;i<n;i++)if(match[i]<0)augment(i);int c=0;for(int i=0;i<n;i++)c+=match[i]>=0;return c/2;}
};

struct PairInfo {
    int u, v;
    int u_top, u_bot;
    int v_top, v_bot;
    int bucket_w;
    int paper_id;
};


struct Design {
    int W,T,C,tot_B;
    vector<string> S_init;
    vector<PairInfo> pairs;
    vector<int> prod_to_paper,prod_to_pair_idx,paper_bucket;
    vector<bool> paper_is_paired;
    vector<uint64_t> internal_catalog;
    vector<int> column_order;
};

// Exactly the same state machine handles synthetic and actual orders.
// Construct a fresh router when committing the selected paper design.
#ifndef PRE_MIN_GAIN
#define PRE_MIN_GAIN 1
#endif
#ifndef PRE_SUPPORT_LIMIT
#define PRE_SUPPORT_LIMIT 1
#endif
#ifndef PRE_ONLY_ANCHORLESS
#define PRE_ONLY_ANCHORLESS 1
#endif
// Exactly the same state machine handles synthetic and actual orders.
// Construct a fresh router when committing the selected paper design.
#include <array>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <cassert>
#include <limits>

// Complete comparison graph: every backing except the bottom one is visible.
// Distances are invariant under relabeling. One BFS serves every catalog and query.
struct ExactBackingPermutations {
    int n, states;
    std::array<int,13> fact{};
    std::vector<std::array<uint8_t,9>> permutations;
    std::vector<uint8_t> distance, operation;
    std::vector<int> parent;
    std::vector<std::vector<int>> reverse_edges;
    explicit ExactBackingPermutations(int count):n(count) {
        assert(n>=2 && n<=9);
        fact[0]=1;for(int i=1;i<=12;i++)fact[i]=fact[i-1]*i;
        states=fact[n];permutations.resize(states);
        std::array<uint8_t,9> p{};for(int i=0;i<n;i++)p[i]=i;
        int r=0;do{permutations[r++]=p;}while(std::next_permutation(p.begin(),p.begin()+n));
        assert(r==states);
        distance.assign(states,255);operation.assign(states,255);parent.assign(states,-1);
        std::vector<int> queue;queue.reserve(states);queue.push_back(0);distance[0]=0;
        for(size_t head=0;head<queue.size();head++) {
            int from=queue[head];const auto p=permutations[from];
            for(int i=0;i<n-1;i++)for(int j=0;j<n;j++)if(i!=j) {
                auto q=p;uint8_t k=q[i];
                if(i<j)for(int t=i;t<j;t++)q[t]=q[t+1];
                else for(int t=i;t>j;t--)q[t]=q[t-1];
                q[j]=k;int to=rank(q);
                if(distance[to]!=255)continue;
                distance[to]=distance[from]+1;parent[to]=from;operation[to]=(k<<4)|j;queue.push_back(to);
            }
        }
        assert((int)queue.size()==states);
        reverse_edges.resize(states);
        for(int id=0;id<states;id++) {
            const auto p=permutations[id];
            for(int i=0;i<n;i++)for(int j=0;j<n-1;j++)if(i!=j) {
                auto q=p;int k=q[i];
                if(i<j)for(int t=i;t<j;t++)q[t]=q[t+1];else for(int t=i;t>j;t--)q[t]=q[t-1];
                q[j]=k;reverse_edges[id].push_back(rank(q));
            }
            std::sort(reverse_edges[id].begin(),reverse_edges[id].end());
            reverse_edges[id].erase(std::unique(reverse_edges[id].begin(),reverse_edges[id].end()),reverse_edges[id].end());
        }
    }
    int rank(const std::array<uint8_t,9>& p) const {
        int value=0;unsigned used=0;
        for(int i=0;i<n;i++) {
            int v=p[i];value+=__builtin_popcount(((1u<<v)-1)&~used)*fact[n-1-i];used|=1u<<v;
        }
        return value;
    }
    static const ExactBackingPermutations& get(int n) {
        static std::array<std::unique_ptr<ExactBackingPermutations>,10> tables;
        if(!tables[n])tables[n]=std::make_unique<ExactBackingPermutations>(n);
        return *tables[n];
    }
};

#include <algorithm>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

// Exact memoization for the complete-comparison-core permutation graph.
// n plus the labeled transitive closure determines the target order set.
namespace requirement_distance_cache {
struct Entry {
    std::vector<int> orders;
    std::vector<uint8_t> distance;
};
struct Statistics {
    size_t lookups=0,hits=0,insertions=0,capacity_skips=0;
    size_t payload_bytes=0,accounted_bytes=0,peak_accounted_bytes=0;
    size_t bfs_runs=0,orders_reused=0;
};
inline Statistics& statistics(){static Statistics value;return value;}
inline auto& entries(){static std::map<std::string,std::shared_ptr<const Entry>> value;return value;}
#ifndef REQUIREMENT_CACHE_BYTE_LIMIT
#define REQUIREMENT_CACHE_BYTE_LIMIT (64ULL*1024ULL*1024ULL)
#endif
inline constexpr size_t byte_limit=REQUIREMENT_CACHE_BYTE_LIMIT;

template<class Closure>
inline std::string key(int n,const Closure& closure) {
    std::string answer;answer.reserve(1+2*n);answer.push_back(char(n));
    for(int k=0;k<n;k++){answer.push_back(char(closure[k]&255));answer.push_back(char((closure[k]>>8)&255));}
    return answer;
}
inline std::shared_ptr<const Entry> lookup(const std::string& key) {
    auto& st=statistics();st.lookups++;
    auto it=entries().find(key);if(it==entries().end())return {};
    st.hits++;return it->second;
}
inline std::shared_ptr<const Entry> remember(const std::string& key,
        const std::vector<int>& orders,const std::vector<uint8_t>& distance) {
    auto found=entries().find(key);if(found!=entries().end())return found->second;
    // The budget includes vector payload and a deliberately conservative
    // allowance for the entry, shared control block, map node and string.
    size_t payload=orders.size()*sizeof(int)+distance.size()*sizeof(uint8_t);
    size_t accounted=payload+sizeof(Entry)+sizeof(std::string)+key.size()+1+192;
    auto& st=statistics();
    if(accounted>byte_limit||st.accounted_bytes>byte_limit-accounted){st.capacity_skips++;return {};}
    auto value=std::make_shared<Entry>();value->orders=orders;value->distance=distance;
    payload=value->orders.capacity()*sizeof(int)+value->distance.capacity()*sizeof(uint8_t);
    accounted=payload+sizeof(Entry)+sizeof(std::string)+key.size()+1+192;
    if(accounted>byte_limit||st.accounted_bytes>byte_limit-accounted){st.capacity_skips++;return {};}
    entries().emplace(key,value);st.insertions++;st.payload_bytes+=payload;st.accounted_bytes+=accounted;
    st.peak_accounted_bytes=std::max(st.peak_accounted_bytes,st.accounted_bytes);
    return value;
}
} // namespace requirement_distance_cache

#include <cstddef>
#include <ctime>

// Absolute process CPU deadlines. Zero preserves the original unlimited path.
namespace routing_budget {
inline bool expired(clock_t deadline) {
    return deadline>0 && clock()>=deadline;
}
struct Counters {
    std::size_t future_build_aborts=0;
    std::size_t support_utility_aborts=0;
    std::size_t support_drop_aborts=0;
    std::size_t policy_early_stops=0;
    std::size_t extract_dp_aborts=0;
    std::size_t trace_skipped_calls=0;
    std::size_t trace_dp_aborts=0;
    std::size_t post_option_skips=0;
};
inline Counters& counters() {static Counters value;return value;}
}


struct ExactGraphBacking {
    struct Requirement {
        std::array<unsigned,9> pred{},closure{};
        std::vector<int> orders;
        std::string distance_cache_key;
        std::shared_ptr<const requirement_distance_cache::Entry> cached_distance;
        bool possible=true;
    };
    int n;
    const ExactBackingPermutations& table;
    std::vector<Requirement> req;
    std::vector<double> future_value;
    bool complete=true;
    template<class DesignType>
    ExactGraphBacking(const DesignType& d,const std::vector<uint64_t>& catalog,clock_t optional_deadline=0):n(d.tot_B),table(ExactBackingPermutations::get(n)) {
        assert(n<=9);
        std::array<unsigned,9> adjacent{};
        std::array<int,64> zero{},one{};zero.fill(-1);one.fill(-1);
        for(int j=0;j<64;j++)if(j<d.C || j>=d.T) {
            int count=0;
            for(int k=0;k<n;k++) {
                char c=d.S_init[k][j];if(c=='.')continue;
                count++;if(c=='0')zero[j]=k;else one[j]=k;
            }
            assert(count==2 && zero[j]>=0 && one[j]>=0);
            adjacent[zero[j]]|=1u<<one[j];adjacent[one[j]]|=1u<<zero[j];
        }
        for(int k=0;k<n;k++)if(adjacent[k]!=(((1u<<n)-1)^(1u<<k)))complete=false;
        assert(complete);
        req.resize(catalog.size());
        for(int a=0;a<(int)catalog.size();a++) {
            auto& r=req[a];int main=d.prod_to_paper[a];
            for(int j=0;j<64;j++)if((j<d.C || j>=d.T) && d.S_init[main][j]=='.') {
                int top=(catalog[a]>>j&1)?one[j]:zero[j];
                int bottom=(catalog[a]>>j&1)?zero[j]:one[j];
                r.pred[bottom]|=1u<<top;
            }
            r.closure=r.pred;
            for(int k=0;k<n;k++)for(int v=0;v<n;v++)if(r.closure[v]>>k&1)r.closure[v]|=r.closure[k];
            for(int k=0;k<n;k++)if(r.closure[k]>>k&1)r.possible=false;
            assert(r.possible);
            // Enumerate only valid linear extensions in lexicographic order.
            unsigned constrained=0;for(int k=0;k<n;k++)constrained|=r.pred[k];
            if(!constrained){r.orders.push_back(0);continue;}
            r.distance_cache_key=requirement_distance_cache::key(n,r.closure);
            r.cached_distance=requirement_distance_cache::lookup(r.distance_cache_key);
            if(r.cached_distance){r.orders=r.cached_distance->orders;requirement_distance_cache::statistics().orders_reused++;continue;}
            auto enumerate=[&](auto&& self,unsigned used,int depth,int rank)->void {
                if(depth==n){r.orders.push_back(rank);return;}
                for(int k=0;k<n;k++)if(!(used>>k&1) && !(r.pred[k]&~used)) {
                    int add=__builtin_popcount(((1u<<k)-1)&~used)*table.fact[n-1-depth];
                    self(self,used|(1u<<k),depth+1,rank+add);
                }
            };
            enumerate(enumerate,0,0,0);
            assert(!r.orders.empty());
        }
        std::string key;key.push_back(char(n));
        for(const auto& r:req)for(int k=0;k<n;k++){key.push_back(char(r.pred[k]&255));key.push_back(char(r.pred[k]>>8));}
        static std::map<std::string,std::vector<double>> cache;
        auto saved=cache.find(key);
        if(saved!=cache.end()){future_value=saved->second;return;}
        future_value.assign(table.states,0.0);
        auto abandon_future=[&]() {
            future_value.assign(table.states,0.0);
            routing_budget::counters().future_build_aborts++;
        };
        if(routing_budget::expired(optional_deadline)){abandon_future();return;}
        std::vector<int> queue;queue.reserve(table.states);
        std::vector<uint8_t> distance(table.states,255);
        // Each bit is one requirement. From target permutations, propagate all
        // requirements at this depth through the same reversed legal moves.
        vector<vector<uint8_t>> packed(req.size());
        if(n==5 || n==6 || n==7){
            vector<int> misses;
            for(int a=0;a<(int)req.size();a++)
                if(!req[a].distance_cache_key.empty() && !req[a].cached_distance)misses.push_back(a);
            vector<uint64_t> seen(table.states),front(table.states),spread(table.states);
            for(int l=0;l<(int)misses.size();l+=64){
                int batch=min(64,int(misses.size())-l);
                const uint64_t full=batch==64?~uint64_t(0):(uint64_t(1)<<batch)-1;
                array<uint8_t*,64> out{};
                fill(seen.begin(),seen.end(),0);
                for(int j=0;j<batch;j++){
                    auto a=misses[l+j];packed[a].assign(table.states,0);out[j]=packed[a].data();
                    for(int id:req[a].orders)seen[id]|=1ULL<<j;
                }
                front=seen;
                for(uint8_t depth=1;;depth++){
                    if(routing_budget::expired(optional_deadline)){abandon_future();return;}
                    fill(spread.begin(),spread.end(),0);
                    for(int id=0;id<table.states;id++)if(front[id])
                        for(int previous:table.reverse_edges[id])spread[previous]|=front[id];
                    bool complete=true;
                    for(int id=0;id<table.states;id++){
                        uint64_t add=spread[id]&~seen[id];front[id]=add;seen[id]|=add;
                        complete&=seen[id]==full;
                        for(;add;add&=add-1)out[__builtin_ctzll(add)][id]=depth;
                    }
                    if(complete)break;
                }
            }
        }
        for(size_t a=0;a<req.size();a++) {
            auto& r=req[a];
            if(routing_budget::expired(optional_deadline)){abandon_future();return;}
            unsigned constrained=0;for(int k=0;k<n;k++)constrained|=r.pred[k];
            if(!constrained)continue;
            if(!r.cached_distance)r.cached_distance=requirement_distance_cache::lookup(r.distance_cache_key);
            if(r.cached_distance){
                const auto& cached=r.cached_distance->distance;assert(int(cached.size())==table.states);
                for(int id=0;id<table.states;id++)future_value[id]+=double(cached[id])/req.size();
                continue;
            }
            requirement_distance_cache::statistics().bfs_runs++;
            if(n==5 || n==6 || n==7){distance.swap(packed[a]);}
            else{
                std::fill(distance.begin(),distance.end(),255);queue.clear();
                for(int id:r.orders){distance[id]=0;queue.push_back(id);}
                for(size_t head=0;head<queue.size();head++) {
                    if((head&255)==0 && routing_budget::expired(optional_deadline)){abandon_future();return;}
                    int id=queue[head];
                    for(int before:table.reverse_edges[id])if(distance[before]==255) {
                        distance[before]=distance[id]+1;queue.push_back(before);
                    }
                }
                assert((int)queue.size()==table.states);
            }
            r.cached_distance=requirement_distance_cache::remember(r.distance_cache_key,r.orders,distance);
            for(int id=0;id<table.states;id++)future_value[id]+=double(distance[id])/req.size();
        }
        cache.emplace(key,future_value);
    }
    bool satisfied(const Requirement& r,const std::array<uint8_t,9>& order) const {
        unsigned used=0;for(int i=0;i<n;i++){int k=order[i];if(r.pred[k]&~used)return false;used|=1u<<k;}return true;
    }
    mutable int last_route_product=-1;
    mutable std::array<uint8_t,9> last_route_order{};
    mutable std::vector<std::pair<int,int>> last_route_moves;
    mutable size_t route_cache_hits=0,route_cache_misses=0;

    template<class DeckType>
    void route(DeckType& deck,int a,std::vector<std::pair<int,int>>& moves) const {
        std::array<uint8_t,9> order{};
        for(int i=0;i<n;i++){assert(deck.order[i]<n);order[i]=deck.order[i];}
        if(a==last_route_product&&order==last_route_order){
            route_cache_hits++;
#ifdef ROUTE_CACHE_VERIFY
            auto reference=deck;std::vector<std::pair<int,int>> expected;
            route_uncached(reference,a,expected);
            assert(expected==last_route_moves);
#endif
            for(auto op:last_route_moves)deck.move_paper(op.first,op.second,moves);
#ifdef ROUTE_CACHE_VERIFY
            assert(deck.order==reference.order);
            assert(deck.position==reference.position);
#endif
            return;
        }
        route_cache_misses++;
        size_t begin=moves.size();
        route_uncached(deck,a,moves);
        last_route_product=a;last_route_order=order;
        last_route_moves.assign(moves.begin()+begin,moves.end());
    }

    template<class DeckType>
    void route_uncached(DeckType& deck,int a,std::vector<std::pair<int,int>>& moves) const {
        const auto& r=req[a];
        std::array<uint8_t,9> order{},position{};
        for(int i=0;i<n;i++){assert(deck.order[i]<n);order[i]=deck.order[i];position[order[i]]=i;}
        if(satisfied(r,order))return;
        // Choose the best future state among all shortest legal one-move plans.
        int move_k=-1,move_d=-1;double one_value=1e100;
        for(int i=0;i<n-1;i++)for(int j=0;j<n;j++)if(i!=j) {
            auto q=order;int k=q[i];
            if(i<j)for(int t=i;t<j;t++)q[t]=q[t+1];else for(int t=i;t>j;t--)q[t]=q[t-1];
            q[j]=k;if(satisfied(r,q)) {
                double value=future_value[table.rank(q)];
                if(value<one_value){one_value=value;move_k=k;move_d=j;}
            }
        }
        if(move_k>=0){deck.move_paper(move_k,move_d,moves);return;}
        int best=255,best_id=-1;double best_value=1e100;
        for(int id:r.orders) {
            const auto& target=table.permutations[id];std::array<uint8_t,9> relative{};
            for(int i=0;i<n;i++)relative[i]=position[target[i]];
            int relative_id=table.rank(relative),cost=table.distance[relative_id];
            double value=future_value[id];
            if(cost<best || (cost==best && value<best_value)) {best=cost;best_id=relative_id;best_value=value;}
        }
        assert(best_id>=0);
        std::vector<uint8_t> ops;
        for(int id=best_id;id;id=table.parent[id])ops.push_back(table.operation[id]);
        for(auto it=ops.rbegin();it!=ops.rend();++it){int op=*it;deck.move_paper(order[op>>4],op&15,moves);}
#ifndef NDEBUG
        for(int i=0;i<n;i++)order[i]=deck.order[i];
        assert(satisfied(r,order));
#endif
    }
};

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <tuple>
#include <utility>
#include <vector>

namespace comparison_backing {
using Mask=uint16_t;
using Move=std::pair<int,int>;

inline bool closure(std::vector<Mask>& reach) {
    const int n=reach.size();
    for(int k=0;k<n;k++)for(int i=0;i<n;i++)if(reach[i]&(Mask(1)<<k))reach[i]|=reach[k];
    for(int i=0;i<n;i++)if(reach[i]&(Mask(1)<<i))return false;
    return true;
}

struct Search {
    int n;
    std::vector<Mask> adjacent,reach,pred;
    std::array<Mask,12> conflict{};
    std::array<int8_t,4096> memo{};
    std::array<uint32_t,4096> stamp{};
    uint32_t epoch=0;

    Search(const std::vector<Mask>& graph,const std::vector<Mask>& transitive):
        n(graph.size()),adjacent(graph),reach(transitive),pred(n,0) {
        for(int i=0;i<n;i++)for(int j=0;j<n;j++)if(reach[i]&(Mask(1)<<j))pred[j]|=Mask(1)<<i;
    }

    static std::array<int,12> positions(const std::vector<int>& order) {
        std::array<int,12> p{};
        for(int i=0;i<(int)order.size();i++)p[order[i]]=i;
        return p;
    }
    bool visible(int v,const std::array<int,12>& pos) const {
        Mask m=adjacent[v];
        while(m){int u=__builtin_ctz(m);m&=m-1;if(pos[u]>pos[v])return true;}
        return false;
    }
    bool already_before_successors(int v,Mask alive,const std::array<int,12>& pos) const {
        Mask m=reach[v]&alive;
        while(m){int u=__builtin_ctz(m);m&=m-1;if(pos[u]<pos[v])return false;}
        return true;
    }
    bool satisfied(const std::vector<int>& order) const {
        auto pos=positions(order);
        for(int v=0;v<n;v++)if(!already_before_successors(v,(Mask(1)<<n)-1,pos))return false;
        return true;
    }
    int cover(Mask alive) {
        if(stamp[alive]==epoch)return memo[alive];
        int v=-1,degree=0;
        Mask iter=alive;
        while(iter) {
            int u=__builtin_ctz(iter);iter&=iter-1;
            int d=__builtin_popcount(unsigned(conflict[u]&alive));
            if(d>degree){degree=d;v=u;}
        }
        int value=0;
        if(v>=0) {
            Mask remaining=alive&~(Mask(1)<<v);
            value=std::min(1+cover(remaining),degree+cover(remaining&~conflict[v]));
        }
        stamp[alive]=epoch;memo[alive]=value;return value;
    }
    // Exact lower bound with visibility ignored, on the unfrozen vertices.
    int lower_bound(const std::vector<int>& order,Mask alive) {
        conflict.fill(0);auto pos=positions(order);
        for(int v=0;v<n;v++)if(alive&(Mask(1)<<v)) {
            Mask m=reach[v]&alive;
            while(m){int u=__builtin_ctz(m);m&=m-1;if(pos[u]<pos[v]){conflict[u]|=Mask(1)<<v;conflict[v]|=Mask(1)<<u;}}
        }
        ++epoch;
        if(epoch==0){stamp.fill(0);++epoch;}
        return cover(alive);
    }
    int inversions(const std::vector<int>& order,Mask alive) const {
        auto pos=positions(order);int ans=0;
        for(int v=0;v<n;v++)if(alive&(Mask(1)<<v)) {
            Mask m=reach[v]&alive;
            while(m){int u=__builtin_ctz(m);m&=m-1;ans+=pos[v]>pos[u];}
        }
        return ans;
    }
    void move(std::vector<int>& order,int v,int depth,std::vector<Move>& out) const {
        auto pos=positions(order);int from=pos[v];
        if(from==depth)return;
        assert(visible(v,pos));
        order.erase(order.begin()+from);order.insert(order.begin()+depth,v);out.emplace_back(v,depth);
    }

    std::vector<Move> greedy(std::vector<int> order,int style=0) {
        Mask alive=(Mask(1)<<n)-1;std::vector<Move> answer;
        while(alive) {
            if(satisfied(order))break;
            // Such a source is already before all remaining successors. Freezing
            // it preserves every required relation to future moved vertices.
            bool removed=true;
            while(removed) {
                removed=false;auto pos=positions(order);
                for(int v=0;v<n;v++)if((alive&(Mask(1)<<v)) && !(pred[v]&alive) && already_before_successors(v,alive,pos)) {
                    alive&=~(Mask(1)<<v);removed=true;
                }
            }
            if(!alive || satisfied(order))break;
            std::vector<int> best_order;std::vector<Move> best_moves;int best_source=-1;
            std::tuple<int,int,int,int> best_key{100000,100000,100000,100000};
            auto consider=[&](int v,std::vector<int> next,std::vector<Move> seq) {
                auto pos=positions(next);
                if(!already_before_successors(v,alive,pos)) {
                    int destination=n;Mask m=reach[v]&alive;
                    while(m){int u=__builtin_ctz(m);m&=m-1;destination=std::min(destination,pos[u]);}
                    assert(destination<pos[v]);
                    // Every fixed predecessor is before this destination: its
                    // transitive successors include all the successors of v.
                    move(next,v,destination,seq);
                }
                Mask rest=alive&~(Mask(1)<<v);
                int estimate=seq.size()+lower_bound(next,rest);
                int inv=inversions(next,rest),distance=0;
                auto before=positions(order),after=positions(next);
                for(auto [u,d]:seq)distance+=std::abs(after[u]-before[u]);
                auto key=style==0?std::make_tuple(estimate,inv,int(seq.size()),distance):
                                    std::make_tuple(estimate,int(seq.size()),inv,distance);
                if(key<best_key){best_key=key;best_source=v;best_order=std::move(next);best_moves=std::move(seq);}
            };
            auto pos=positions(order);
            for(int v=0;v<n;v++)if((alive&(Mask(1)<<v)) && !(pred[v]&alive)) {
                if(visible(v,pos))consider(v,order,{});
                else {
                    // A source needing to move has a direct required successor,
                    // hence an unfrozen graph neighbor. That neighbor is visible
                    // at their shared window and can expose v with one move.
                    Mask neighbors=adjacent[v]&alive;
                    assert(neighbors);
                    while(neighbors) {
                        int u=__builtin_ctz(neighbors);neighbors&=neighbors-1;
                        assert(pos[u]<pos[v]);
                        for(int d=pos[v];d<n;d++) {
                            auto next=order;std::vector<Move> seq;
                            move(next,u,d,seq);consider(v,std::move(next),std::move(seq));
                        }
                    }
                }
            }
            assert(best_source>=0);
            answer.insert(answer.end(),best_moves.begin(),best_moves.end());
            order=std::move(best_order);alive&=~(Mask(1)<<best_source);
        }
        assert(satisfied(order));assert(answer.size()<=2*unsigned(n));
        return answer;
    }

    static uint64_t permutation_key(const std::vector<int>& order) {
        uint64_t key=0;for(int v:order)key=(key<<4)|uint64_t(v);return key;
    }
    // Direct insertions can repair several inversions at once. Prefer a move
    // decreasing the exact visibility-free distance, with one-step activation
    // moves allowed when the optimal paper is currently hidden.
    std::vector<Move> descent(std::vector<int> order,std::vector<Move> best) {
        Mask all=(Mask(1)<<n)-1;
        std::vector<Move> prefix;
        std::vector<uint64_t> visited{permutation_key(order)};
        int lb=lower_bound(order,all);
        while(lb && int(prefix.size())+lb<int(best.size())) {
            auto pos=positions(order);
            std::tuple<int,int,int,int> best_key{10000,10000,10000,10000};
            std::vector<int> chosen;int chosen_v=-1,chosen_d=-1,chosen_lb=10000;
            for(int v:order)if(visible(v,pos))for(int d=0;d<n;d++)if(d!=pos[v]) {
                auto next=order;next.erase(next.begin()+pos[v]);next.insert(next.begin()+d,v);
                auto key=permutation_key(next);
                if(std::find(visited.begin(),visited.end(),key)!=visited.end())continue;
                int lower=lower_bound(next,all);
                if(lower>lb || int(prefix.size())+1+lower>=int(best.size()))continue;
                int inv=inversions(next,all),visible_count=0;auto p=positions(next);
                for(int u=0;u<n;u++)visible_count+=visible(u,p);
                auto score=std::make_tuple(lower,inv,-visible_count,std::abs(d-pos[v]));
                if(score<best_key) {
                    best_key=score;chosen=std::move(next);chosen_v=v;chosen_d=d;chosen_lb=lower;
                }
            }
            if(chosen_v<0)break;
            prefix.emplace_back(chosen_v,chosen_d);order=std::move(chosen);lb=chosen_lb;
            visited.push_back(permutation_key(order));
            if(lb==0){best=prefix;break;}
            auto rest=greedy(order,0);
            if(prefix.size()+rest.size()<best.size()) {
                best=prefix;best.insert(best.end(),rest.begin(),rest.end());
            }
        }
        return best;
    }
};

inline bool plan(const std::vector<int>& current,const std::vector<Mask>& graph,
                 const std::vector<Mask>& required,std::vector<Move>& answer) {
    const int n=current.size();
    if(n<1 || n>12 || (int)graph.size()!=n || (int)required.size()!=n)return false;
    auto reach=required;if(!closure(reach))return false;
    Search search(graph,reach);
    answer=search.greedy(current,0);
    int optimum_lower=search.lower_bound(current,(Mask(1)<<n)-1);
    if((int)answer.size()>optimum_lower) {
        auto alt=search.greedy(current,1);
        if(alt.size()<answer.size())answer=std::move(alt);
    }
    if((int)answer.size()>optimum_lower)answer=search.descent(current,std::move(answer));
    return true;
}
} // namespace comparison_backing

// Assumptions: all B backing papers occupy the first B positions; each backing
// window has exactly two opaque papers, one black and one white. Both suffix and
// control holes of the new main paper become required ordering edges.
template<class Simulator>
bool route_graph(Simulator& sim,const std::vector<uint64_t>& opaque,
                 const std::vector<uint64_t>& black,int p_new,uint64_t target,
                 int B,int T,int C,std::vector<std::pair<int,int>>& moves) {
    using comparison_backing::Mask;
    if(B<1 || B>12)return false;
    std::vector<Mask> graph(B,0),required(B,0);
    for(int j=0;j<64;j++)if(j<C || j>=T) {
        int a=-1,b=-1,count=0;
        for(int k=0;k<B;k++)if((opaque[k]>>j)&1){if(count==0)a=k;else if(count==1)b=k;count++;}
        if(count==0)continue;
        if(count!=2)return false;
        graph[a]|=Mask(1)<<b;graph[b]|=Mask(1)<<a;
        if((opaque[p_new]>>j)&1)continue;
        bool match_a=!((black[a]^target)&(1ULL<<j));
        bool match_b=!((black[b]^target)&(1ULL<<j));
        if(!match_a && !match_b)return false;
        if(match_a && match_b)continue;
        if(match_b)std::swap(a,b);
        required[a]|=Mask(1)<<b;
    }
    std::vector<int> current(sim.order.begin(),sim.order.begin()+B);
    for(int k:current)if(k<0 || k>=B)return false;
    std::vector<std::pair<int,int>> plan;
    if(!comparison_backing::plan(current,graph,required,plan))return false;
    for(auto [k,d]:plan)sim.move_paper(k,d,moves);
    return true;
}


template<class D> bool has_complete_backing_graph(const D& d) {
    const int b=d.tot_B;
    if(b<2 || b>8)return false;
    unsigned adjacent[8]={};
    for(int j=0;j<N;j++)if(j<d.C || j>=d.T) {
        int first=-1,second=-1,count=0;
        for(int k=0;k<b;k++)if(d.S_init[k][j]!='.') {
            if(count==0)first=k;else if(count==1)second=k;
            count++;
        }
        if(count!=2)return false;
        adjacent[first]|=1u<<second;adjacent[second]|=1u<<first;
    }
    for(int k=0;k<b;k++)if(adjacent[k]!=(((1u<<b)-1)^(1u<<k)))return false;
    return true;
}

// Approximate local MDP for one staircase anchor. Four interchangeable types:
// low bit = anchor color, high bit = (number of products on the paper - 1).
namespace bucket_mdp {
constexpr int max_size=10;
inline int frequency(int type){return 1+(type>>1);}
inline uint32_t insert(uint32_t code,int type,int position) {
    const uint32_t mask=(uint32_t(1)<<(2*position))-1;
    return (code&mask)|(uint32_t(type)<<(2*position))|((code&~mask)<<2);
}
inline bool expired(clock_t deadline){return deadline>0 && clock()>=deadline;}
struct Model {
    int n=0,products=256,stages=0;
    std::array<int,4> counts{};
    std::array<double,2> repair{};
    double no_event=0,span=0,residual=0;
    std::vector<uint32_t> codes,missing_codes;
    std::unordered_map<uint32_t,int> rank,missing_rank;
    std::vector<std::array<uint8_t,max_size>> types;
    std::vector<std::array<int,max_size>> head_move,insertion;
    std::vector<std::array<int,2>> first;
    std::vector<int> removed,outside;
    std::vector<double> correction,missing_correction;
    std::vector<double> direct_correction,direct_missing_correction;

    bool build(const std::array<int,4>& composition,const std::array<int,2>& requests,
               int catalog_size,int horizon,double repair_weight,int state_cap,clock_t deadline) {
        counts=composition;products=catalog_size;
        std::vector<int> sorted;
        int main_mass=0;
        for(int type=0;type<4;type++)for(int k=0;k<counts[type];k++) {
            sorted.push_back(type);main_mass+=frequency(type);
        }
        n=int(sorted.size());if(n<2 || n>max_size || expired(deadline))return false;
        for(int c=0;c<2;c++)repair[c]=repair_weight*requests[c];
        // Local main events never require repair at their own anchor.
        no_event=std::max(0.0,products-main_mass-repair[0]-repair[1]);
        do {
            uint32_t code=0;for(int i=0;i<n;i++)code|=uint32_t(sorted[i])<<(2*i);
            codes.push_back(code);
            if(int(codes.size())>state_cap)return false;
            if((codes.size()&255)==0 && expired(deadline))return false;
        }while(std::next_permutation(sorted.begin(),sorted.end()));
        const int size=int(codes.size());
        rank.reserve(size*2);missing_rank.reserve(size*2);
        for(int id=0;id<size;id++)rank.emplace(codes[id],id);
        types.resize(size);head_move.resize(size);first.resize(size);removed.resize(size);
        for(int id=0;id<size;id++) {
            if((id&127)==0 && expired(deadline))return false;
            const uint32_t code=codes[id];first[id]={n,n};
            for(int j=0;j<n;j++) {
                const int type=(code>>(2*j))&3;types[id][j]=uint8_t(type);
                first[id][type&1]=std::min(first[id][type&1],j);
                head_move[id][j]=rank.at(insert(code>>2,code&3,j));
            }
            auto found=missing_rank.emplace(code>>2,int(missing_codes.size()));
            if(found.second){missing_codes.push_back(code>>2);outside.push_back(code&3);}
            removed[id]=found.first->second;
        }
        const int missing_size=int(missing_codes.size());
        insertion.resize(missing_size);
        for(int id=0;id<missing_size;id++) {
            if((id&127)==0 && expired(deadline))return false;
            for(int j=0;j<n;j++)insertion[id][j]=rank.at(insert(missing_codes[id],outside[id],j));
        }
        std::vector<double> value(size),absent(missing_size),reference(size),missing_reference(missing_size);
        auto static_cost=[&](uint32_t code,int length) {
            double result=0;int first_color[2]={length,length};
            for(int j=0;j<length;j++) {
                const int type=(code>>(2*j))&3;
                result+=frequency(type)*(j+1);first_color[type&1]=std::min(first_color[type&1],j);
            }
            for(int c=0;c<2;c++)result+=repair[c]*first_color[c];
            return result/products;
        };
        for(int id=0;id<size;id++)value[id]=reference[id]=static_cost(codes[id],n);
        for(int id=0;id<missing_size;id++)absent[id]=missing_reference[id]=static_cost(missing_codes[id],n-1);
        // e[j][q]: cost of exposing a distinguished target at depth j, then
        // lending it as main. r[j][q] exposes it without removing it.
        // The moved head may only be reinserted below that target. This is
        // exactly the legal local blocker operation, with unit move cost.
        std::vector<double> e(size*n),r(size*n),next_value(size),next_absent(missing_size);
        for(int iteration=0;iteration<horizon;iteration++) {
            if(expired(deadline))break;
            for(int id=0;id<size;id++){e[id]=1+absent[removed[id]];r[id]=value[id];}
            bool abort=false;
            for(int j=1;j<n && !abort;j++)for(int id=0;id<size;id++) {
                if((id&255)==0 && expired(deadline)){abort=true;break;}
                double best_e=1e100,best_r=1e100;
                for(int position=j;position<n;position++) {
                    const int after=(j-1)*size+head_move[id][position];
                    best_e=std::min(best_e,e[after]);best_r=std::min(best_r,r[after]);
                }
                e[j*size+id]=1+best_e;r[j*size+id]=1+best_r;
            }
            if(abort)break;
            for(int id=0;id<size;id++) {
                double total=no_event*value[id];
                for(int j=0;j<n;j++)total+=frequency(types[id][j])*e[j*size+id];
                for(int c=0;c<2;c++) {
                    // A one-color bucket cannot satisfy the opposite color
                    // locally; leave that event unchanged rather than invent
                    // a legal repair or index beyond the transform table.
                    total+=repair[c]*(first[id][c]<n?r[first[id][c]*size+id]:value[id]);
                }
                next_value[id]=total/products;
            }
            for(int id=0;id<missing_size;id++) {
                if((id&127)==0 && expired(deadline)){abort=true;break;}
                double best_idle=1e100,best_same=1e100,best_color[2]={1e100,1e100};
                std::array<double,max_size> best_main;best_main.fill(1e100);
                for(int position=0;position<n;position++) {
                    const int full=insertion[id][position];
                    best_idle=std::min(best_idle,value[full]);
                    best_same=std::min(best_same,e[position*size+full]);
                    for(int c=0;c<2;c++)best_color[c]=std::min(best_color[c],
                        first[full][c]<n?r[first[full][c]*size+full]:value[full]);
                    for(int j=0;j<n-1;j++) {
                        const int target=j+int(position<=j);
                        best_main[j]=std::min(best_main[j],e[target*size+full]);
                    }
                }
                double total=no_event*best_idle+(frequency(outside[id])-1)*best_same;
                for(int c=0;c<2;c++)total+=repair[c]*best_color[c];
                for(int j=0;j<n-1;j++)total+=frequency((missing_codes[id]>>(2*j))&3)*best_main[j];
                // The old main returns on the next GLOBAL request, and its
                // insertion can depend on that newly revealed request. The
                // previous product is excluded; the other paired product is
                // still possible, with a return and a new main extraction.
                next_absent[id]=1+total/(products-1);
            }
            if(abort)break;
            double minimum=*std::min_element(next_value.begin(),next_value.end());
            double low=1e100,high=-1e100;
            for(int id=0;id<size;id++) {
                low=std::min(low,next_value[id]-value[id]);high=std::max(high,next_value[id]-value[id]);
                next_value[id]-=minimum;
            }
            for(double& v:next_absent)v-=minimum;
            value.swap(next_value);absent.swap(next_absent);stages++;
            residual=high-low;
        }
        if(stages==0)return false;
        // Only differences between insertion choices matter. Center each
        // missing-type class separately so absent-main constants do not leak
        // into unrelated route comparisons. Keep the existing one-step
        // score and add this longer-horizon correction to it.
        correction.resize(size);missing_correction.resize(missing_size);
        double center=0;
        for(int id=0;id<size;id++){correction[id]=products*(value[id]-reference[id]);center+=correction[id]/size;}
        for(double& v:correction)v-=center;
        std::array<double,4> centers{};std::array<int,4> class_size{};
        for(int id=0;id<missing_size;id++) {
            missing_correction[id]=products*(absent[id]-missing_reference[id]);
            centers[outside[id]]+=missing_correction[id];class_size[outside[id]]++;
        }
        for(int id=0;id<missing_size;id++)missing_correction[id]-=centers[outside[id]]/class_size[outside[id]];
        span=*std::max_element(correction.begin(),correction.end())-*std::min_element(correction.begin(),correction.end());
        // Populate by copying the finalized doubles, preserving every bit.
        // Impossible multiset codes retain NaN and take the old fallback.
        const double invalid=std::numeric_limits<double>::quiet_NaN();
        direct_correction.assign(size_t(1)<<(2*n),invalid);
        direct_missing_correction.assign(size_t(1)<<(2*(n-1)),invalid);
        for(int id=0;id<size;id++)direct_correction[codes[id]]=correction[id];
        for(int id=0;id<missing_size;id++)direct_missing_correction[missing_codes[id]]=missing_correction[id];
        return true;
    }
    bool lookup(uint32_t code,int length,double& result) const {
        if(length==n) {
            if(code>=direct_correction.size())return false;
            result=direct_correction[code];return !std::isnan(result);
        }
        if(length==n-1) {
            if(code>=direct_missing_correction.size())return false;
            result=direct_missing_correction[code];return !std::isnan(result);
        }
        return false;
    }
};
} // namespace bucket_mdp

// Fixed storage for an exact insertion-position cache. Empty entries are
// identified by a generation stamp; no query allocates or frees cache nodes.
#include <array>
#include <cstdint>
#include <cstring>
namespace nov9_insert_fast {
struct Table {
    static constexpr int capacity=256,key_capacity=32;
    struct Entry {
        uint64_t hash=0;
        uint32_t generation=0;
        uint8_t length=0,position=0;
        std::array<uint8_t,key_capacity> key{};
        double cost=0;
    };
    std::array<Entry,capacity> entries{};
    uint32_t generation=0;
    int used=0;
    void clear() {
        used=0;
        if(++generation==0) {
            for(auto&entry:entries)entry.generation=0;
            generation=1;
        }
    }
    static uint64_t hash_key(const uint8_t*key,int length) {
        uint64_t value=14695981039346656037ULL;
        for(int i=0;i<length;i++){value^=key[i];value*=1099511628211ULL;}
        return value;
    }
    int locate(const uint8_t*key,int length,uint64_t hash,bool&hit) const {
        hit=false;
        for(int step=0;step<capacity;step++) {
            const int index=int((hash+uint64_t(step))&(capacity-1));
            const auto& entry=entries[index];
            if(entry.generation!=generation)return index;
            if(entry.hash==hash && entry.length==length &&
               std::memcmp(entry.key.data(),key,size_t(length))==0){hit=true;return index;}
        }
        return -1;
    }
    void store(int index,const uint8_t*key,int length,uint64_t hash,int position,double cost) {
        auto&entry=entries[index];
        if(entry.generation!=generation)used++;
        entry.generation=generation;entry.hash=hash;entry.length=uint8_t(length);
        entry.position=uint8_t(position);entry.cost=cost;
        std::memcpy(entry.key.data(),key,size_t(length));
    }
};
} // namespace nov9_insert_fast

struct OnlineRouter {
    const Design &design;
    const vector<uint64_t> &A;
    const int W,T,C,tot_B;
    const vector<PairInfo> &pairs;
    const vector<int> &prod_to_paper,&prod_to_pair_idx,&paper_bucket;
    const vector<bool> &paper_is_paired;
    DeckTracker sim;
    int active_main=-1;
    vector<int> active_supports,support_cut;
    vector<vector<int>> bucket_members;
    vector<array<int,2>> hole_request_count;
    vector<uint64_t> opaque,black;
    const uint64_t index_main_mask;
    unique_ptr<ExactGraphBacking> graph;
    vector<array<double,32>> support_utility;
    double support_order_weight=1.0;
std::vector<std::unique_ptr<bucket_mdp::Model>> bucket_mdp_tables;
std::vector<uint8_t> bucket_mdp_projected,bucket_mdp_paper_type;
int bucket_mdp_projection_cap=2;
mutable size_t bucket_mdp_projection_hits=0;
double bucket_mdp_weight=0;
mutable size_t bucket_mdp_hits=0,bucket_mdp_misses=0,bucket_mdp_support_fallbacks=0;

void prepare_bucket_mdp(clock_t absolute_deadline) {
    const clock_t started=clock();
    const double cap=std::max(0.0,getenv("BMDP_CPU")?atof(getenv("BMDP_CPU")):.10);
    bucket_mdp_weight=getenv("BMDP_WEIGHT")?atof(getenv("BMDP_WEIGHT")):.5;
    const int horizon=std::max(0,std::min(12,getenv("BMDP_HORIZON")?atoi(getenv("BMDP_HORIZON")):3));
    const int limit=std::max(2,std::min(bucket_mdp::max_size,getenv("BMDP_SIZE")?atoi(getenv("BMDP_SIZE")):9));
    const int state_cap=std::max(1,getenv("BMDP_STATES")?atoi(getenv("BMDP_STATES")):6000);
    const double repair_weight=std::max(0.0,std::min(1.0,getenv("BMDP_REPAIR")?atof(getenv("BMDP_REPAIR")):.5));
    const clock_t local_deadline=started+clock_t(search_budget::seconds(cap)*CLOCKS_PER_SEC);
    const clock_t deadline=absolute_deadline>0?std::min(absolute_deadline,local_deadline):local_deadline;
    bucket_mdp_tables.resize(T);bucket_mdp_projected.assign(T,0);
    bucket_mdp_paper_type.assign(K,0);
    for(int w=C;w<T;w++)for(int paper:bucket_members[w])
        bucket_mdp_paper_type[paper]=uint8_t((sim.S[paper][w]-'0')+2*int(paper_is_paired[paper]));
    bucket_mdp_projection_cap=std::max(1,std::min(2,getenv("BPROJ_CAP")?atoi(getenv("BPROJ_CAP")):2));
    const bool projection_enabled=!getenv("BPROJ_ENABLE") || atoi(getenv("BPROJ_ENABLE"));
    std::vector<int> buckets;
    for(int w=C;w<T;w++)if(bucket_members[w].size()>=2 &&
        (int(bucket_members[w].size())<=limit || projection_enabled))buckets.push_back(w);
    auto composition=[&](int w) {
        std::array<int,4> count{};
        for(int paper:bucket_members[w])count[bucket_mdp_paper_type[paper]]++;
        if(int(bucket_members[w].size())>limit)for(int& c:count)c=std::min(c,bucket_mdp_projection_cap);
        return count;
    };
    // Preserve the original small-bucket models. Large buckets use at most
    // two front occurrences of each type; only the MDP delta is projected.
    auto state_estimate=[&](int w) {
        const auto count=composition(w);double states=1;int used=0;
        for(int type=0;type<4;type++)for(int k=1;k<=count[type];k++)states*=double(++used)/k;
        return states;
    };
    std::sort(buckets.begin(),buckets.end(),[&](int x,int y) {
        const double sx=state_estimate(x),sy=state_estimate(y);
        if(sx!=sy)return sx<sy;
        return hole_request_count[x][0]+hole_request_count[x][1]>hole_request_count[y][0]+hole_request_count[y][1];
    });
    int built=0,projected=0,states=0,stages=0;double max_span=0;
    if(bucket_mdp_weight!=0 && horizon>0)for(int w:buckets) {
        if(bucket_mdp::expired(deadline))break;
        const auto count=composition(w);
        const bool is_projection=int(bucket_members[w].size())>limit;
        auto model=std::make_unique<bucket_mdp::Model>();
        if(!model->build(count,hole_request_count[w],P,horizon,repair_weight,state_cap,deadline))continue;
        built++;projected+=is_projection;states+=int(model->codes.size());stages+=model->stages;max_span=std::max(max_span,model->span);
        bucket_mdp_projected[w]=is_projection;
        std::cerr<<"BMDP_BUCKET w="<<w<<" n="<<model->n<<" actual_n="<<bucket_members[w].size()
            <<" projected="<<is_projection<<" states="<<model->codes.size()
            <<" stages="<<model->stages<<" correction_span="<<model->span<<" residual="<<model->residual<<"\n";
        bucket_mdp_tables[w]=std::move(model);
    }
    std::cerr<<"BMDP_BUILD buckets="<<built<<" projected="<<projected<<" projection_cap="<<bucket_mdp_projection_cap<<" states="<<states<<" stages="<<stages
        <<" correction_span="<<max_span<<" weight="<<bucket_mdp_weight<<" repair="<<repair_weight
        <<" cpu="<<double(clock()-started)/CLOCKS_PER_SEC<<" timeout="<<bucket_mdp::expired(deadline)<<"\n";
}

template<class Item>
double bucket_mdp_adjust(int count,const Item& item,double baseline) const {
    if(bucket_mdp_weight==0 || count<1 || bucket_mdp_tables.empty())return baseline;
    const int w=paper_bucket[item(0)];
    if(w<C || w>=T || !bucket_mdp_tables[w])return baseline;
    for(int paper:active_supports)if(paper_bucket[paper]==w) {
        bucket_mdp_support_fallbacks++;return baseline;
    }
    uint32_t code=0;int modeled_count=0;std::array<int,4> seen{};
    const bool projected=bucket_mdp_projected[w];
    if(!projected && count>bucket_mdp::max_size)return baseline;
    for(int j=0;j<count;j++) {
        const int paper=item(j);
        const int type=bucket_mdp_paper_type[paper];
        if(projected && seen[type]++>=bucket_mdp_projection_cap)continue;
        code|=uint32_t(type)<<(2*modeled_count++);
    }
    double delta=0;
    // Removing a real main can leave the projected multiset unchanged when
    // another copy of its type enters the front-two projection. In that case
    // use the full local model, as a deliberate approximation.
    if(!bucket_mdp_tables[w]->lookup(code,modeled_count,delta)){bucket_mdp_misses++;return baseline;}
    bucket_mdp_hits++;bucket_mdp_projection_hits+=projected;
    return baseline+bucket_mdp_weight*delta;
}


    OnlineRouter(const Design &d,const vector<uint64_t> &catalog,clock_t optional_deadline=0):
        design(d),A(catalog),W(d.W),T(d.T),C(d.C),tot_B(d.tot_B),pairs(d.pairs),
        prod_to_paper(d.prod_to_paper),prod_to_pair_idx(d.prod_to_pair_idx),paper_bucket(d.paper_bucket),
        paper_is_paired(d.paper_is_paired),support_cut(K,-1),opaque(K,0),black(K,0),
        index_main_mask(((1ULL<<T)-1)^((1ULL<<C)-1)),graph(has_complete_backing_graph(d)?make_unique<ExactGraphBacking>(d,catalog,optional_deadline):nullptr) {
        sim.init(d.S_init);
        hole_request_count.resize(T,{0,0});
        for(int a=0;a<P;a++)for(int w=C;w<T;w++)
            if(sim.S[prod_to_paper[a]][w]=='.')hole_request_count[w][(A[a]>>w)&1]++;

        bucket_members.resize(T);
        for(int k=0;k<K;k++)if(paper_bucket[k]>=0)bucket_members[paper_bucket[k]].push_back(k);
        for(int k=0;k<K;k++)for(int j=0;j<N;j++) {
            if(sim.S[k][j]!='.')opaque[k]|=1ULL<<j;
            if(sim.S[k][j]=='1')black[k]|=1ULL<<j;
        }
        support_utility.resize(K);
        if(support_order_weight!=0 && !routing_budget::expired(optional_deadline)) {
            double expectation[65][32]={};
            for(int g=0;g<=64;g++) {
                double prob=ldexp(1.0,-g);
                for(int n=0;n<=g;n++) {
                    for(int r=0;r<32;r++)expectation[g][r]+=prob*max(0,n-r-2);
                    if(n<g)prob*=double(g-n)/(n+1);
                }
            }
            for(int k=tot_B;k<K;k++) {
                if(routing_budget::expired(optional_deadline)) {
                    support_utility.assign(K,std::array<double,32>{});
                    routing_budget::counters().support_utility_aborts++;
                    break;
                }
                int w=paper_bucket[k];if(w<C)continue;
                for(int a=0;a<P;a++) {
                    uint64_t holes=index_main_mask&~opaque[prod_to_paper[a]];
                    if(!(holes&(1ULL<<w)))continue;
                    uint64_t bad=(black[k]^A[a])&opaque[k]&holes;
                    int cut=bad?63-__builtin_clzll(bad):C-1;
                    if(cut>=w)continue;
                    uint64_t cover=opaque[k]&holes&(~0ULL<<(cut+1))&~(1ULL<<w);
                    int g=__builtin_popcountll(cover);
                    for(int r=0;r<32;r++)support_utility[k][r]+=expectation[g][r];
                }
            }
        }
        if(const char *e=getenv("POLICY_MASK"))route_policies=atoi(e)|1;
        if(const char *e=getenv("ROUTE_QUEUE_WEIGHT"))route_queue_weight=atof(e);
        if(const char *e=getenv("ROUTE_DROP_WEIGHT"))route_drop_weight=atof(e);
        if(const char *e=getenv("ROUTE_DROP_MODEL"))route_drop_model=atoi(e);
        support_drop.assign(K,vector<double>(T+1));
        if(route_drop_weight!=0 && route_drop_model==0)for(int k=tot_B;k<K;k++) {
            if(routing_budget::expired(optional_deadline)) {
                for(auto& row:support_drop)std::fill(row.begin(),row.end(),0.0);
                routing_budget::counters().support_drop_aborts++;
                break;
            }
            int w=paper_bucket[k];if(w<C)continue;
            for(int cut=C-1;cut<w;cut++) {
                int dropped=0;
                for(int a=0;a<P;a++) {
                    int main=prod_to_paper[a],next_w=paper_bucket[main];
                    if(main==k)continue;
                    uint64_t holes=index_main_mask&~opaque[main];
                    uint64_t cover=holes&opaque[k]&(~0ULL<<(cut+1));
                    bool accessible=next_w<=cut || sim.S[k][next_w]=='.';
                    dropped+=!accessible || bool((black[k]^A[a])&cover);
                }
                support_drop[k][cut+1]=double(dropped)/P;
            }
        }
    }

    double joint_extract_support_weight=getenv("EXTRACT_UTILITY")?atof(getenv("EXTRACT_UTILITY")):.5;
    int joint_extract_mode=getenv("EXTRACT_DP")?atoi(getenv("EXTRACT_DP")):1;
    long long joint_extract_calls=0,joint_extract_improved=0,joint_extract_cache_hits=0;double joint_extract_gain=0;
    map<string,vector<int>> joint_extract_cache;
    // Product/target, paper attributes, and MDP/utility weights stay fixed
    // during query(a). Only its policy clones share these cached positions.
    nov9_insert_fast::Table nov9_insert_positions;
    bool nov9_insert_enabled=!getenv("NOV9_INSERT_CACHE") || atoi(getenv("NOV9_INSERT_CACHE"));
    long long nov9_insert_requests=0,nov9_insert_hits=0,nov9_insert_misses=0;
    long long nov9_insert_scanned=0,nov9_insert_peak=0,nov9_insert_long_keys=0,nov9_insert_full=0;
    int route_mode=0;
    bool front_support_enabled=getenv("FRONT_SUPPORT")?atoi(getenv("FRONT_SUPPORT")):true;
    long long front_support_kept=0,front_support_core_returns=0;
    int post_support_choices=getenv("POST_SUPPORT_CHOICES")?atoi(getenv("POST_SUPPORT_CHOICES")):4;
    int post_support_rank=0,post_first_option_count=0;
    int post_second_choices=clamp(search_budget::integer("POST_SECOND_CHOICES",4),1,8);
    int post_second_rank=0,post_second_option_count=0;
    long long post_second_trials=0,post_second_wins=0,post_second_saved=0;
    long long post_choice_trials=0,post_choice_wins=0,post_choice_saved=0;

    static constexpr int batch_limit=3, overlay_count=3;
    struct BatchPlacement {int cut; uint64_t seen,bad;int price;bool keep;};
    struct BatchPlan {array<int,3> moves{};int score=100000;int donor=-1,cut=-1;};
    vector<int> batch_sources;
    array<vector<BatchPlacement>,3> batch_placements;
    array<BatchPlan,batch_limit> batch_plans{};
    array<BatchPlan,overlay_count> overlays{};
    int batch_choice=0;
    bool batch_failed=false;
    int route_policies=251|(((1<<(batch_limit+overlay_count))-1)<<10);
    bool route_branching=false;
    clock_t route_deadline=0;
    int route_guard_hits=0;
    unsigned policy_effects=0;
    int runtime_return_penalty=1,runtime_pre_limit=1,runtime_anchorless=1,runtime_support_limit=3;
    // Break close route choices using expected retrieval/repair cost of the
    // remaining bucket queues, in addition to immediate moves and support returns.
    double route_queue_weight=0.1,route_drop_weight=.5;
    int route_drop_model=1;
    vector<vector<double>> support_drop;
    struct RouteState {
        vector<int> order,position,supports,cuts;
        int main;
    };
    RouteState save_route_state() const {return {sim.order,sim.position,active_supports,support_cut,active_main};}
    void restore_route_state(const RouteState& s) {
        sim.order=s.order;sim.position=s.position;active_supports=s.supports;support_cut=s.cuts;active_main=s.main;
    }

    struct PortfolioEntry {
        RouteState state;
        vector<pair<int,int>> moves;
        double score;
    };
    bool portfolio_enabled=false;
    vector<PortfolioEntry> portfolio;
    void collect_portfolio(const vector<pair<int,int>>& moves) {
        if(!portfolio_enabled)return;
        double score=moves.size()+route_state_value();
        int cap=clamp(search_budget::integer("PORTFOLIO_KEEP",8),1,32);
        if(int(portfolio.size())>=cap && score>=portfolio.back().score-1.e-9)return;
        for(const auto& p:portfolio)if(p.moves==moves)return;
        auto it=upper_bound(portfolio.begin(),portfolio.end(),score,
            [](double x,const PortfolioEntry& p){return x<p.score;});
        portfolio.insert(it,PortfolioEntry{save_route_state(),moves,score});
        if(int(portfolio.size())>cap)portfolio.pop_back();
    }
    struct SharedTail{
        vector<int> start,supports,cuts;
        int main,limit,penalty,rank,second_rank,late,late_at,pending,pending_at,cost;
    };
    vector<SharedTail> tails;int used_tails=0;
    double route_state_value() const {
        double cost=0;
        if(route_queue_weight!=0){
            array<int,64> start{},length{};
            for(int w=C+1;w<T;w++)start[w]=start[w-1]+int(bucket_members[w-1].size());
            array<int,K> papers;
            for(int k:sim.order)if(k!=active_main && k>=tot_B){
                int w=paper_bucket[k];
                if(w>=C && w<T)papers[start[w]+length[w]++]=k;
            }
            for(int w=C;w<T;w++) {
                const int* b=papers.data()+start[w];
                double q=0;
                for(int i=0;i<length[w];i++)q+=(1+int(paper_is_paired[b[i]]))*(i+1);
                if(length[w]>=2) {
                    int first[2]={length[w],length[w]};
                    for(int i=0;i<length[w];i++)first[sim.S[b[i]][w]-'0']=min(first[sim.S[b[i]][w]-'0'],i);
                    q+=.5*(hole_request_count[w][0]*first[0]+hole_request_count[w][1]*first[1]);
                    if(active_main>=0 && paper_bucket[active_main]==w){
                        int color=(black[active_main]>>w)&1;
                        q-=.25*hole_request_count[w][color]*first[color];
                    }
                    if(first[0]+first[1]==1)q+=.5*(int(paper_is_paired[b[0]])-int(paper_is_paired[b[1]]));
                }
                for(int i=0;i<length[w]&&i<32;i++)q-=support_order_weight*support_utility[b[i]][i];
                cost+=route_queue_weight*q/P;
            }
        }
        if(route_drop_weight!=0)for(int k:active_supports)cost+=route_drop_weight*(route_drop_model==1 ? 1.0 : support_drop[k][support_cut[k]+1]);
        return cost;
    }
    int return_alternative=-1;
    bool return_front=false;
    int return_trials=0,return_wins=0;
    void set_route_policy(int id) {
        route_mode=0;
        runtime_return_penalty=1;runtime_pre_limit=1;runtime_anchorless=1;runtime_support_limit=3;
        if(id==1)route_mode=1;
        if(id==2)route_mode=2;
        if(id==3)route_mode=4;
        if(id==4)runtime_return_penalty=0;
        if(id==5)runtime_return_penalty=2;
        if(id==6)runtime_pre_limit=0;
        if(id==7)runtime_anchorless=0;
        if(id==8)runtime_support_limit=1;
        if(id==9)runtime_support_limit=2;
        if(id>=10){route_mode=8;batch_choice=id-10;}
    }
    // Compare this alternative with the ordinary route from the same deck.
    // A donor can be inserted before the main is promoted, or after promotion
    // if the new main leaves the donor's own handle open.
    struct PaintIndex{
        uint64_t ink[2][64][3]{};
        bool ready=false;
        void prepare(const vector<uint64_t>& opaque,const vector<uint64_t>& black){
            if(ready)return;
            for(int k=0;k<K;k++)for(uint64_t b=opaque[k];b;b&=b-1){
                const int j=__builtin_ctzll(b);
                ink[int(black[k]>>j&1)][j][k/64]|=1ULL<<(k%64);
            }
            ready=true;
        }
    } paint_index;
    unsigned keep_sources=0,park_sources=0;int extra_source=-1;
    bool route_with_sources(int a,vector<pair<int,int>>& moves){
        const int m=prod_to_paper[a],n=tot_B+int(active_supports.size());
        if(n>8)return false;
        array<int,8> id{};array<uint64_t,8> intervening{};
        uint64_t extra=0,ordinary_ink=0,front_ink=0;unsigned remote=0;int count=0;
        for(int k:sim.order){
            bool source=find(active_supports.begin(),active_supports.end(),k)!=active_supports.end();
            if(k>=tot_B && !source){ordinary_ink|=opaque[k];continue;}
            if(ordinary_ink){
                if(!source || !(opaque[k]&~(ordinary_ink|front_ink)))return false;
                remote|=1u<<count;intervening[count]=ordinary_ink;
            }
            if(source)extra|=opaque[k];
            id[count++]=k;front_ink|=opaque[k];
            if(count==n)break;
        }
        uint64_t holes=~opaque[m] & (~index_main_mask|extra);
        const unsigned full=(1u<<n)-1;
        array<uint64_t,256> cover;cover[0]=0;
        array<uint64_t,8> wrong;
        for(int j=0;j<n;j++)wrong[j]=(black[id[j]]^A[a])&opaque[id[j]]&holes;
        for(unsigned st=1;st<=full;st++)cover[st]=cover[st&(st-1)]|opaque[id[__builtin_ctz(st)]];
        array<uint8_t,256> legal{};
        for(unsigned st=full;st;st--){
            if(!legal[st] && st!=full)continue;
            for(unsigned left=st;left;left&=left-1){
                int j=__builtin_ctz(left);unsigned bit=left&-left,prev=st^bit;
                if(!(wrong[j]&~cover[prev]))legal[prev]|=bit;
            }
        }
        if(!legal[0])return false;
        struct Entry{int k,d;};array<Entry,8> route{},winner{};
        int best=-1,goals=0;double value=1.e30;
        array<uint8_t,8> goal{},where{};
        auto enact=[&](unsigned fixed)->bool{
            unsigned todo=full^fixed;int depth=__builtin_popcount(todo);
            auto walk=[&](auto&& self,array<uint8_t,8> q,unsigned pending,int t)->bool{
                if(!pending)return true;
                uint64_t seen=0;
                for(int i=0;i<n;i++){
                    int x=q[i],p=id[x];bool vis=(opaque[p]&~(seen|((pending>>x&1)?intervening[x]:0)))!=0;seen|=opaque[p];
                    if(!vis || !(pending>>x&1))continue;
                    int j=where[x]+1;while(j<n && (pending>>goal[j]&1))++j;
                    int d=n;if(j<n){d=0;while(q[d]!=goal[j])d++;}
                    int front=n-__builtin_popcount(remote&pending);
                    d=min(d,front);d-=i<d;
                    if(d==i && !(remote>>x&1))continue;
                    auto v=q;
                    if(i<d)move(v.begin()+i+1,v.begin()+d+1,v.begin()+i);
                    else move_backward(v.begin()+d,v.begin()+i,v.begin()+i+1);
                    v[d]=x;route[t]={p,d};
                    if(self(self,v,pending^(1u<<x),t+1))return true;
                }
                return false;
            };
            array<uint8_t,8> q{};for(int i=0;i<n;i++)q[i]=i;
            if(!walk(walk,q,todo,0))return false;
            array<uint8_t,9> core{};int k=0;for(int j=0;j<n;j++)if(id[goal[j]]<tot_B)core[k++]=id[goal[j]];
            double v=graph?graph->future_value[graph->table.rank(core)]:0;
            if(best<0 || v<value){best=depth;value=v;winner=route;}
            return true;
        };
        const int cap=search_budget::integer("KEEP_DIST",3);
        for(int d=0;d<=min(cap+__builtin_popcount(remote),n-1) && best<0;d++){
            for(unsigned fixed=0;fixed<=full;fixed++)if(!(fixed&remote) && __builtin_popcount(full^fixed)==d){
                auto grow=[&](auto&& self,unsigned set,int h)->void{
                    if(goals>=120 && best>=0)return;
                    if(set==full){++goals;enact(fixed);return;}
                    unsigned waiting=fixed&~set,z=legal[set]&((full^fixed)|(waiting&-waiting));
                    for(;z;z&=z-1){unsigned bit=z&-z;
                        int i=__builtin_ctz(z);goal[h]=i;where[i]=h;self(self,set|bit,h+1);
                    }
                };
                grow(grow,0,0);
            }
        }
        if(best<0)return false;
        for(int i=0;i<best;i++){
            int paper=winner[i].k;
            uint64_t visible=opaque[paper];
            for(int d=0;d<sim.position[paper] && visible;d++)visible&=~opaque[sim.order[d]];
            if(!visible)return false;
            sim.move_paper(paper,winner[i].d,moves);
        }
        sort(active_supports.begin(),active_supports.end(),[&](int x,int y){return sim.depth_of(x)<sim.depth_of(y);});
        for(int k:active_supports)support_cut[k]=C-1;
        runtime_pre_limit=0;return true;
    }

    bool stay_main=false;
    bool switch_pair_in_place(int a,vector<pair<int,int>>& moves){
        if(!graph || tot_B>7 || sim.order[0]!=active_main)return false;
        const auto& tab=graph->table;
        array<uint8_t,9> start{};
        for(int i=0;i<tot_B;i++){int k=sim.order[i+1];if(k>=tot_B)return false;start[i]=k;}
        const auto& required=graph->req[a].pred;
        if(none_of(required.begin(),required.begin()+tot_B,[](unsigned m){return m!=0;}))return true;
        int root=tab.rank(start),goal=-1;
        vector<int> parent(tab.states,-1),queue{root};parent[root]=root;
        vector<uint8_t> edge(tab.states),ok(tab.states);
        for(int id:graph->req[a].orders)ok[id]=1;
        for(size_t i=0;i<queue.size() && goal<0;i++){
            int id=queue[i];if(ok[id]){goal=id;break;}
            auto p=tab.permutations[id];uint64_t cover=opaque[active_main];
            for(int v=0;v<tot_B;v++){
                int paper=p[v];bool visible=(opaque[paper]&~cover)!=0;cover|=opaque[paper];if(!visible)continue;
                for(int d=0;d<tot_B;d++)if(d!=v){auto q=p;
                    if(v<d)move(q.begin()+v+1,q.begin()+d+1,q.begin()+v);
                    else move_backward(q.begin()+d,q.begin()+v,q.begin()+v+1);
                    q[d]=paper;int dest=tab.rank(q);if(parent[dest]>=0)continue;
                    edge[dest]=uint8_t(8*paper+d);parent[dest]=id;
                    if(ok[dest]){goal=dest;break;}queue.push_back(dest);
                }
                if(goal>=0)break;
            }
        }
        if(goal<0)return false;
        vector<uint8_t> path;for(int id=goal;id!=root;id=parent[id])path.push_back(edge[id]);
        for(auto i=path.rbegin();i!=path.rend();i++)sim.move_paper(*i/8,1+(*i%8),moves);
        return true;
    }
    bool add_backing=false;
    int choice_rank=0,choice_count=0;
    // Store exact legal placements for a paper when used under this product.
    struct Goal {uint16_t id;uint8_t slot;};
    vector<vector<Goal>> target_donor;
    struct CachedRoute{int path,slot;double price;};
    array<CachedRoute,K> donor_route{};
    vector<vector<uint16_t>> relative_path;


    int late_donor=-1,late_depth=-1;
    vector<uint64_t> backing_color;
    vector<array<uint64_t,8>> backing_cover;

    bool borrow_backing(int a,vector<pair<int,int>>& actions) {
        const int B=tot_B;
        if((B!=5 && B!=6 && B!=7) || prod_to_pair_idx[a]<0 || active_supports.size()>=3)return false;
        array<int,7> initial{};
        for(int i=0;i<B;i++){initial[i]=sim.order[i];if(initial[i]>=B)return false;}
        int m=prod_to_paper[a],wnew=paper_bucket[m];
        uint64_t target=A[a],ph=index_main_mask&~opaque[m],sh=~(opaque[m]|index_main_mask);
        uint64_t curr=sim.current_display(),wrong=(curr^target)&ph,seen=0;
        struct Donor{uint64_t ink,bad;int k,gain;};
        Donor donors[K];int dn=0;array<int,64> rank{};array<bool,64> opened{};
        for(int k:sim.order){
            uint64_t visible=opaque[k]&~seen;seen|=opaque[k];
            if(k<B || k==m)continue;
            int home=paper_bucket[k],depth=rank[home]++;
            if(!depth)opened[home]=visible>>home&1;
            if((!visible && !opened[home]) || depth>2 || ((opaque[k]>>wnew&1) && (opaque[m]>>paper_bucket[k]&1)) ||
                  ((black[k]^target)&opaque[k]&ph))continue;
            if(find(active_supports.begin(),active_supports.end(),k)!=active_supports.end())continue;
            int w=paper_bucket[k];bool busy=false;
            for(int t:active_supports)busy|=paper_bucket[t]==w;
            if(busy||w==wnew)continue;
            donors[dn++]={opaque[k]&sh,(black[k]^target)&opaque[k]&sh,k,2*__builtin_popcountll(wrong&opaque[k])-3*depth};
        }
        if(!dn)return false;
        if(!graph)return false;
        const auto& tab=graph->table;
        if(backing_color.empty()){
            backing_color.resize(tab.states);
            backing_cover.resize(tab.states);
            for(int id=0;id<tab.states;id++){
                auto& cover=backing_cover[id];cover[0]=0;
                for(int d=0;d<B;d++){
                    int k=tab.permutations[id][d];
                    backing_color[id]|=black[k]&~cover[d];
                    cover[d+1]=cover[d]|opaque[k];
                }
            }
        }
        array<uint8_t,9> pos{},relative{};
        for(int i=0;i<B;i++)pos[initial[i]]=i;
        struct Result {int helper,depth,path;double price;};
        array<Result,K> plans{};int nplan=0;
        if(target_donor.empty())target_donor.resize(P*K);
        for(int j=0;j<dn;j++){
            auto donor=donors[j];int k=donor.k;
            auto& feasible=target_donor[a*K+k];
            if(feasible.empty()){
                for(int id=0;id<tab.states;id++){
                    uint64_t wrong=(backing_color[id]^target)&sh;
                    if(wrong&~donor.ink)continue;
                    int d=B;while(wrong&backing_cover[id][d])--d;
                    if(!(donor.bad&~backing_cover[id][d]))feasible.push_back({(uint16_t)id,(uint8_t)d});
                }
                if(feasible.empty())feasible.push_back({65535,0});
            }
            if(feasible.front().id==65535)continue;
            auto& cache=donor_route[k];
            if(cache.path<0){
                array<uint8_t,9> order{};for(int i=0;i<B;i++)order[i]=initial[i];
                int row=tab.rank(order);
                if(relative_path.empty())relative_path.resize(tab.states);
                auto& paths=relative_path[row];
                if(paths.empty()){
                    paths.resize(tab.states);
                    for(int id=0;id<tab.states;id++){
                        auto& q=tab.permutations[id];for(int i=0;i<B;i++)relative[i]=pos[q[i]];
                        paths[id]=tab.rank(relative);
                    }
                }
                cache.price=1e30;
                for(auto g:feasible){
                    int path=paths[g.id];double price=tab.distance[path]+.15*graph->future_value[g.id];
                    if(price<cache.price)cache={path,g.slot,price};
                }
            }
            plans[nplan++]={k,cache.slot,cache.path,cache.price-.335*donor.gain};
        }
        choice_count=nplan;
        if(choice_rank>=nplan)return false;
        stable_sort(plans.begin(),plans.begin()+nplan,[](auto x,auto y){return x.price<y.price;});
        auto picked=plans[choice_rank];
        int best_path=picked.path,helper=picked.helper,depth=picked.depth;
        if(best_path<0)return false;
        array<uint8_t,9> ops{};int n=0;
        for(int id=best_path;id;id=tab.parent[id])ops[n++]=tab.operation[id];
        while(n){int op=ops[--n];sim.move_paper(initial[op>>4],op&15,actions);}
        auto visible_now=[&](int k){
            uint64_t unseen=opaque[k];for(int q:sim.order){if(q==k)return bool(unseen);unseen&=~opaque[q];}return false;
        };
        if(!visible_now(helper)){
            int home=paper_bucket[helper];
            for(;;){
                int first=-1;for(int q:sim.order)if(paper_bucket[q]==home){first=q;break;}
                if(first==helper)break;
                if(!visible_now(first))return false;
                sim.move_paper(first,sim.depth_of(helper),actions);
            }
        }
        if(!visible_now(helper))return false;
        if(opaque[helper]>>wnew&1){
            late_donor=helper;late_depth=depth;runtime_pre_limit=0;
            return true;
        }
        sim.move_paper(helper,depth,actions);
        active_supports.push_back(helper);support_cut[helper]=C-1;
        return true;
    }
    vector<pair<int,int>> query(int a) {
        portfolio.clear();
        used_tails=0;
        nov9_insert_positions.clear();
        batch_sources.clear();
        post_support_rank=0;post_second_rank=0;
        bool guard=route_branching && route_deadline>0 && clock()>=route_deadline;
        if(guard)route_guard_hits++;
        if(!route_branching || route_policies==1 || guard){set_route_policy(0);return query_impl(a);}
        const auto before=save_route_state();
        RouteState best_state;
        vector<pair<int,int>> best_moves;
        double best=1e100;int best_policy=0,best_options=0;
        int best_first_rank=0,best_second_options=0;
        unsigned distinct=~0u;
        for(int i=0;i<10+batch_limit+overlay_count;i++)if(route_policies&(1<<i)) {
            if(i>=10 && (before.main<0 || before.main==prod_to_paper[a] || before.supports.size()>2 ||
                find(before.supports.begin(),before.supports.end(),prod_to_paper[a])!=before.supports.end()))continue;
            if(i>0 && i<10 && !(distinct&(1u<<i)))continue;
            if(i>0 && routing_budget::expired(route_deadline)) {
                routing_budget::counters().policy_early_stops++;
                break;
            }
            restore_route_state(before);
            set_route_policy(i);
            auto moves=query_impl(a);
            if(i==0)distinct=policy_effects;
            if(batch_failed)continue;
            assert(moves.size()<=2*K && sim.current_display()==A[a]);
            double score=moves.size()+route_state_value();collect_portfolio(moves);
            if(score<best-1e-9){best=score;best_policy=i;best_options=post_first_option_count;
                best_first_rank=0;best_second_options=post_second_option_count;
                best_state=save_route_state();best_moves=move(moves);}
        }
        for(int rank=1;rank<std::min(post_support_choices,best_options);rank++) {
            if(route_deadline>0 && clock()>=route_deadline)break;
            restore_route_state(before);set_route_policy(best_policy);post_support_rank=rank;
            auto moves=query_impl(a);post_choice_trials++;
            if(batch_failed)continue;
            assert(moves.size()<=2*K && sim.current_display()==A[a]);
            double score=moves.size()+route_state_value();collect_portfolio(moves);
            if(score<best-1e-9){post_choice_wins++;post_choice_saved+=int(best_moves.size())-int(moves.size());
                best_first_rank=rank;best_second_options=post_second_option_count;
                best=score;best_state=save_route_state();best_moves=move(moves);}
        }
        // Keep the winning first donor and policy fixed while varying the
        // next post-promotion donor. All replays start from the original deck.
        const int second_first_rank=best_first_rank,second_policy=best_policy;
        const int second_options=min(post_second_choices,best_second_options);
        for(int rank=1;rank<second_options;rank++) {
            if(routing_budget::expired(route_deadline))break;
            restore_route_state(before);set_route_policy(second_policy);
            post_support_rank=second_first_rank;post_second_rank=rank;
            auto moves=query_impl(a);post_second_trials++;
            if(batch_failed || post_second_option_count<=rank)continue;
            assert(moves.size()<=2*K && sim.current_display()==A[a]);
            double score=moves.size()+route_state_value();collect_portfolio(moves);
            if(score<best-1e-9){post_second_wins++;post_second_saved+=int(best_moves.size())-int(moves.size());
                best=score;best_state=save_route_state();best_moves=move(moves);}
        }
        // Later return/borrow alternatives use their original donor policy.
        post_second_rank=0;
        // Returning a visible main or support can expose a different bank
        // head at the same price. Replay the preferred ordinary policy from
        // the original deck, pinning only that sheet's return position.
        const int next_main=prod_to_paper[a];
        auto try_other_head=[&](int paper) {
            if(paper<0 || paper==next_main || routing_budget::expired(route_deadline))return;
            const int w=paper_bucket[paper];
            if(w==paper_bucket[next_main] ||
               find(best_state.supports.begin(),best_state.supports.end(),paper)!=best_state.supports.end())return;
            bool front=true;
            for(int k:bucket_members[w])if(k!=best_state.main && best_state.position[k]<best_state.position[paper]){
                front=false;break;
            }
            restore_route_state(before);set_route_policy(best_policy);post_support_rank=0;
            return_alternative=paper;return_front=!front;used_tails=0;
            auto alternative=query_impl(a);
            return_alternative=-1;used_tails=0;++return_trials;
            if(batch_failed)return;
            assert(alternative.size()<=2*K && sim.current_display()==A[a]);
            double score=alternative.size()+route_state_value();collect_portfolio(alternative);
            if(score<best-1e-9){
                ++return_wins;best=score;best_state=save_route_state();best_moves=move(alternative);
            }
        };
        try_other_head(before.main);
        for(int k:before.supports)try_other_head(k);
        const int ordinary=best_policy;
        for(auto &entry:donor_route)entry.path=-1;
        if(prod_to_pair_idx[a]>=0 && !routing_budget::expired(route_deadline)){
            auto try_borrow=[&](int policy,int rank){
                restore_route_state(before);add_backing=true;set_route_policy(policy);post_support_rank=0;choice_rank=rank;choice_count=0;
                auto moves=query_impl(a);add_backing=false;
                if(!batch_failed){
                    assert(sim.current_display()==A[a]);
                    double score=moves.size()+route_state_value();collect_portfolio(moves);
                    if(score<best){best=score;best_moves=move(moves);best_state=save_route_state();}
                }
                return choice_count;
            };
            int options=try_borrow(0,0);
            if(options>=2 && !routing_budget::expired(route_deadline))try_borrow(0,1);
            if(ordinary!=0 && !routing_budget::expired(route_deadline))try_borrow(ordinary,0);
            if((distinct&2) && ordinary!=1 && !routing_budget::expired(route_deadline))try_borrow(1,0);
        }
        if(before.main>=0 && prod_to_pair_idx[a]>=0 && before.main!=prod_to_paper[a] && before.supports.size()<=3){
            int newmain=prod_to_paper[a],wh=paper_bucket[newmain];
            vector<int> list={before.main};list.insert(list.end(),before.supports.begin(),before.supports.end());
            sort(list.begin()+1,list.end(),[&](int x,int y){return before.position[x]<before.position[y];});
            uint64_t core_seen=0,core_color=0,homes=0,homecolor=0;
            vector<int> first;
            for(int k:before.order){
                if(k<tot_B){core_color|=black[k]&~core_seen;core_seen|=opaque[k];continue;}
                if(k==newmain || find(list.begin(),list.end(),k)!=list.end())continue;
                uint64_t bit=1ULL<<paper_bucket[k];
                if(homes&bit)continue;
                homes|=bit;homecolor|=black[k]&bit;first.push_back(k);
            }
            for(unsigned mask=1;mask<(1u<<list.size());mask++){
                if(routing_budget::expired(route_deadline))break;
                if(__builtin_popcount(mask)>2)continue;
                uint64_t exposed=0,bad=0,ph=index_main_mask&~opaque[newmain];bool ok=true;
                for(int j=0;j<int(list.size());j++)if(mask>>j&1){int k=list[j];
                    if((opaque[k]>>wh&1)||paper_bucket[k]==wh){ok=false;break;}
                    uint64_t paint=opaque[k]&ph;exposed|=paint&~(black[k]^A[a]);bad|=paint&(black[k]^A[a]);
                }
                if(!ok)continue;
                vector<pair<int,int>> extra_candidates;
                if(tot_B+__builtin_popcount(mask)<8){
                    uint64_t wrong=ph&(homecolor^A[a]),suffix=~(index_main_mask|opaque[newmain]);
                    for(int k:first){
                        int home=paper_bucket[k];if(home==wh || (opaque[k]>>wh&1))continue;
                        bool busy=false;for(int j=0;j<int(list.size());j++)if((mask>>j&1)&&paper_bucket[list[j]]==home)busy=true;
                        if(busy)continue;
                        uint64_t color=opaque[k]&~(black[k]^A[a]);
                        uint64_t b=ph&opaque[k]&~color;
                        if(((bad|b)&~(exposed|color)) || (__builtin_popcount(mask)==1 && (bad && b)))continue;
                        int gain=4*__builtin_popcountll(color&(wrong|bad)&ph)-2*__builtin_popcountll(b)
                            +__builtin_popcountll(color&suffix&(core_color^A[a]));
                        if(gain>=3)extra_candidates.push_back({-gain,k});
                    }
                    sort(extra_candidates.begin(),extra_candidates.end());
                    int lim=search_budget::integer("KEEP_EXTRA",1);
                    if(int(extra_candidates.size())>lim)extra_candidates.resize(lim);
                }
                if(!(bad&~exposed))extra_candidates.insert(extra_candidates.begin(),{0,-1});
                unsigned possible_park=0;
                int lastback=0;for(int k=0;k<tot_B;k++)lastback=max(lastback,before.position[k]);
                for(int j=1;j<int(list.size());j++)if(!(mask>>j&1)){
                    int k=list[j],cut=before.cuts[k];
                    if(k!=newmain && paper_bucket[k]!=wh && before.position[k]>lastback &&
                       (wh<=cut || !(opaque[k]>>wh&1)))possible_park|=1u<<j;
                }
                for(auto [hint,ex]:extra_candidates)for(unsigned park=ex>=0?0:possible_park;;park=(park-1)&possible_park){
                    restore_route_state(before);set_route_policy(0);post_support_rank=0;keep_sources=mask;park_sources=park;extra_source=ex;
                    auto alt=query_impl(a);keep_sources=0;park_sources=0;extra_source=-1;
                    if(!batch_failed && sim.current_display()==A[a]){
                        double sc=alt.size()+route_state_value();collect_portfolio(alt);
                        if(sc<best-1e-9){best=sc;best_state=save_route_state();best_moves=move(alt);}
                    }
                    if(!park || routing_budget::expired(route_deadline))break;
                }
            }
        }
        if(before.main==prod_to_paper[a] && prod_to_pair_idx[a]>=0 && !routing_budget::expired(route_deadline)){
            restore_route_state(before);set_route_policy(0);post_support_rank=0;stay_main=true;runtime_pre_limit=0;
            auto alt=query_impl(a);stay_main=false;
            if(!batch_failed){assert(sim.current_display()==A[a]);double sc=alt.size()+route_state_value();collect_portfolio(alt);
                if(sc<best-1e-9){best=sc;best_state=save_route_state();best_moves=move(alt);}}
        }
        post_support_rank=0;
        restore_route_state(best_state);
        set_route_policy(0);
        return best_moves;
    }

    // Rank joint destinations for the previous main and its outstanding loans.
    // All policy alternatives for one request start from exactly this deck.
    void prepare_batch(int a_r,int p_new,int w_new,uint64_t target_mask) {
        const int paired=prod_to_pair_idx[a_r]>=0;
        auto& sources=batch_sources;
        sources={active_main};
        sources.insert(sources.end(),active_supports.begin(),active_supports.end());
        sort(sources.begin(),sources.end(),[&](int a,int b){return sim.depth_of(a)<sim.depth_of(b);});
        const int n=sources.size();
        array<bool,K> is_source{};
        for(int k:sources)is_source[k]=true;
        uint64_t holes=index_main_mask&~opaque[p_new];
        array<int,64> run{};uint64_t wrong=0,second=0,third=0,done=0;
        for(int k:sim.order)if(k>=tot_B && k!=p_new && !is_source[k]) {
            int w=paper_bucket[k];
            if(w<C || w>=T)continue;
            uint64_t bit=1ULL<<w;
            if(done&bit)continue;
            if(!((black[k]^target_mask)&bit)){done|=bit;continue;}
            if(++run[w]==1)wrong|=bit;
            else if(run[w]==2)second|=bit;
            else if(run[w]==3)third|=bit;
        }
        struct Donor {uint64_t cover,bad,home;int paper,cut,rank;};
        vector<Donor> donors;
        overlays.fill(BatchPlan{});
        struct Later {uint64_t cover,home;int cost;};
        vector<Later> later;
        array<int,64> depth{};
        for(int k:sim.order)if(k>=tot_B && k!=p_new && !is_source[k]){
            int w=paper_bucket[k];
            if(w<C || w>=T)continue;
            int rank=depth[w]++;
            uint64_t bit=1ULL<<w;
            if(rank>2 || w==w_new)continue;
            uint64_t bad=(black[k]^target_mask)&opaque[k]&holes;
            if(rank<=1){
                uint64_t previous=0;
                for(int cut=w-1;cut>=C-1;--cut){
                    uint64_t cover=opaque[k]&holes&(~0ULL<<(cut+1));
                    if(cover==previous)continue;
                    previous=cover;
                    uint64_t colors=cover&~bad;
                    if(__builtin_popcountll(colors)<3 || !(colors&wrong))continue;
                    donors.push_back({cover,bad&cover,bit,k,cut,rank});
                }
            }
            if(!(bit&holes) || (bad&bit))continue;
            int after=bad?64-__builtin_clzll(bad):C;
            uint64_t cover=opaque[k]&holes&(~0ULL<<after);
            int cost=rank+2;
            if(__builtin_popcountll(cover&wrong)+__builtin_popcountll(cover&second)+__builtin_popcountll(cover&third)>cost)
                later.push_back({cover,bit,cost});
        }
        auto& placements=batch_placements;
        for(auto& choices:placements)choices.clear();
        int last_back=0;for(int b=0;b<tot_B;b++)last_back=max(last_back,sim.depth_of(b));
        for(int i=0;i<n;i++) {
            int k=sources[i],w=paper_bucket[k],old_cut=k==active_main?C-1:support_cut[k];
            uint64_t home=1ULL<<w;
            placements[i].push_back({w,(!((black[k]^target_mask)&home)?home:0),0,8,false});
            if(w==w_new)continue;
            uint64_t previous=~0ULL;
            for(int cut=w-1;cut>=C-1;--cut){
                if(cut<w_new && (opaque[k]>>w_new&1))continue;
                uint64_t cover=holes&opaque[k]&(~0ULL<<(cut+1));
                bool keep=(cut==old_cut && (!paired || sim.depth_of(k)>last_back));
                if(!keep && (cover==previous || !(cover&wrong)))continue;
                previous=cover;
                placements[i].push_back({cut,cover,cover&(black[k]^target_mask),keep?5:13,keep});
            }
        }
        array<int,3> current{},ids{};
        auto& bests=batch_plans;
        bests.fill(BatchPlan{});
        unsigned steps=0;
        bool expired=false;
        auto choose=[&](auto&& self,int i,int price)->void {
            if(expired || (price>=bests.back().score && price+13>=max(overlays[1].score,overlays[2].score)))return;
            if((++steps&255)==0 && routing_budget::expired(route_deadline)){expired=true;return;}
            if(i<n){
                for(int j=0;j<int(placements[i].size());j++){
                    current[i]=j;
                    self(self,i+1,price+placements[i][j].price);
                }
                return;
            }
            uint64_t covered=0,bad=0,repaired=0;
            for(int j=0;j<n;j++)ids[j]=j;
            // Draw low placements first. The final physical order on ties
            // is checked again before any route is accepted.
            for(int a=1;a<n;a++)for(int b=a;b>0;b--){
                if(placements[ids[b-1]][current[ids[b-1]]].cut>=placements[ids[b]][current[ids[b]]].cut)break;
                swap(ids[b-1],ids[b]);
            }
            for(int t=0;t<n;t++){
                int j=ids[t];const auto &p=placements[j][current[j]];
                if(current[j]==0)repaired|=p.seen;
                else {bad=(bad&~p.seen)|p.bad;covered|=p.seen;}
            }
            uint64_t left=holes&~(covered|repaired);
            int retained=0;for(int j=0;j<n;j++)retained+=current[j]!=0;
            if(retained>0 && retained<runtime_support_limit)for(const Donor& donor:donors){
                bool early=!(holes&donor.home) || (covered&donor.home);
                if(bad&~(donor.cover&~donor.bad))continue;
                if(early && donor.cut<w_new && ((opaque[donor.paper]>>w_new)&1))continue;
                bool clash=false;
                for(int j=0;j<n;j++)if(current[j] && paper_bucket[sources[j]]==paper_bucket[donor.paper])clash=true;
                if(clash)continue;
                uint64_t blocked=0;
                for(int j=0;j<n;j++)if(current[j]){
                    const auto &p=placements[j][current[j]];
                    if(p.cut<donor.cut)blocked|=p.seen;
                }
                const uint64_t paint=donor.cover&~blocked;
                if((donor.bad&paint) || (bad&~paint))continue;
                const uint64_t residual=left&~paint;
                const int score=price+13+8*donor.rank+8*(__builtin_popcountll(residual&wrong)+
                    __builtin_popcountll(residual&second)+__builtin_popcountll(residual&third));
                BatchPlan proposal{current,score,donor.paper,donor.cut};
                int lo=early?2:0,hi=early?3:2;
                for(int z=lo;z<hi;z++)if(proposal.score<overlays[z].score)swap(overlays[z],proposal);
            }
            if(bad)return;
            int score=price+8*(__builtin_popcountll(left&wrong)+__builtin_popcountll(left&second)+__builtin_popcountll(left&third));
            int gain=0;
            for(const auto& opt:later) if(!(covered&opt.home)) {
                uint64_t z=opt.cover&left;
                gain=max(gain,__builtin_popcountll(z&wrong)+__builtin_popcountll(z&second)
                              +__builtin_popcountll(z&third)-opt.cost);
            }
            int loans=0;for(int j=0;j<n;j++)loans+=current[j]>0;
            if(loans<runtime_support_limit)score-=8*gain;
            BatchPlan candidate{current,score};
            for(auto& plan:bests)if(candidate.score<plan.score)swap(plan,candidate);
        };
        choose(choose,0,0);
    }

    vector<pair<int,int>> query_impl(int a_r) {
        post_first_option_count=0;post_second_option_count=0;
        // A policy can diverge only at one of the decisions marked below.
        policy_effects=(1u<<8)|(1u<<9);
        auto tested_threshold=[&](int gain,int gate){
            if(gain==gate-1)policy_effects|=1u<<4;
            if(gain==gate)policy_effects|=1u<<5;
        };
        uint64_t target_mask = A[a_r];
        batch_failed=false;
        int p_new = prod_to_paper[a_r];
        int w_new = paper_bucket[p_new];
        vector<pair<int, int>> turn_moves;

        auto get_bucket_papers = [&](int u, int exclude_p = -1) {
            vector<int> res;res.reserve(bucket_members[u].size());
            for(int p:bucket_members[u])if(p!=exclude_p)res.push_back(p);
            sort(res.begin(),res.end(),[&](int x,int y){return sim.depth_of(x)<sim.depth_of(y);});
            return res;
        };


        // The next order can receive the outgoing main at the front of its
        // bucket. A second head of the same color is therefore less urgent.
        const int return_color=int((black[p_new]>>w_new)&1);
        const double return_credit=.25*hole_request_count[w_new][return_color];
        auto future_order_cost = [&](const vector<int>& items, int omitted, bool bucket_mdp_insert=false) {
            vector<int> b;
            for(int k:items) if(k!=omitted)b.push_back(k);
            double cost=0;
            for(int i=0;i<(int)b.size();i++)cost+=(1+int(paper_is_paired[b[i]]))*(i+1);
            if(b.size()>=2) {
                int w=paper_bucket[b[0]];
                if(w>=C) {
                    // Approximate repair demand at this anchor from the whole catalog.
                    int first[2]={(int)b.size(),(int)b.size()};
                    for(int i=0;i<(int)b.size();i++)
                        first[sim.S[b[i]][w]-'0']=min(first[sim.S[b[i]][w]-'0'],i);
                    cost+=.5*(hole_request_count[w][0]*first[0]+hole_request_count[w][1]*first[1]);
                    if(route_branching && omitted>=0 && paper_bucket[omitted]==w){
                        int color=(black[omitted]>>w)&1;
                        cost-=.25*hole_request_count[w][color]*first[color];
                    }
                    if(first[0]+first[1]==1)
                        cost+=.5*(int(paper_is_paired[b[0]])-int(paper_is_paired[b[1]]));
                } else if(sim.S[b[0]][w]!=sim.S[b[1]][w]) {
                    cost+=.5*((1+int(paper_is_paired[b[0]]))-(1+int(paper_is_paired[b[1]])));
                } else {
                    int run=1;
                    while(run<(int)b.size() && sim.S[b[run]][w]==sim.S[b[0]][w])run++;
                    cost+=HEAD_PENALTY*(run-1);
                }
            }
            if(bucket_mdp_insert)cost=bucket_mdp_adjust(int(b.size()),[&](int i){return b[i];},cost);
            for(int i=0;i<(int)b.size()&&i<32;i++)cost-=support_order_weight*support_utility[b[i]][i];
            return cost;
        };
        // Evaluate an insertion without allocating either the inserted
        // sequence or its omitted-paper copy. Arithmetic matches the vector
        // objective above, including the floating-point accumulation order.
        auto future_order_cost_view = [&](int count,const auto& item,int omitted) {
            double cost=0;
            for(int i=0;i<count;i++)cost+=(1+int(paper_is_paired[item(i)]))*(i+1);
            if(count>=2) {
                int w=paper_bucket[item(0)];
                if(w>=C) {
                    // Approximate repair demand at this anchor from the whole catalog.
                    int first[2]={count,count};
                    for(int i=0;i<count;i++)
                        first[sim.S[item(i)][w]-'0']=min(first[sim.S[item(i)][w]-'0'],i);
                    cost+=.5*(hole_request_count[w][0]*first[0]+hole_request_count[w][1]*first[1]);
                    if(route_branching && omitted>=0 && paper_bucket[omitted]==w){
                        int color=(black[omitted]>>w)&1;
                        cost-=.25*hole_request_count[w][color]*first[color];
                    }
                    if(first[0]+first[1]==1)
                        cost+=.5*(int(paper_is_paired[item(0)])-int(paper_is_paired[item(1)]));
                } else if(sim.S[item(0)][w]!=sim.S[item(1)][w]) {
                    cost+=.5*((1+int(paper_is_paired[item(0)]))-(1+int(paper_is_paired[item(1)])));
                } else {
                    int run=1;
                    while(run<count && sim.S[item(run)][w]==sim.S[item(0)][w])run++;
                    cost+=HEAD_PENALTY*(run-1);
                }
            }
            cost=bucket_mdp_adjust(count,item,cost);
            for(int i=0;i<count&&i<32;i++)cost-=support_order_weight*support_utility[item(i)][i];
            return cost;
        };
        auto insert_best = [&](int k, const vector<int>& others, int minimum, int omitted, bool is_return) {
            if(is_return)policy_effects|=(1u<<1)|(1u<<2);
            double best=1e100;int pos=minimum;
            int w=paper_bucket[k],count=others.size()+1;
            static_assert(K<256,"compact insertion-cache key requires byte-sized IDs");
            const bool cache_requested=route_branching && nov9_insert_enabled && !(is_return && k==return_alternative);
            const int key_length=7+int(others.size());
            const bool cacheable=cache_requested && key_length<=nov9_insert_fast::Table::key_capacity &&
                0<=k && k<K && 0<=minimum && minimum<=int(others.size()) &&
                others.size()<K && -1<=omitted && omitted<K;
            if(cache_requested && key_length>nov9_insert_fast::Table::key_capacity)nov9_insert_long_keys++;
            std::array<uint8_t,nov9_insert_fast::Table::key_capacity> cache_key;
            uint64_t cache_hash=0;int cache_slot=-1,cached_position=0;double cached_cost=0;
            bool cache_hit=false;
            if(cacheable) {
                bool home_loan=false;
                for(int support:active_supports)home_loan|=paper_bucket[support]==w;
                cache_key[0]=uint8_t(k);cache_key[1]=uint8_t(minimum);
                cache_key[2]=uint8_t(omitted+1);cache_key[3]=uint8_t(is_return);
                cache_key[4]=uint8_t(route_mode&3);cache_key[5]=uint8_t(home_loan);
                cache_key[6]=uint8_t(others.size());
                for(int i=0;i<int(others.size());i++)cache_key[7+i]=uint8_t(others[i]);
                cache_hash=nov9_insert_fast::Table::hash_key(cache_key.data(),key_length);
                cache_slot=nov9_insert_positions.locate(cache_key.data(),key_length,cache_hash,cache_hit);
                if(cache_hit) {
                    const auto&entry=nov9_insert_positions.entries[cache_slot];
                    cached_position=entry.position;cached_cost=entry.cost;
                    cache_hit=cached_position>=minimum && cached_position<=int(others.size());
                }
                nov9_insert_requests++;
                if(cache_hit)nov9_insert_hits++;else nov9_insert_misses++;
                if(cache_slot<0)nov9_insert_full++;
            }
            bool scan=!cache_hit;
#ifdef NOV9_INSERT_CACHE_VERIFY
            scan=true;
#endif
            if(scan) {
                nov9_insert_scanned+=max(0,int(others.size())-minimum+1);
                int omitted_other=-1,target_other=-1;
                for(int i=0;i<int(others.size());i++) {
                    if(others[i]==omitted)omitted_other=i;
                    if(others[i]==p_new)target_other=i;
                }
                for(int p=minimum;p<=(int)others.size();p++) {
                    if(is_return && k==return_alternative && (p==0)!=return_front)continue;
                    auto inserted_at=[&](int i) {return i==p?k:others[i-int(i>p)];};
                    int skip=k==omitted?p:(omitted_other<0?-1:omitted_other+int(omitted_other>=p));
                    auto filtered_at=[&](int i) {return inserted_at(i+int(skip>=0 && i>=skip));};
                    double c=future_order_cost_view(count-int(skip>=0),filtered_at,omitted);
                    if(is_return && (route_mode&3))c=1e-5*c+((route_mode&3)==1 ? .01*p : -.01*p);
                    if(is_return) {
                        if(w==w_new) {
                            int idx=k==p_new?p:(target_other<0?count:target_other+int(target_other>=p));
                            c+=NOW_WEIGHT*idx;
                        } else if(sim.S[p_new][w]=='.') {
                            char want=((target_mask>>w)&1)?'1':'0';
                            int idx=0;while(idx<count && sim.S[inserted_at(idx)][w]!=want)idx++;
                            c+=NOW_WEIGHT*idx;
                        }
                    }
    #ifdef COL3_FAST_VERIFY
                    {
                        auto seq=others;seq.insert(seq.begin()+p,k);
                        double reference=future_order_cost(seq,omitted,true);
                        if(is_return && (route_mode&3))reference=1e-5*reference+((route_mode&3)==1 ? .01*p : -.01*p);
                        if(is_return) {
                            if(w==w_new) {
                                int idx=find(seq.begin(),seq.end(),p_new)-seq.begin();
                                reference+=NOW_WEIGHT*idx;
                            } else if(sim.S[p_new][w]=='.') {
                                char want=((target_mask>>w)&1)?'1':'0';
                                int idx=0;while(idx<(int)seq.size() && sim.S[seq[idx]][w]!=want)idx++;
                                reference+=NOW_WEIGHT*idx;
                            }
                        }
                        assert(c==reference);
                        vector<int> filtered;for(int paper:seq)if(paper!=omitted)filtered.push_back(paper);
                        assert(int(filtered.size())==count-int(skip>=0));
                        for(int i=0;i<int(filtered.size());i++)assert(filtered[i]==filtered_at(i));
                    }
    #endif
                    if(c<best){best=c;pos=p;}
                }
            }
            if(cacheable) {
                if(cache_hit) {
#ifdef NOV9_INSERT_CACHE_VERIFY
                    if(pos!=cached_position || best!=cached_cost) {
                        cerr<<"NOV9_INSERT_CACHE_MISMATCH paper="<<k<<" position="<<pos
                            <<" cached="<<cached_position<<" cost="<<best<<" cached_cost="<<cached_cost<<"\n";
                        std::abort();
                    }
#endif
                    pos=cached_position;best=cached_cost;
                } else if(cache_slot>=0) {
                    assert(minimum<=pos && pos<=int(others.size()));
                    nov9_insert_positions.store(cache_slot,cache_key.data(),key_length,cache_hash,pos,best);
                    nov9_insert_peak=max(nov9_insert_peak,(long long)nov9_insert_positions.used);
                }
            }
            int depth=pos==(int)others.size()?sim.depth_of(others.back()):sim.depth_of(others[pos])-1;
            sim.move_paper(k,depth,turn_moves);
        };
        auto send_below_target = [&](int k,int target,const vector<int>& bp) {
            vector<int> others(bp.begin()+1,bp.end());
            int minimum=find(others.begin(),others.end(),target)-others.begin()+1;
            insert_best(k,others,minimum,target,false);
        };
        // Return incompatible supports; retained supports stay between their original buckets.
        sort(active_supports.begin(),active_supports.end(),[&](int x,int y){return sim.depth_of(x)<sim.depth_of(y);});
        auto visible_now=[&](int k) {
            uint64_t above=0;
            for(int q:sim.order) {if(q==k) return bool(opaque[k]&~above);above|=opaque[q];}
            return false;
        };
        vector<bool> loan(K,false);
        for(int k:active_supports)loan[k]=true;
        uint64_t returning_anchors=0;
        for(int k:active_supports)returning_anchors|=1ULL<<paper_bucket[k];
        if(active_main>=0)returning_anchors|=1ULL<<paper_bucket[active_main];
        auto return_support=[&](int support) {
            int w=paper_bucket[support];
            auto bp=get_bucket_papers(w,support);
            insert_best(support,bp,0,-1,true);
            loan[support]=false;
        };
        auto return_main_now=[&]() {
            if(active_main<0)return;
            auto bp=get_bucket_papers(paper_bucket[active_main],active_main);
            insert_best(active_main,bp,0,p_new,true);
            active_main=-1;
        };
        int pidx=prod_to_pair_idx[a_r],pending=-1,pending_cut=-1;
        if(stay_main){
            int last=0;for(int k=0;k<tot_B;k++)last=max(last,sim.depth_of(k));
            uint64_t holes=index_main_mask&~opaque[p_new];vector<int> kept;
            for(int k:active_supports){
                uint64_t cover=holes&opaque[k]&(~0ULL<<(support_cut[k]+1));
                if(sim.depth_of(k)>last && !(cover&(black[k]^target_mask))){kept.push_back(k);continue;}
                if(!visible_now(k)){batch_failed=true;return {};}
                return_support(k);
            }
            active_supports=move(kept);
            if(!switch_pair_in_place(a_r,turn_moves)){batch_failed=true;return {};}
        }else if(keep_sources){
            vector<int> list={active_main};list.insert(list.end(),active_supports.begin(),active_supports.end());
            vector<int> kept,parked,release;
            for(int j=0;j<int(list.size());j++)((keep_sources>>j&1)?kept:(park_sources>>j&1)?parked:release).push_back(list[j]);
            if(extra_source>=0)kept.push_back(extra_source);
            active_main=-1;active_supports=kept;
            sort(release.begin(),release.end(),[&](int x,int y){return sim.depth_of(x)<sim.depth_of(y);});
            for(int k:release){
                if(!visible_now(k)){batch_failed=true;return {};}
                auto others=get_bucket_papers(paper_bucket[k],k);insert_best(k,others,0,p_new,true);
            }
            if(!route_with_sources(a_r,turn_moves)){batch_failed=true;return {};}
            if(!parked.empty()){
                uint64_t seen=opaque[p_new];
                for(int k:sim.order){
                    if(k==p_new)continue;
                    if(find(parked.begin(),parked.end(),k)!=parked.end() &&
                       (opaque[k] & (black[k]^target_mask) & index_main_mask & ~seen)){batch_failed=true;return {};}
                    seen|=opaque[k];
                }
                active_supports.insert(active_supports.end(),parked.begin(),parked.end());
            }
        }else if(route_mode==8) {
            if(batch_sources.empty())prepare_batch(a_r,p_new,w_new,target_mask);
            const auto& proposal=batch_choice<batch_limit?batch_plans[batch_choice]:overlays[batch_choice-batch_limit];
            if(proposal.score>=100000){batch_failed=true;return {};}
            pending=proposal.donor;pending_cut=proposal.cut;
            if(pending>=0)runtime_pre_limit=0;
            const auto& chosen=proposal.moves;
            const auto& placements=batch_placements;
            const auto& sources=batch_sources;
            int n=sources.size();
            uint64_t holes=index_main_mask&~opaque[p_new];
            array<bool,K> is_source{};
            for(int k:sources)is_source[k]=true;
            bool needs_early=batch_choice==batch_limit+2;
            auto acquire_if_open=[&](){
                if(!needs_early || pending<0)return;
                int w=paper_bucket[pending];uint64_t anchor=uint64_t(1)<<w;
                // A compulsory old source in this bucket first returns to its
                // selected position. Until then its retrieval order is tentative.
                for(int k:sources)if(is_source[k] && paper_bucket[k]==w)return;
                auto bucket=get_bucket_papers(w);
                int owner=-1;
                for(int k:sim.order)if(opaque[k]&anchor){owner=k;break;}
                if(owner!=bucket[0])return;
                while(bucket[0]!=pending){
                    send_below_target(bucket[0],pending,bucket);
                    bucket=get_bucket_papers(w);
                }
                int last=0;for(int b=0;b<tot_B;b++)last=max(last,sim.depth_of(b));
                int dest=-1;
                for(int d=last+1;d<K;d++){
                    int k=sim.order[d];if(k==p_new || is_source[k])continue;
                    if(paper_bucket[k]>pending_cut){dest=d;break;}
                }
                if(dest<0 || dest>sim.depth_of(pending))return;
                sim.move_paper(pending,dest,turn_moves);
                active_supports.push_back(pending);support_cut[pending]=pending_cut;
                is_source[pending]=true;
                pending=-1;
            };
            active_main=-1;active_supports.clear();
            for(int i=0;i<n;i++){
                acquire_if_open();
                int k=sources[i],w=paper_bucket[k];const auto &p=placements[i][chosen[i]];
                if(!p.keep){
                    if(!visible_now(k)){batch_failed=true;return {};}
                    if(chosen[i]==0){
                        auto others=get_bucket_papers(w,k);
                        insert_best(k,others,0,p_new,true);
                        is_source[k]=false;
                    }else{
                        int before=-1;
                        for(int v:sim.order)if(v>=tot_B && v!=p_new && !is_source[v] && paper_bucket[v]>p.cut){before=v;break;}
                        if(before<0){batch_failed=true;return {};}
                        sim.move_paper(k,sim.depth_of(before)-int(sim.depth_of(k)<sim.depth_of(before)),turn_moves);
                    }
                }
                if(chosen[i]){
                    active_supports.push_back(k);
                    support_cut[k]=p.cut;
                }
            }
            acquire_if_open();
            if(needs_early && pending>=0){batch_failed=true;return {};}
            uint64_t seen=opaque[p_new];
            for(int k:sim.order){
                if(k==p_new)continue;
                if(pending<0 && is_source[k] && ((black[k]^target_mask)&opaque[k]&holes&~seen)){
                    batch_failed=true;return {};
                }
                seen|=opaque[k];
            }
        }else{
        vector<int> retained;
        uint64_t future_holes=index_main_mask & ~opaque[p_new];
        for(int support:active_supports) {
            bool core_return=false;
            if(front_support_enabled && prod_to_pair_idx[a_r]!=-1)
                for(int b=0;b<tot_B;b++)core_return|=sim.depth_of(support)<sim.depth_of(b);
            if(core_return)front_support_core_returns++;
            if(support==p_new && !core_return) {retained.push_back(support);continue;}
            int cut=support_cut[support];
            uint64_t cover=future_holes & opaque[support] & (~0ULL<<(cut+1));
            bool accessible_new=(w_new<=cut || sim.S[support][w_new]=='.');
            bool keep=!core_return && accessible_new &&
                      !((black[support]^target_mask)&cover);
            if(keep) {
                retained.push_back(support);continue;
            }
            if(!visible_now(support)) {
                return_main_now();
                while(!visible_now(support)) {
                    int upper=-1;
                    for(int k:retained) if(sim.depth_of(k)<sim.depth_of(support) &&
                        (upper<0 || sim.depth_of(k)<sim.depth_of(upper)))upper=k;
                    assert(upper>=0);
                    return_support(upper);
                    retained.erase(find(retained.begin(),retained.end(),upper));
                }
            }
            int w=paper_bucket[support];
            uint64_t bad=(black[support]^target_mask)&opaque[support]&future_holes;
            int new_cut=max(cut,bad ? 63-__builtin_clzll(bad):C-1);
            if(sim.S[support][w_new]!='.')new_cut=max(new_cut,w_new);
            bool migrated=false;
            if(new_cut<w && support!=p_new) {
                int first=-1;
                for(int k:sim.order) if(k>=tot_B && k!=active_main && k!=p_new && !loan[k] && paper_bucket[k]>new_cut) {
                    first=k;break;
                }
                if(first>=0) {
                    uint64_t above=opaque[p_new];
                    for(int k:sim.order) {
                        if(k==first)break;
                        if(k!=support && k!=active_main)above|=opaque[k];
                    }
                    uint64_t visible_cover=opaque[support]&future_holes&~above&~returning_anchors;
                    uint64_t bg=black[p_new],seen=opaque[p_new];
                    for(int k:sim.order) {
                        if(k==support || k==active_main || k==p_new)continue;
                        if(loan[k] && find(retained.begin(),retained.end(),k)==retained.end())continue;
                        bg|=black[k]&~seen;seen|=opaque[k];
                    }
                    int gain=__builtin_popcountll((bg^target_mask)&visible_cover)-runtime_return_penalty;
                    tested_threshold(gain,MIGRATE_GAIN);
                    if(gain>=MIGRATE_GAIN) {
                        int d=sim.depth_of(first)-int(sim.depth_of(support)<sim.depth_of(first));
                        sim.move_paper(support,d,turn_moves);
                        support_cut[support]=new_cut;
                        retained.push_back(support);
                        migrated=true;
                    }
                }
            }
            if(!migrated)return_support(support);
        }
        active_supports=retained;

        // Step 1: 前ターンの主紙 active_main をバケット w_old にスマート返却（先頭に同色が偏るのを抑える）

        if(active_main!=-1 && active_main!=p_new && !(route_mode&4)) {
            int wold=paper_bucket[active_main];
            uint64_t holes=index_main_mask & ~opaque[p_new];
            uint64_t bad=(black[active_main]^target_mask)&opaque[active_main]&holes;
            int cut=bad ? 63-__builtin_clzll(bad) : C-1;
            // The new main must stay accessible at its anchor before promotion.
            if(sim.S[active_main][w_new]!='.') cut=max(cut,w_new);
            if(cut<wold) {
                uint64_t seen=0,bg=0;
                for(int k:sim.order) if(k!=active_main) {bg|=black[k]&~seen;seen|=opaque[k];}
                uint64_t cover=holes & opaque[active_main] & (~0ULL<<(cut+1));
                cover &= ~(1ULL<<wold);
                int gain=__builtin_popcountll((bg^target_mask)&cover)-runtime_return_penalty;
                tested_threshold(gain,MIN_GAIN);
                if(gain>=MIN_GAIN) {
                    int dest=-1;
                    for(int k:sim.order) if(k!=active_main && k>=tot_B &&
                        find(active_supports.begin(),active_supports.end(),k)==active_supports.end() && paper_bucket[k]>cut) {dest=sim.depth_of(k)-1;break;}
                    if(dest>=0) {
                        // The new singleton main covers the entire suffix. If this
                        // old main is compatible at every exposed prefix hole and
                        // leaves the new anchor open, moving it below the backing
                        // stack is unnecessary for the current query.
                        bool leave_in_front=front_support_enabled && route_branching && pidx==-1 && cut==C-1
                            && sim.S[active_main][w_new]=='.';
                        if(leave_in_front)front_support_kept++;
                        else sim.move_paper(active_main,dest,turn_moves);
                        active_supports.push_back(active_main);
                        policy_effects|=1u<<3;
                        support_cut[active_main]=cut;
                        active_main=-1;
                    }
                }
            }
        }
        if (active_main != -1 && (active_main != p_new || pidx != -1)) {
            int w_old = paper_bucket[active_main];
            auto bp = get_bucket_papers(w_old, active_main);
            insert_best(active_main,bp,0,p_new,true);
            active_main = -1;
        }

        }
        late_donor=-1;
        if(add_backing && !borrow_backing(a_r,turn_moves)){batch_failed=true;return {};}
        if(pidx!=-1 && !add_backing && !keep_sources && !stay_main) {if(graph)graph->route(sim,a_r,turn_moves);else {bool ok=route_graph(sim,opaque,black,p_new,target_mask,tot_B,T,C,turn_moves);assert(ok);}}

        // Acquire a support while its anchor is still exposed, before the new main covers it.
        {
        uint64_t holes=index_main_mask & ~opaque[p_new];
        for(int layer=0;layer<runtime_pre_limit && int(active_supports.size())-
            int(find(active_supports.begin(),active_supports.end(),p_new)!=active_supports.end())<runtime_support_limit &&
            __builtin_popcountll(holes&((1ULL<<w_new)-1))>=2;layer++) {
            uint64_t diff=(sim.current_display() ^ target_mask)&holes;
            array<uint64_t,K+1> prefix_opaque;
            array<uint64_t,65> above_cut;
            array<int,64> first_depth;
            first_depth.fill(K);
            array<bool,K> loans{};
            for(int k:active_supports)loans[k]=true;
            uint64_t locked=0,seen=0;
            for(int d=0;d<K;d++) {
                const int k=sim.order[d],w=paper_bucket[k];
                prefix_opaque[d]=seen|opaque[p_new];
                if(k>=tot_B && k!=p_new && !loans[k] && first_depth[w]==K){
                    first_depth[w]=d;
                    locked|=seen&(1ULL<<w);
                }
                seen|=opaque[k];
            }
            int first_later=K;
            for(int cut=T-1;cut>=C-1;cut--) {
                if(cut+1<T)first_later=min(first_later,first_depth[cut+1]);
                above_cut[cut+1]=first_later<K?prefix_opaque[first_later]:~0ULL;
            }
            bool near=false;
            int chosen=-1,chosen_cut=-1,best_gain=PRE_MIN_GAIN-1;
            array<int,64> rank_in_bucket{};
            for(int d=0;d<K;d++) {
                int k=sim.order[d],w=paper_bucket[k];
                if(k==p_new || k<tot_B || w>=w_new) continue;
                if(loans[k])continue;
                int rank=rank_in_bucket[w]++;
                if(locked>>w&1)continue;
                if(runtime_anchorless && (holes & (1ULL<<w))){policy_effects|=1u<<7;continue;}
                uint64_t possible=diff&holes&opaque[k];
                if(__builtin_popcountll(possible)<rank+runtime_return_penalty+PRE_MIN_GAIN)continue;
                uint64_t bad=(black[k]^target_mask)&opaque[k]&index_main_mask;
                if(bad&~above_cut[w])continue;
                int cut=C-1,hi=w-1;
                while(cut<hi){
                    int mid=cut+(hi-cut)/2;
                    if(bad&~above_cut[mid+1])cut=mid+1;else hi=mid;
                }
                uint64_t covered=index_main_mask & opaque[k] & ~above_cut[cut+1];
                int gain=__builtin_popcountll(diff & covered) - rank - 1 - runtime_return_penalty;
                near|=gain==PRE_MIN_GAIN-1;
                if(gain>best_gain) {best_gain=gain;chosen=k;chosen_cut=cut;}
            }
            if(chosen<0){if(near)policy_effects|=1u<<4;break;}
            if(best_gain==PRE_MIN_GAIN)policy_effects|=1u<<5;
            policy_effects|=1u<<6;
            int w=paper_bucket[chosen];
            while(true) {
                auto bp=get_bucket_papers(w);
                int top_p=bp[0];
                if(top_p==chosen) {
                    int dest=sim.depth_of(chosen);
                    for(int k:sim.order) {
                        if(k==p_new || k<tot_B) continue;
                        if(find(active_supports.begin(),active_supports.end(),k)!=active_supports.end()) continue;
                        if(paper_bucket[k]>chosen_cut) {dest=sim.depth_of(k);break;}
                    }
                    sim.move_paper(chosen,dest,turn_moves);
                    active_supports.push_back(chosen);
                    support_cut[chosen]=chosen_cut;
                    break;
                }
                send_below_target(top_p,chosen,bp);
            }
        }

        }

        // Prefixes reaching the same deck and the same remaining decisions
        // share a suffix. Keep the one that got here in fewer moves.
        int tail_start=int(turn_moves.size());
        if(route_branching){
            const int later=add_backing?late_donor:-1;
            for(int h=0;h<used_tails;h++){const auto& t=tails[h];
                if(t.cost>tail_start || t.main!=active_main || t.limit!=runtime_support_limit ||
                        t.penalty!=runtime_return_penalty || t.rank!=post_support_rank ||
                        t.second_rank!=post_second_rank ||
                        t.late!=later || (later>=0 && t.late_at!=late_depth) ||
                        t.pending!=pending || (pending>=0 && t.pending_at!=pending_cut) ||
                        t.supports!=active_supports || t.start!=sim.order)continue;
                bool ok=true;
                for(int k:active_supports)ok&=support_cut[k]==t.cuts[k];
                if(!ok)continue;
                batch_failed=true;return {};
            }
            if(used_tails==int(tails.size()))tails.emplace_back();
            auto& t=tails[used_tails++];t.cost=tail_start;
            t.start=sim.order;t.supports=active_supports;t.cuts=support_cut;t.main=active_main;
            t.limit=runtime_support_limit;t.penalty=runtime_return_penalty;t.rank=post_support_rank;
            t.second_rank=post_second_rank;
            t.late=later;t.late_at=late_depth;t.pending=pending;t.pending_at=pending_cut;
        }
        // Rearrange all blockers jointly, preserving the untouched suffix order.
        auto joint_extract_plan = [&](const vector<int>& bucket) {
            vector<int> empty;
            if(!route_branching || joint_extract_mode==0)return empty;
            int q=find(bucket.begin(),bucket.end(),p_new)-bucket.begin();
            int n=int(bucket.size())-1,m=n-q;
            if(q<2 || q>12 || n>24)return empty;
            string cache_key(1,char(q));for(int k:bucket)cache_key.push_back(char(k));
            auto saved=joint_extract_cache.find(cache_key);
            if(saved!=joint_extract_cache.end()){joint_extract_cache_hits++;return saved->second;}
            if(route_deadline>0 && clock()>=route_deadline)return empty;
            vector<int> free(bucket.begin(),bucket.begin()+q);
            vector<int> tail(bucket.begin()+q+1,bucket.end());
            // Preserve the old choice on ties: only strictly improve its objective.
            vector<int> greedy=bucket;
            while(greedy[0]!=p_new) {
                int k=greedy[0];greedy.erase(greedy.begin());
                int minimum=find(greedy.begin(),greedy.end(),p_new)-greedy.begin()+1;
                int where=minimum;double best=1e100;
                for(int pos=minimum;pos<=int(greedy.size());pos++) {
                    auto seq=greedy;seq.insert(seq.begin()+pos,k);
                    double c=future_order_cost(seq,p_new);
                    if(c<best){best=c;where=pos;}
                }
                greedy.insert(greedy.begin()+where,k);
            }
            greedy.erase(greedy.begin());
            auto joint_cost=[&](const vector<int>& order) {
                double cost=future_order_cost(order,p_new);
                for(int i=0;i<int(order.size())&&i<32;i++)cost+=(1-joint_extract_support_weight)*support_order_weight*support_utility[order[i]][i];
                return cost;
            };
            double greedy_cost=joint_cost(greedy);

            int colors[2]={};vector<int> tail_seen(m+1);
            for(int i=0;i<q;i++)colors[sim.S[free[i]][w_new]-'0']|=1<<i;
            for(int i=0;i<m;i++)tail_seen[i+1]=tail_seen[i]|(1<<(sim.S[tail[i]][w_new]-'0'));
            const int masks=1<<q,stride=m+1;
            vector<double> dp(masks*stride,1e100);
            vector<int8_t> parent(masks*stride,-1);
            dp[0]=0;
            for(int mask=0;mask<masks;mask++) {
                if((mask&63)==0 && routing_budget::expired(route_deadline)) {
                    routing_budget::counters().extract_dp_aborts++;
                    return empty;
                }
                for(int j=0;j<=m;j++) {
                int state=mask*stride+j,position=__builtin_popcount(unsigned(mask))+j;
                double old=dp[state];if(old>1e90)continue;
                int seen=tail_seen[j]|((mask&colors[0])?1:0)|((mask&colors[1])?2:0);
                auto add=[&](int paper,int choice,int next_state) {
                    int color=sim.S[paper][w_new]-'0',after=seen|(1<<color);
                    double delta=(1+int(paper_is_paired[paper]))*(position+1);
                    if(position<32)delta-=joint_extract_support_weight*support_order_weight*support_utility[paper][position];
                    if(n>=2)for(int c=0;c<2;c++)if(!(after>>c&1))delta+=.5*hole_request_count[w_new][c]-(c==return_color?return_credit:0.);
                    if(position==1) {
                        int first=mask?free[__builtin_ctz(unsigned(mask))]:tail[0];
                        if(sim.S[first][w_new]!=sim.S[paper][w_new])
                            delta+=.5*(int(paper_is_paired[first])-int(paper_is_paired[paper]));
                    }
                    double value=old+delta;
                    if(value<dp[next_state]-1e-10){dp[next_state]=value;parent[next_state]=choice;}
                };
                for(int i=0;i<q;i++)if(!(mask>>i&1))add(free[i],i,(mask|(1<<i))*stride+j);
                if(j<m)add(tail[j],q,mask*stride+j+1);
                }
            }
            joint_extract_calls++;
            if(dp.back()>=greedy_cost-1e-7){joint_extract_cache.emplace(cache_key,empty);return empty;}
            vector<int> result;int mask=masks-1,j=m;
            while(mask || j) {
                int choice=parent[mask*stride+j];assert(choice>=0);
                if(choice==q){result.push_back(tail[j-1]);j--;}
                else{result.push_back(free[choice]);mask^=1<<choice;}
            }
            reverse(result.begin(),result.end());
            assert(abs(joint_cost(result)-dp.back())<1e-6);
            joint_extract_improved++;joint_extract_gain+=greedy_cost-dp.back();
            joint_extract_cache.emplace(cache_key,result);
            return result;
        };

        // Step 3: 目的の主紙 p_new をバケット w_new から露出させて深さ 0 へ移動
        if (active_main != p_new) {
            auto initial_bucket=get_bucket_papers(w_new);
            auto plan=joint_extract_plan(initial_bucket);
            if(!plan.empty()) {
                int q=find(initial_bucket.begin(),initial_bucket.end(),p_new)-initial_bucket.begin();
                vector<bool> placed(K,false);
                for(int i=q+1;i<int(initial_bucket.size());i++)placed[initial_bucket[i]]=true;
                for(int i=0;i<q;i++) {
                    int k=initial_bucket[i],index=find(plan.begin(),plan.end(),k)-plan.begin(),next=-1,last=-1;
                    for(int j=index+1;j<int(plan.size());j++)if(placed[plan[j]]){next=plan[j];break;}
                    if(next<0)for(int j=int(plan.size())-1;j>=0;j--)if(placed[plan[j]]){last=plan[j];break;}
                    int depth=next>=0?sim.depth_of(next)-1:sim.depth_of(last>=0?last:p_new);
                    sim.move_paper(k,depth,turn_moves);placed[k]=true;
                }
                auto actual=get_bucket_papers(w_new);assert(actual[0]==p_new);
                actual.erase(actual.begin());assert(actual==plan);
                sim.move_paper(p_new,0,turn_moves);active_main=p_new;
            } else while (true) {
                auto bp = get_bucket_papers(w_new);
                int top_p = bp[0];
                if (top_p == p_new) {
                    sim.move_paper(p_new, 0, turn_moves);
                    active_main = p_new;
                    break;
                } else {
                    send_below_target(top_p,p_new,bp);
                }
            }
        }

        active_supports.erase(remove(active_supports.begin(),active_supports.end(),p_new),active_supports.end());

        if(pending>=0) {
            int w=paper_bucket[pending];
            for(int k:active_supports)if(paper_bucket[k]==w){batch_failed=true;return {};}
            while(true){
                auto bp=get_bucket_papers(w);
                if(!visible_now(bp[0])){batch_failed=true;return {};}
                if(bp[0]==pending)break;
                send_below_target(bp[0],pending,bp);
            }
            int last_back=0;for(int b=0;b<tot_B;b++)last_back=max(last_back,sim.depth_of(b));
            int dest=-1;
            for(int d=last_back+1;d<K;d++){
                int k=sim.order[d];if(k==p_new)continue;
                bool loan=find(active_supports.begin(),active_supports.end(),k)!=active_supports.end();
                if(loan?support_cut[k]>=pending_cut:paper_bucket[k]>pending_cut){dest=d;break;}
            }
            if(dest<0 || dest>sim.depth_of(pending)){batch_failed=true;return {};}
            sim.move_paper(pending,dest,turn_moves);
            active_supports.push_back(pending);support_cut[pending]=pending_cut;
            uint64_t seen=opaque[p_new];
            for(int k:sim.order){
                if(find(active_supports.begin(),active_supports.end(),k)!=active_supports.end() &&
                   ((black[k]^target_mask)&opaque[k]&~seen)){batch_failed=true;return {};}
                seen|=opaque[k];
            }
        }
        if(add_backing && late_donor>=0){
            for(int k:active_supports)if(paper_bucket[k]==paper_bucket[late_donor]){batch_failed=true;return {};}
            if(!visible_now(late_donor)){batch_failed=true;return {};}
            sim.move_paper(late_donor,late_depth+1,turn_moves);
            active_supports.push_back(late_donor);support_cut[late_donor]=C-1;
        }
        // Borrow main sheets across an interval of buckets.
        uint64_t holes=index_main_mask & ~opaque[p_new];
        int support_decision=0;
        const bool want_post_options=post_support_choices>1 || post_second_choices>1;
        const bool allow_post_options=route_branching && want_post_options
            && !routing_budget::expired(route_deadline);
        if(route_branching && want_post_options && !allow_post_options)
            routing_budget::counters().post_option_skips++;
        for(int layer=(int)active_supports.size();layer<runtime_support_limit;layer++) {
            uint64_t diff=sim.current_display() ^ target_mask;
            array<uint64_t,K+1> prefix_opaque;
            array<uint64_t,65> above_cut;
            array<int,64> first_depth;
            first_depth.fill(K);
            array<bool,K> loans{};
            for(int k:active_supports)loans[k]=true;
            uint64_t locked=0,seen=0;
            for(int d=0;d<K;d++) {
                const int k=sim.order[d],w=paper_bucket[k];
                prefix_opaque[d]=seen;
                if(k>=tot_B && k!=p_new && !loans[k] && first_depth[w]==K){
                    first_depth[w]=d;
                    locked|=seen&(1ULL<<w);
                }
                seen|=opaque[k];
            }
            int first_later=K;
            for(int cut=T-1;cut>=C-1;cut--) {
                if(cut+1<T)first_later=min(first_later,first_depth[cut+1]);
                above_cut[cut+1]=first_later<K?prefix_opaque[first_later]:~0ULL;
            }
            bool near=false;
            int chosen=-1,chosen_cut=-1,best_gain=MIN_GAIN-1;
            struct SupportOption {int gain,paper,cut;};
            vector<SupportOption> options;
            array<int,64> rank_in_bucket{};
            for(int d=0;d<K;d++) {
                int k=sim.order[d],w=paper_bucket[k];
                if(k==p_new || k<tot_B) continue;
                if(loans[k])continue;
                int rank=rank_in_bucket[w]++;
                if(locked>>w&1)continue;
                if(!(holes & (1ULL<<w))) continue;
                uint64_t possible=diff&holes&opaque[k];
                if(__builtin_popcountll(possible)<rank+runtime_return_penalty+MIN_GAIN)continue;
                uint64_t bad=(black[k]^target_mask)&opaque[k]&index_main_mask;
                if(bad&~above_cut[w])continue;
                int cut=C-1,hi=w-1;
                while(cut<hi){
                    int mid=cut+(hi-cut)/2;
                    if(bad&~above_cut[mid+1])cut=mid+1;else hi=mid;
                }
                uint64_t covered=index_main_mask & opaque[k] & ~above_cut[cut+1];
                int gain=__builtin_popcountll(diff & covered) - rank - 1 - runtime_return_penalty;
                near|=gain==MIN_GAIN-1;
                if(allow_post_options && gain>=MIN_GAIN)options.push_back({gain,k,cut});
                if(gain>best_gain) {best_gain=gain;chosen=k;chosen_cut=cut;}
            }
            if(support_decision==0)post_first_option_count=int(options.size());
            if(support_decision==1)post_second_option_count=int(options.size());
            const int option_rank=support_decision==0?post_support_rank:
                (support_decision==1?post_second_rank:0);
            if(option_rank>0 && option_rank<int(options.size())) {
                stable_sort(options.begin(),options.end(),[](auto a,auto b){return a.gain>b.gain;});
                auto option=options[option_rank];chosen=option.paper;chosen_cut=option.cut;
            }
            support_decision++;
            if(chosen<0){if(near)policy_effects|=1u<<4;break;}
            if(best_gain==MIN_GAIN)policy_effects|=1u<<5;
            int w=paper_bucket[chosen];
            while(true) {
                auto bp=get_bucket_papers(w);
                int top_p=bp[0];
                if(top_p==chosen) {
                    int dest=sim.depth_of(chosen);
                    for(int k:sim.order) {
                        if(k==p_new || k<tot_B) continue;
                        if(find(active_supports.begin(),active_supports.end(),k)!=active_supports.end()) continue;
                        if(paper_bucket[k]>chosen_cut) {dest=sim.depth_of(k);break;}
                    }
                    sim.move_paper(chosen,dest,turn_moves);
                    active_supports.push_back(chosen);
                    support_cut[chosen]=chosen_cut;
                    break;
                }
                send_below_target(top_p,chosen,bp);
            }
        }

        // Step 4: 残りのインデックス窓 0 .. T-1 のうち不一致の窓をバケット内で可視紙を後送して反転
        uint64_t cur_disp = sim.current_display();
        uint64_t diff_disp = cur_disp ^ target_mask;
        for (int j = 0; j < T; ++j) {
            if ((diff_disp >> j) & 1) {
                auto bp = get_bucket_papers(j, active_main);
                char want_bit = ((target_mask >> j) & 1) ? '1' : '0';
                while (sim.S[bp[0]][j] != want_bit) {
                    int tidx = -1;
                    for (int idx = 1; idx < (int)bp.size(); ++idx) {
                        if (sim.S[bp[idx]][j] == want_bit) { tidx = idx; break; }
                    }
                    vector<int> others(bp.begin()+1,bp.end());
                    insert_best(bp[0],others,tidx,-1,false);
                    bp = get_bucket_papers(j, active_main);
                }
            }
        }


        return turn_moves;
    }
};

// Experimental trace search with a relaxed terminal deck. A candidate may omit
// more than one operation and keep a different order inside each home bucket.
// Unlike trace_shortener, it need not reproduce the original final permutation.
#include <cstring>
namespace nov5_relaxed_trace {
struct Statistics {
    long long calls=0,trials=0,display_ok=0,state_ok=0,accepted=0,saved=0;
    long long skipped_invisible=0,extra_supports=0;
    long long wide_accepts=0,interleaved_supports=0,triple_trials=0;
    long long early_main=0,early_color=0;
    double cpu=0;
};
inline Statistics& stats(){static Statistics x;return x;}

template<class Order>
inline bool rebuild_state(OnlineRouter&router,const Order&order,int product,
                          OnlineRouter::RouteState&state,int max_supports) {
    const int main=router.prod_to_paper[product];
    if(order.empty() || order[0]!=main)return false;
    array<int,K> position{};array<bool,K> is_support{};
    int first_backing=K,last_backing=-1,backing_count=0;
    for(int d=0;d<K;d++) {
        int k=order[d];position[k]=d;
        if(k<router.tot_B){first_backing=min(first_backing,d);last_backing=d;backing_count++;}
    }
    if(backing_count!=router.tot_B)return false;
    static const bool wide=!getenv("NOV5_RELAX_WIDE") || atoi(getenv("NOV5_RELAX_WIDE"));
    if(!wide) {
        if(router.prod_to_pair_idx[product]>=0 && first_backing!=1)return false;
        for(int d=first_backing;d<=last_backing;d++)if(order[d]>=router.tot_B)return false;
    }
    // Every ordinary sheet before or among the backings is now a front loan.
    // On the next paired request, core_return is true for every such loan.
    // Processing loans in deck order, with return_main_now if needed, exposes
    // each loan's prefix anchor and returns it below every backing. Therefore
    // graph->route still starts with a contiguous backing prefix. For a new
    // singleton, no backing route is needed. Current display validity was
    // checked before this reconstruction, including when it is a paired main.
    for(int d=1;d<last_backing;d++)if(order[d]>=router.tot_B) {
        if(!router.front_support_enabled)return false;
        is_support[order[d]]=true;
    }

    // Ordinary papers must still form the ordered home buckets. A sheet may
    // occur early as a loan, but may not split a home bucket's ordinary papers.
    int next_bucket=router.T;
    for(int d=K-1;d>last_backing;d--) {
        int k=order[d],w=router.paper_bucket[k];
        if(w<router.C || w>=router.T)return false;
        if(w>next_bucket)is_support[k]=true;
        else next_bucket=w;
    }
    vector<int> supports,cuts(K,-1);
    array<int,64> support_home{};support_home.fill(-1);
    for(int d=1;d<K;d++)if(is_support[order[d]]) {
        int k=order[d],w=router.paper_bucket[k];
        if(w<router.C || w>=router.T || w==router.paper_bucket[main])return false;
        // The existing return procedure assumes no two loans share a home
        // bucket. Enforce this before handing a relaxed state back to it.
        if(support_home[w]>=0)return false;
        support_home[w]=k;supports.push_back(k);
    }
    if(int(supports.size())>max_supports)return false;
    int previous=router.C-1;
    array<int,K+1> next_regular{};next_regular[K]=router.T;
    for(int d=K-1;d>=0;d--) {
        int k=order[d];next_regular[d]=next_regular[d+1];
        if(k!=main && k>=router.tot_B && !is_support[k])next_regular[d]=router.paper_bucket[k];
    }
    for(int d=1;d<K;d++) {
        int k=order[d];if(k<router.tot_B)continue;
        int w=router.paper_bucket[k];
        if(is_support[k]) {
            int cut=previous;
            if(cut>=w || next_regular[d+1]<=cut)return false;
            cuts[k]=cut;
        }else {
            if(w<previous)return false;
            previous=w;
        }
    }
    state.order.assign(order.begin(),order.end());state.position.assign(position.begin(),position.end());
    state.supports=move(supports);state.cuts=move(cuts);state.main=main;
    return true;
}

inline vector<pair<int,int>> optimize(OnlineRouter&router,int product,
                                      const vector<int>&start,vector<pair<int,int>> moves,
                                      clock_t deadline) {
    static const bool enabled=!getenv("NOV5_RELAX") || atoi(getenv("NOV5_RELAX"));
    static const double budget=search_budget::seconds(getenv("NOV5_RELAX_BUDGET")?atof(getenv("NOV5_RELAX_BUDGET")):
        (search_budget::integer("ROUTE_PORTFOLIO",1)!=0?.9:.52));
    static const int pair_limit=getenv("NOV5_RELAX_PAIRS")?max(0,atoi(getenv("NOV5_RELAX_PAIRS"))):100;
    static const int triple_limit=getenv("NOV5_RELAX_TRIPLES")?max(0,atoi(getenv("NOV5_RELAX_TRIPLES"))):24;
    static const int max_supports=getenv("NOV5_RELAX_SUPPORTS")?clamp(atoi(getenv("NOV5_RELAX_SUPPORTS")),0,12):4;
    static const int minimum=getenv("NOV5_RELAX_MIN_MOVES")?atoi(getenv("NOV5_RELAX_MIN_MOVES")):5;
    auto&log=stats();
    if(!enabled || int(moves.size())<minimum || log.cpu>=budget || routing_budget::expired(deadline))return moves;
    const clock_t begun=clock();
    const clock_t local_deadline=min(deadline,begun+clock_t(max(0.0,budget-log.cpu)*CLOCKS_PER_SEC));
    unsigned tick_count=0;
    bool time_up=false;
    auto expired=[&](){if((tick_count++&31)==0)time_up=clock()>=local_deadline;return time_up;};
    log.calls++;
    const auto original_state=router.save_route_state();
    auto best_state=original_state;
    auto best_moves=moves;
    const double initial_score=moves.size()+router.route_state_value();
    double best_score=initial_score;
    const int m=moves.size();
    struct Action {int paper,left,right,depth,from;};
    vector<Action> actions;actions.reserve(m);
    vector<int> reference=start;
    for(auto [paper,depth]:moves) {
        auto it=find(reference.begin(),reference.end(),paper);
        int from=int(it-reference.begin());
        reference.erase(it);reference.insert(reference.begin()+depth,paper);
        actions.push_back({paper,depth?reference[depth-1]:-1,depth+1<K?reference[depth+1]:-1,depth,from});
    }
    assert(reference==original_state.order);
    static const bool allow_core_absolute=!getenv("NOV5_RELAX_CORE_ABSOLUTE") || atoi(getenv("NOV5_RELAX_CORE_ABSOLUTE"));
    static const int prune_mode=getenv("NOV6_TRACE_PRUNE")?atoi(getenv("NOV6_TRACE_PRUNE")):3;
    static_assert(K<=256,"compact paper/depth representation");
    assert(m<=2*K);
    const int target_main=router.prod_to_paper[product];
    const uint64_t target=router.A[product];
    array<uint8_t,K> compact_start,compact_position;
    array<int,K> action_count{};
    for(const auto&action:actions)action_count[action.paper]++;
    for(int d=0;d<K;d++){compact_start[d]=uint8_t(start[d]);compact_position[start[d]]=uint8_t(d);}
    vector<int> action_papers;action_papers.reserve(m);
    array<uint64_t,K> correct{},fixed_cover{};
    uint64_t fixed_seen=0,fixed_display=0,all_correct=0;
    bool fixed_above_main=false;
    for(int d=0;d<K;d++) {
        const int k=start[d];
        if(action_count[k]) {
            action_papers.push_back(k);
            fixed_cover[k]=router.opaque[k]&~fixed_seen;
            correct[k]=router.opaque[k]&~(router.black[k]^target);
            all_correct|=correct[k];
        }else {
            fixed_display|=router.black[k]&~fixed_seen;fixed_seen|=router.opaque[k];
            fixed_above_main|=d<int(compact_position[target_main]);
        }
    }
    const uint64_t base_wrong=fixed_display^target;
    int last_a=-2,last_b=-2,last_c=-2,last_reason=0;
    auto impossible=[&](int omit_a,int omit_b,int omit_c) {
        // The same omissions are tested with up to three destination modes.
        // Necessary feasibility is independent of those insertion anchors.
        if(omit_a==last_a && omit_b==last_b && omit_c==last_c)return last_reason;
        last_a=omit_a;last_b=omit_b;last_c=omit_c;last_reason=0;
        if(!prune_mode)return 0;
        int omitted[3],multiplicity[3],n_omitted=0;
        for(int index:{omit_a,omit_b,omit_c})if(index>=0) {
            int k=actions[index].paper,j=0;
            while(j<n_omitted && omitted[j]!=k)j++;
            if(j==n_omitted){omitted[j]=k;multiplicity[j]=0;n_omitted++;}
            multiplicity[j]++;
        }
        int new_fixed[3],n_fixed=0;
        for(int i=0;i<n_omitted;i++)if(multiplicity[i]==action_count[omitted[i]])new_fixed[n_fixed++]=omitted[i];
        bool main_fixed=action_count[target_main]==0;
        for(int i=0;i<n_fixed;i++)main_fixed|=new_fixed[i]==target_main;
        if((prune_mode&1) && main_fixed) {
            bool blocked=fixed_above_main;
            for(int i=0;i<n_fixed;i++)blocked|=compact_position[new_fixed[i]]<compact_position[target_main];
            if(blocked)return last_reason=1;
        }
        if(!(prune_mode&2))return 0;
        // Unmoved sheets retain their relative order. Add newly fixed sheets
        // from bottom to top, changing only windows without an earlier old
        // immutable owner. Remaining movable sheets may paint any correct
        // window; granting all of them this freedom is a safe upper bound.
        for(int i=0;i<n_fixed;i++)for(int j=i+1;j<n_fixed;j++)
            if(compact_position[new_fixed[i]]<compact_position[new_fixed[j]])swap(new_fixed[i],new_fixed[j]);
        uint64_t wrong=base_wrong;
        for(int i=0;i<n_fixed;i++) {
            int k=new_fixed[i];uint64_t cover=fixed_cover[k];
            wrong=(wrong&~cover)|((router.black[k]^target)&cover);
        }
        uint64_t possible=all_correct;
        if(n_fixed) {
            possible=0;
            for(int k:action_papers) {
                bool fixed=false;for(int i=0;i<n_fixed;i++)fixed|=k==new_fixed[i];
                if(!fixed)possible|=correct[k];
            }
        }
        if(wrong&~possible)return last_reason=2;
        return 0;
    };
    int repair_begin=m,last_home=router.T;
    while(repair_begin) {
        const auto& op=actions[repair_begin-1];
        if(op.paper<router.tot_B || op.from>=op.depth)break;
        int w=router.paper_bucket[op.paper];
        if(w>last_home)break;
        last_home=w;--repair_begin;
    }
    array<uint64_t,256> repaired{};
    int repairs_tried=0;
    bool different_core=false,different_left=false;
    unsigned drop_tail=0;
    auto trial=[&](int omit_a,int omit_b,bool after_left,int omit_c=-1,bool core_absolute=false,
                   int redirect=-1,int anchor=-1)->bool {
        different_core=different_left=false;
        if(expired())return false;
        log.trials++;log.triple_trials+=omit_c>=0;
        const int doomed=drop_tail?0:impossible(omit_a,omit_b,omit_c);
        log.early_main+=doomed==1;log.early_color+=doomed==2;
#ifndef NOV6_TRACE_VERIFY_PRUNE
        if(doomed==1 || (doomed==2 && (omit_b<0 || repairs_tried>=40)))return false;
#endif
        auto order=compact_start,pos=compact_position;
        array<pair<int,int>,2*K> result;array<uint8_t,2*K> replay_from;
        int result_count=0,invisibles=0;
        // prefix[d] is the opacity above deck position d. Moving a sheet
        // invalidates only prefixes strictly after the first changed position.
        array<uint64_t,K+1> prefix;prefix[0]=0;int prefix_valid=0;
        for(int i=0;i<m;i++) {
            if(i==omit_a || i==omit_b || i==omit_c || (i>=repair_begin && i-repair_begin<32 && (drop_tail>>(i-repair_begin)&1)))continue;
            const auto&action=actions[i];int k=action.paper,from=pos[k];
            while(prefix_valid<from) {
                prefix[prefix_valid+1]=prefix[prefix_valid]|router.opaque[order[prefix_valid]];
                prefix_valid++;
            }
            uint64_t covered=prefix[from];
            if(!(router.opaque[k]&~covered)) {
                // Retaining an earlier sheet can make a later correction both
                // invisible and unnecessary. Skip it, then validate the whole
                // resulting barcode and the next-query state below.
                invisibles++;continue;
            }
            int depth;
            if(i>=repair_begin) {
                uint64_t bit=1ULL<<router.paper_bucket[k];
                int owner=k;
                if(covered&bit)for(int z=0;z<from;z++)if(router.opaque[order[z]]&bit){owner=order[z];break;}
                if(!((router.black[owner]^target)&bit))continue;
            }
            if(redirect>=0 && k<router.tot_B && i>redirect){
                uint64_t rem=~router.index_main_mask&~router.opaque[target_main];
                uint64_t mismatch=0;
                for(int x:order){
                    mismatch|=(router.black[x]^target)&router.opaque[x]&rem;
                    rem&=~router.opaque[x];
                    if(!rem || mismatch)break;
                }
                if(!mismatch)continue;
            }
            if(i==redirect)depth=pos[anchor]-int(from<pos[anchor]);
            else if(k==router.prod_to_paper[product] && action.depth==0)depth=0;
            else if(core_absolute && k<router.tot_B)depth=action.depth;
            else if(after_left && action.left>=0)depth=pos[action.left]+1-int(from<pos[action.left]);
            else if(action.right>=0)depth=pos[action.right]-int(from<pos[action.right]);
            else if(action.left>=0)depth=pos[action.left]+1-int(from<pos[action.left]);
            else depth=0;
            if(k<router.tot_B && depth!=action.depth)different_core=true;
            if(action.left>=0 && depth!=pos[action.left]+1-int(from<pos[action.left]))different_left=true;
            if(depth<0 || depth>=K)return false;
            if(from==depth)continue;
            if(from<depth)std::memmove(order.data()+from,order.data()+from+1,size_t(depth-from));
            else std::memmove(order.data()+depth+1,order.data()+depth,size_t(from-depth));
            order[depth]=uint8_t(k);
            const int first_changed=min(from,depth),last_changed=max(from,depth);
            prefix_valid=min(prefix_valid,first_changed);
            if(last_changed-first_changed<28){
                for(int d=first_changed;d<=last_changed;d++)pos[order[d]]=uint8_t(d);
            }else {
                const uint8_t lo=first_changed+(from<depth),len=last_changed-first_changed;
                const uint8_t delta=from<depth?255:1;
                for(auto& p:pos)p+=uint8_t(p-lo)<len?delta:0;
                pos[k]=uint8_t(depth);
            }
            replay_from[result_count]=from;result[result_count++]={k,depth};
        }
        if(result_count>=int(moves.size()))return false;
        uint64_t displayed=0,seen=0;
        for(int k:order){displayed|=router.black[k]&~seen;seen|=router.opaque[k];if(seen==~0ULL)break;}
        auto consider=[&](const auto& q,int new_count){
            log.display_ok++;
            OnlineRouter::RouteState candidate;
            if(!rebuild_state(router,q,product,candidate,max_supports))return false;
            log.state_ok++;
            router.restore_route_state(candidate);
            double score=new_count+router.route_state_value();
            if(score<best_score-1e-9){
                best_score=score;best_state=move(candidate);best_moves.assign(result.begin(),result.begin()+new_count);
                log.skipped_invisible+=invisibles;
            }
            router.restore_route_state(original_state);
            return true;
        };
        if(displayed==target)return consider(order,result_count);
        if(result_count>=int(best_moves.size()) || order[0]!=target_main || repairs_tried>=40)return false;
        uint64_t bad=displayed^target;
        uint64_t hs=0x92add3479d933ea1ULL;
        for(int j=0;j<K;j+=8){uint64_t word;memcpy(&word,order.data()+j,8);hs=(hs^word)*0x9e3779b97f4a7c15ULL;}
        if(repaired[hs>>56]==hs)return false;
        repaired[hs>>56]=hs;repairs_tried++;
        auto finish=[&](int donor,int dest,const auto& changed,bool visible){
            OnlineRouter::RouteState viable;
            if(!rebuild_state(router,changed,product,viable,max_supports))return false;
            int promote=-1,paid=-1;
            for(int t=0;t<result_count;t++){if(result[t].first==donor)paid=t;
                if(result[t].first==target_main && result[t].second==0)promote=t;}
            int starts[3]={paid,promote,0};
            bool any=false;
            if(visible && result_count+1<int(best_moves.size())){
                result[result_count]={donor,dest};any=consider(changed,result_count+1);
            }
            for(int repl=0;repl<3;repl++){
                int t0=starts[repl];
                if((repl && (visible || result_count+1>=int(best_moves.size()) || paid>=t0)) ||
                   t0<0 || (repl==2 && promote==0))continue;
                if(expired())return any;
                array<int,K> stable{};for(int t=t0;t<result_count;t++)stable[result[t].first]=1;
                int right=dest+1;while(right<K && stable[changed[right]])right++;
                auto actual=compact_start;
                array<pair<int,int>,2*K> ops;
                auto relocate=[](auto& o,int src,int dst){
                    int a=o[src];if(src<dst)memmove(o.data()+src,o.data()+src+1,dst-src);
                    else if(src>dst)memmove(o.data()+dst+1,o.data()+dst,src-dst);o[dst]=a;
                };
                int rp=compact_position[donor],nops=0;bool ok=true;
                for(int t=0;t<=result_count && ok;t++){
                    if(t==t0){
                        int ip=find(actual.begin(),actual.end(),donor)-actual.begin();
                        uint64_t free=router.opaque[donor];for(int p=0;p<ip && free;p++)free&=~router.opaque[actual[p]];
                        if(!free){ok=false;break;}
                        int at=right<K?int(find(actual.begin(),actual.end(),changed[right])-actual.begin()):K;
                        at-=ip<at;
                        if(at!=ip){relocate(actual,ip,at);ops[nops++]={donor,at};}
                    }
                    if(t==result_count)break;
                    auto [p,depth]=result[t];int old_from=replay_from[t];
                    if(p==donor)rp=depth;else {rp-=old_from<rp;rp+=depth<=rp;}
                    if(t==t0 && repl==0)continue;
                    int from=t<t0?old_from:int(find(actual.begin(),actual.end(),p)-actual.begin());
                    int at=depth;
                    if(t>=t0){
                        int ix=find(actual.begin(),actual.end(),donor)-actual.begin();ix-=from<ix;
                        at-=rp<depth;at+=at>ix || (at==ix && viable.position[p]>dest);
                        uint64_t free=router.opaque[p];for(int j=0;j<from&&free;j++)free&=~router.opaque[actual[j]];
                        if(!free){ok=false;break;}
                    }
                    if(from!=at){relocate(actual,from,at);ops[nops++]={p,at};}
                }
                if(!ok || actual!=changed)continue;
                router.restore_route_state(viable);double score=nops+router.route_state_value();
                router.restore_route_state(original_state);
                if(score<best_score){best_score=score;best_state=viable;best_moves.assign(ops.begin(),ops.begin()+nops);}
                any=true;break;
            }
            return any;
        };
        array<uint64_t,K+1> pref;pref[0]=0;int valid=0;
        auto grow=[&](int d){while(valid<d){pref[valid+1]=pref[valid]|router.opaque[order[valid]];valid++;}};
        int above=0;while(above<K && !(router.opaque[order[above]]&bad))above++;
        if(above==0||above==K)return false;
        grow(above);const int upper=above;
        const int top=order[upper],owner=((bad&~router.opaque[top])==0)?top:-1;
        auto& paint=router.paint_index;paint.prepare(router.opaque,router.black);
        uint64_t selectable[3]={~0ULL,~0ULL,~0ULL};
        for(uint64_t b=bad;b;b&=b-1){
            int j=__builtin_ctzll(b),c=int(target>>j&1);
            for(int i=0;i<3;i++)selectable[i]&=paint.ink[c][j][i];
        }
        for(uint64_t b=~(pref[upper]|bad);b && (selectable[0]|selectable[1]|selectable[2]);b&=b-1){
            int j=__builtin_ctzll(b),c=1^int(target>>j&1);
            for(int i=0;i<3;i++)selectable[i]&=~paint.ink[c][j][i];
        }
        for(int block=0;block<3;block++)for(uint64_t b=selectable[block];b;b&=b-1){
            int k=64*block+__builtin_ctzll(b),z=pos[k];
            if(z<=upper)continue;
            grow(z);bool visible=router.opaque[k]&~pref[z];
            if(!visible && !action_count[k] && result_count+1>=int(best_moves.size()))continue;
            uint64_t wrong_ink=router.opaque[k]&(router.black[k]^target);
            for(int d=upper;d>0 && !(wrong_ink&~pref[d]);--d){
                int below=order[d],a=order[d-1];
                if(d!=upper && below>=router.tot_B && a>=router.tot_B &&
                    router.paper_bucket[below]==router.paper_bucket[a])continue;
                auto changed=order;memmove(changed.data()+d+1,changed.data()+d,z-d);changed[d]=k;
                if(finish(k,d,changed,visible))break;
            }
        }
        if(owner>=0 && owner!=target_main){
            int start=pos[owner];grow(start);uint64_t exposed=router.opaque[owner]&~pref[start],curr=displayed;
            for(int d=start+1;d<K && exposed;d++){
                int k=order[d];uint64_t take=exposed&router.opaque[k];
                curr=(curr&~take)|(router.black[k]&take);exposed&=~take;
                if(curr!=target)continue;
                for(int j=d;j<K;j++){
                    if(j>d && (router.opaque[order[j]]&exposed))break;
                    if(j>d && router.paper_bucket[order[j]]==router.paper_bucket[order[j-1]])continue;
                    auto changed=order;memmove(&changed[start],&changed[start+1],j-start);changed[j]=owner;
                    if(finish(owner,j,changed,true))return true;
                }
            }
        }
        return true;
    };
    if(m-repair_begin>=2 && m-repair_begin<24 && !expired()){
        const uint64_t holes=router.index_main_mask&~router.opaque[target_main];
        array<unsigned,96> masks{};int nc=0;
        auto add=[&](unsigned bits){if(__builtin_popcount(bits)<2 || nc>=96)return;
            for(int i=0;i<nc;i++)if(masks[i]==bits)return;
            masks[nc++]=bits;};
        add((1u<<(m-repair_begin))-1);
        for(int k=router.tot_B;k<K && nc<96;k++)if(k!=target_main){
            uint64_t bad=holes&router.opaque[k]&(target^router.black[k]);
            int mincut=bad?64-__builtin_clzll(bad):router.C;
            unsigned subset=0;
            for(int i=m-1;i>=repair_begin;i--){
                int w=router.paper_bucket[actions[i].paper];if(w<mincut)break;
                if(router.opaque[k]>>w&1){subset|=1u<<(i-repair_begin);add(subset);}
            }
        }
        for(int h=0;h<nc && !expired();h++){
            drop_tail=masks[h];trial(-1,-1,false);
            for(int i=0;i<repair_begin && i<4 && !expired();i++)
                if(actions[i].paper!=target_main)trial(i,-1,false);
        }
        drop_tail=0;
    }
    // A returned main or loan must move anyway. In the terminal stack its
    // paper may cover a later repair if parked at a different bucket boundary.
    if(!expired()){
        uint64_t repairs=0;
        for(int i=repair_begin;i<m;i++)repairs|=1ULL<<router.paper_bucket[actions[i].paper];
        array<bool,K> loan{};
        for(int k:original_state.supports)loan[k]=true;
        array<int,64> head{};head.fill(-1);
        array<uint64_t,K> above{};
        uint64_t painted=0;
        for(int k:original_state.order){
            above[k]=painted;painted|=router.opaque[k];
            int w=router.paper_bucket[k];
            if(k!=target_main && k>=router.tot_B && !loan[k] && head[w]<0)head[w]=k;
        }
        for(int i=0;i<repair_begin && !expired();i++){
            int k=actions[i].paper,w=router.paper_bucket[k];
            if(k<router.tot_B || k==target_main || loan[k] || w==router.paper_bucket[target_main] ||
               actions[i].from>=actions[i].depth || action_count[k]!=1)continue;
            uint64_t prev=0;
            if(router.prod_to_pair_idx[product]>=0 && i+1<repair_begin) {
                bool backing_moves=false;
                for(int t=i+1;t<repair_begin;t++)backing_moves|=actions[t].paper<router.tot_B;
                if(backing_moves && !(router.opaque[k]&(1ULL<<router.paper_bucket[target_main]))) {
                    for(int anchor=0;anchor<router.tot_B;anchor++) {
                        uint64_t cover=router.opaque[k]&~above[anchor];
                        if((router.black[k]^target)&cover&router.index_main_mask)continue;
                        trial(-1,-1,false,-1,false,i,anchor);
                    }
                }
            }
            for(int u=router.C;u<=w;u++){
                int anchor=head[u];if(anchor<0 || anchor==k)continue;
                uint64_t cover=router.opaque[k]&~above[anchor];
                if(((router.black[k]^target)&cover) || !(cover&repairs) || cover==prev)continue;
                prev=cover;
                trial(-1,-1,false,-1,false,i,anchor);
            }
        }
    }
    // Try all single deletions first. Its visible-paper failures are themselves
    // omitted, allowing an entire now-occluded repair suffix to disappear.
    for(int i=0;i<m&&!expired();i++) {
        trial(i,-1,false);bool need_left=different_left;
        if(allow_core_absolute && different_core && !expired())trial(i,-1,false,-1,true);
        if(need_left && !expired())trial(i,-1,true);
    }
    // A returned main/support and a subsequent repair can only be removed as a
    // pair. Focus on widely separated operations instead of spending the budget
    // repeatedly deleting adjacent backing moves already solved exactly.
    int tried=0;
    for(int gap=m-1;gap>=1&&tried<pair_limit&&!expired();gap--)
        for(int i=0;i+gap<m&&tried<pair_limit&&!expired();i++) {
            int j=i+gap;
            if(actions[i].paper<router.tot_B && actions[j].paper<router.tot_B)continue;
            trial(i,j,false);tried++;
            if(allow_core_absolute && different_core && !expired())trial(i,j,false,-1,true);
        }
    int triples=0;
    for(int i=0;i+2<m&&triples<triple_limit&&!expired();i++)
        for(int k=m-1;k>=i+2&&triples<triple_limit&&!expired();k--)
            for(int j=k-1;j>i&&triples<triple_limit&&!expired();j--) {
                if(actions[i].paper<router.tot_B && actions[j].paper<router.tot_B &&
                   actions[k].paper<router.tot_B)continue;
                trial(i,j,false,k);triples++;
                if(allow_core_absolute && different_core && !expired())trial(i,j,false,k,true);
            }
    router.restore_route_state(best_state);
    if(best_moves.size()<moves.size()) {
        log.accepted++;log.saved+=moves.size()-best_moves.size();
        log.extra_supports+=int(best_state.supports.size())-int(original_state.supports.size());
        int first=K,last=-1;
        for(int b=0;b<router.tot_B;b++){first=min(first,best_state.position[b]);last=max(last,best_state.position[b]);}
        int interleaved=0;for(int k:best_state.supports)interleaved+=first<best_state.position[k]&&best_state.position[k]<last;
        log.interleaved_supports+=interleaved;
        log.wide_accepts+=interleaved>0 || (router.prod_to_pair_idx[product]>=0 && first>1);
    }
    log.cpu+=double(clock()-begun)/CLOCKS_PER_SEC;
    if(best_moves.size()+1<moves.size() && best_moves.size()>=size_t(minimum))
        return optimize(router,product,start,std::move(best_moves),deadline);
    return best_moves;
}
} // namespace nov5_relaxed_trace

inline bool construction_deadline_expired(clock_t absolute_deadline) {
    return absolute_deadline>0 && clock()>=absolute_deadline;
}

// Keep one witness column for every backing pair, and optimize all excess
// comparison columns independently. Runs after the original lazy MAP search.
inline bool nov8_flex_selected(int offset) {
    if(!search_budget::integer("NOV8_FLEX",1))return false;
    if(const char* mask=getenv("NOV8_FLEX_MASK")) {
        const uint64_t bits=std::strtoull(mask,nullptr,0);
        return offset>=0 && offset<64 && ((bits>>offset)&1);
    }
    const char* configured=getenv("NOV8_FLEX_MODE");
    const string mode=configured?configured:"from";
    if(mode=="none")return false;
    if(mode=="all")return true;
    if(mode=="last")return offset==max(1,search_budget::integer("NOV8_POOL",4))-1;
    return offset>=max(0,search_budget::integer("NOV8_FLEX_FROM",3));
}

inline void nov8_flexible_edges(const vector<uint64_t>&A,int B,int W,int offset,
                               vector<pair<int,int>>&edges,clock_t deadline) {
    if(B!=7 || !nov8_flex_selected(offset))return;
    if(A.empty() || A.size()>256 || W<=0 || W>=64 || int(edges.size())!=W)return;
    const clock_t started=clock();
    const double cap=max(0.0,search_budget::real("NOV8_FLEX_CAP",.08));
    const int iterations=max(0,search_budget::integer("NOV8_FLEX_ITERS",256));
    const int sample_count=max(2,search_budget::integer("NOV8_FLEX_SAMPLES",16));
    const clock_t local_stop=started+clock_t(search_budget::seconds(cap)*CLOCKS_PER_SEC);
    const clock_t stop=deadline>0?min(deadline,local_stop):local_stop;
    auto expired=[&](){return clock()>=stop;};
    if(iterations==0 || expired())return;
    const int T=64-W,needed=64+B;
    const uint64_t prefix_mask=(1ULL<<T)-1;
    bool witness[8][8]={};vector<int>mutable_columns;
    for(int j=0;j<W;j++) {
        auto[u,v]=edges[j];
        if(u<0 || v<0 || u>=B || v>=B || u==v)return;
        if(u>v)swap(u,v);
        if(witness[u][v])mutable_columns.push_back(j);else witness[u][v]=true;
    }
    for(int u=0;u<B;u++)for(int v=u+1;v<B;v++)if(!witness[u][v])return;
    if(mutable_columns.empty())return;
    struct Target {int a,b,hd,full;uint64_t zero,one;};
    vector<Target>targets;targets.reserve(A.size()*(A.size()-1)/2);
    for(int a=0;a<int(A.size());a++) {
        if((a&15)==0 && expired())return;
        for(int b=a+1;b<int(A.size());b++) {
            uint64_t diff=A[a]^A[b];
            if(!(~diff&prefix_mask))continue;
            targets.push_back({a,b,__builtin_popcountll(diff&prefix_mask),
                __builtin_popcountll(diff),(~A[a]&diff)>>T,(A[a]&diff)>>T});
        }
    }
    stable_sort(targets.begin(),targets.end(),[](const auto&a,const auto&b){
        return a.hd!=b.hd?a.hd<b.hd:a.full<b.full;
    });
    uint64_t zeros[64]={},ones[64]={};
    auto assign=[&](int j,pair<int,int>edge,bool add) {
        auto[u,v]=edge;uint64_t bit=1ULL<<j;
        if(add){zeros[u*8+v]|=bit;ones[v*8+u]|=bit;}
        else {zeros[u*8+v]&=~bit;ones[v*8+u]&=~bit;}
    };
    for(int j=0;j<W;j++)assign(j,edges[j],true);
    uint64_t rows[128]={};
    for(int mask=0;mask<(1<<B);mask++)for(int v=0;v<B;v++)if(mask>>v&1)rows[mask]|=127ULL<<(8*v);
    auto acyclic=[&](uint64_t g) {
        unsigned alive=127;
        while(g) {
            uint64_t inc=g;inc|=inc>>32;inc|=inc>>16;inc|=inc>>8;
            unsigned free=alive&~unsigned(inc&255);
            if(!free)return false;
            g&=~rows[free];alive&=~free;
        }
        return true;
    };
    auto render=[&](const Target&t) {
        uint64_t g=0;
        for(int u=0;u<B;u++)for(int v=0;v<B;v++)if(u!=v) {
            int e=u*8+v;if((t.zero&zeros[e])||(t.one&ones[e]))g|=1ULL<<e;
        }
        return g;
    };
    const int n=int(targets.size());
    vector<uint64_t>graphs(n),next_graphs(n);
    vector<uint8_t>valid(n),next_valid(n);
    int feasible=0;
    for(int i=0;i<n;i++) {
        if((i&255)==0 && expired())return;
        graphs[i]=render(targets[i]);valid[i]=acyclic(graphs[i]);feasible+=valid[i];
#ifdef NOV8_FLEX_VERIFY
        uint16_t before[16];const auto&t=targets[i];
        assert(bool(valid[i])==comparison_pair_dag(A[t.a],A[t.b],B,W,edges,before,0));
#endif
    }
    if(expired())return;
    auto samples=graph_sample_orders(A[0]^0xa9f123ef127a29ULL,B,sample_count);
    unordered_map<uint64_t,double>cache;cache.reserve(8192);
    auto metric=[&](uint64_t g) {
        auto saved=cache.find(g);if(saved!=cache.end())return saved->second;
        uint16_t before[16]={};
        uint64_t todo=g;while(todo){int bit=__builtin_ctzll(todo);todo&=todo-1;before[bit%8]|=1u<<(bit/8);}
        double value=graph_dag_metric(before,B,3,samples);cache.emplace(g,value);return value;
    };
    struct Evaluation {double score,cost;int matched;bool complete;};
    auto evaluate=[&](const vector<uint64_t>&gg,const vector<uint8_t>&ok,int count) {
        bool used[256]={};int matched=0;double cost=0;
        for(int i=0;i<n;i++) {
            if((i&255)==0 && expired())return Evaluation{0,cost,matched,false};
            if(!ok[i])continue;
            const auto&t=targets[i];if(used[t.a]||used[t.b])continue;
            used[t.a]=used[t.b]=true;matched++;
            cost+=t.hd+2*metric(gg[i]);
            if(matched==needed)break;
        }
        return Evaluation{-cost-1000.0*(needed-matched)+.001*count,cost,matched,true};
    };
    auto initial=evaluate(graphs,valid,feasible),current=initial,best=initial;
    if(!initial.complete)return;
    auto best_edges=edges;int initial_feasible=feasible,best_feasible=feasible,accepted=0,done=0;
    uint64_t rng=A[0]^(0x97bc179e3aef9107ULL*(offset+1));if(!rng)rng=1;
    auto rnd=[&](){rng^=rng<<7;rng^=rng>>9;return uint32_t(rng);};
    bool interrupted=false;
    for(int it=0;it<iterations && !expired();it++) {
        int x=mutable_columns[rnd()%mutable_columns.size()],y=-1;
        auto oldx=edges[x],oldy=oldx,newx=oldx,newy=oldy;
        if((rnd()&3)==0 && mutable_columns.size()>1) {
            y=mutable_columns[rnd()%mutable_columns.size()];if(y==x)continue;
            oldy=edges[y];newx=oldy;newy=oldx;
        }else {int u=rnd()%B,v=rnd()%(B-1);if(v>=u)v++;newx={u,v};}
        if(newx==oldx)continue;
        uint64_t changed=(1ULL<<x)|(y>=0?1ULL<<y:0),edge_bits=0;
        auto add_edge=[&](pair<int,int>e){edge_bits|=(1ULL<<(8*e.first+e.second))|(1ULL<<(8*e.second+e.first));};
        add_edge(oldx);add_edge(newx);if(y>=0){add_edge(oldy);add_edge(newy);}
        assign(x,oldx,false);assign(x,newx,true);edges[x]=newx;
        if(y>=0){assign(y,oldy,false);assign(y,newy,true);edges[y]=newy;}
        int affected[8],nc=0;uint64_t bb=edge_bits;while(bb){affected[nc++]=__builtin_ctzll(bb);bb&=bb-1;}
        int next_count=feasible;
        for(int i=0;i<n;i++) {
            if((i&255)==0 && expired()){interrupted=true;break;}
            const auto&t=targets[i];uint64_t g=graphs[i];
            if((t.zero|t.one)&changed) {
                g&=~edge_bits;
                for(int k=0;k<nc;k++){int e=affected[k];if((t.zero&zeros[e])||(t.one&ones[e]))g|=1ULL<<e;}
                next_valid[i]=acyclic(g);
            }else next_valid[i]=valid[i];
            next_graphs[i]=g;next_count+=int(next_valid[i])-int(valid[i]);
#ifdef NOV8_FLEX_VERIFY
            assert(g==render(t));uint16_t before[16];
            assert(bool(next_valid[i])==comparison_pair_dag(A[t.a],A[t.b],B,W,edges,before,0));
#endif
        }
        if(interrupted)break;
        auto candidate=evaluate(next_graphs,next_valid,next_count);
        if(!candidate.complete){interrupted=true;break;}
        done++;
        double temp=2.0*pow(.01,double(it)/max(1,iterations));
        bool take=candidate.score>=current.score || (rnd()+.5)/4294967296.0<exp((candidate.score-current.score)/temp);
        if(take) {
            accepted++;graphs.swap(next_graphs);valid.swap(next_valid);feasible=next_count;current=candidate;
            if(current.score>best.score){best=current;best_edges=edges;best_feasible=feasible;}
        }else {
            assign(x,newx,false);assign(x,oldx,true);edges[x]=oldx;
            if(y>=0){assign(y,newy,false);assign(y,oldy,true);edges[y]=oldy;}
        }
    }
    // Partial proposals have modified only local scratch data and edges.
    // Publishing the best complete snapshot also rolls back a timed-out one.
    edges=best_edges;
#ifdef NOV8_FLEX_VERIFY
    for(int u=0;u<B;u++)for(int v=u+1;v<B;v++) {
        bool present=false;for(auto e:edges)present|=(e==pair<int,int>{u,v}||e==pair<int,int>{v,u});assert(present);
    }
#endif
    cerr<<"NOV8_FLEX offset="<<offset<<" done="<<done<<" accepted="<<accepted
        <<" matched="<<initial.matched<<"->"<<best.matched<<" cost="<<initial.cost<<"->"<<best.cost
        <<" feasible="<<initial_feasible<<"->"<<best_feasible<<" metric_cache="<<cache.size()
        <<" interrupted="<<interrupted<<" cpu="<<double(clock()-started)/CLOCKS_PER_SEC
        <<" local_cpu="<<double(clock()-started)/CLOCKS_PER_SEC/search_budget::scale()<<"\n";
}

inline vector<pair<int,int>> make_data_edges(const vector<uint64_t>& A,int B,int W,int offset,
                                            clock_t absolute_deadline=0) {
    int M=B*(B-1)/2,T=64-W;
    auto cheap_complete_edges=[&]() {
        auto e=make_comparison_edges(A[0]^(0x632be59bd9b4e019ULL*offset),B,W,true);
        balance_comparison_polarities(e,B);return e;
    };
    if(construction_deadline_expired(absolute_deadline))return cheap_complete_edges();
    if(W<M || W>2*M) {
        auto e=make_comparison_edges(A[0]^(0x632be59bd9b4e019ULL*offset),B,W,true);
        balance_comparison_polarities(e,B);return e;
    }
    int need=W-M;
    vector<vector<int>> cost(W,vector<int>(W)),same_cost=cost;
    vector<tuple<int,int,int>> candidates;
    for(int x=0;x<W;x++) {
    if(construction_deadline_expired(absolute_deadline))return cheap_complete_edges();
    for(int y=x+1;y<W;y++) {
        int count[4]={};
        for(uint64_t a:A)count[((a>>(T+x)&1)<<1)|(a>>(T+y)&1)]++;
        int bad_same=count[1]*count[2],bad_opposite=count[0]*count[3];
        cost[x][y]=cost[y][x]=min(bad_same,bad_opposite);
        same_cost[x][y]=same_cost[y][x]=bad_same<=bad_opposite;
        candidates.emplace_back(cost[x][y],x,y);
    }
    }
    sort(candidates.begin(),candidates.end());
    vector<int> mate(W,-1);int paired=0;
    for(auto [c,u,v]:candidates)if(mate[u]<0 && mate[v]<0 && paired<need){mate[u]=v;mate[v]=u;paired++;}
    assert(paired==need);
    int score=0;for(int u=0;u<W;u++)if(mate[u]>u)score+=cost[u][mate[u]];
    int initial=score,best=score;auto best_mate=mate;
    uint64_t state=A[0]^(0x9e3779b97f4a7c15ULL*(offset+1));if(!state)state=1;
    auto rnd=[&](){state^=state<<7;state^=state>>9;return uint32_t(state);};
    int iterations=30000;
    for(int it=0;it<iterations;it++) {
        if((it&255)==0 && construction_deadline_expired(absolute_deadline))break;
        int u=rnd()%W,v=mate[u];if(v<0)continue;
        int x=rnd()%W;if(x==u || x==v)continue;int y=mate[x];
        int delta=cost[u][x]-cost[u][v];if(y>=0)delta+=cost[v][y]-cost[x][y];
        double temp=250.0*pow(.01,double(it)/iterations);
        if(delta>0 && (rnd()+.5)/4294967296.0>=exp(-delta/temp))continue;
        mate[u]=x;mate[x]=u;mate[v]=y;if(y>=0)mate[y]=v;score+=delta;
        if(score<best){best=score;best_mate=mate;}
    }
    mate=best_mate;
    struct Group {int x,y,u,v,weight,sign;};
    vector<pair<int,int>> edges;
    for(int u=0;u<B;u++)for(int v=u+1;v<B;v++)edges.push_back({u,v});
    for(int i=M-1;i>0;i--)swap(edges[i],edges[rnd()%(i+1)]);
    vector<Group> groups;
    for(int x=0;x<W;x++)if(mate[x]>x || mate[x]<0) {
        int y=mate[x];auto [u,v]=edges[groups.size()];
        int weight=y<0?1:(same_cost[x][y]?2:0);
        groups.push_back({x,y,u,v,weight,int(rnd()&1)?1:-1});
    }
    assert((int)groups.size()==M);
    vector<int> balance(B);
    for(auto& g:groups){balance[g.u]-=g.sign*g.weight;balance[g.v]+=g.sign*g.weight;}
    auto objective=[&](){int x=0;for(int d:balance)x+=d*d;return x;};
    int energy=objective(),best_energy=energy;auto best_groups=groups;
    bool do_balance=!getenv("DATA_BALANCE") || atoi(getenv("DATA_BALANCE"));
    for(int it=0;it<10000 && do_balance;it++) {
        if((it&255)==0 && construction_deadline_expired(absolute_deadline))break;
        int k=rnd()%M;auto &g=groups[k];
        int delta=2*g.sign*g.weight;
        balance[g.u]+=delta;balance[g.v]-=delta;
        int next=objective();double temp=8.0*pow(.01,double(it)/10000);
        if(next>energy && (rnd()+.5)/4294967296.0>=exp((energy-next)/temp)){balance[g.u]-=delta;balance[g.v]+=delta;continue;}
        g.sign=-g.sign;energy=next;
        if(energy<best_energy){best_energy=energy;best_groups=groups;}
    }
    groups=best_groups;
    // Keep column groups and their relative polarity; optimize only their K_B edge map.
    if(B<=7 && (!getenv("MAP_ENABLE") || atoi(getenv("MAP_ENABLE")))) {
        const clock_t map_started=clock();
        const int map_iters=getenv("MAP_ITERS")?atoi(getenv("MAP_ITERS")):512;
        const double map_cap=getenv("MAP_CAP")?atof(getenv("MAP_CAP")):.09;
        const int map_mode=getenv("MAP_MODE")?atoi(getenv("MAP_MODE")):1;
        const double balance_weight=getenv("MAP_BALANCE_WEIGHT")?atof(getenv("MAP_BALANCE_WEIGHT")):.05;
        const clock_t local_map_deadline=map_started+clock_t(search_budget::seconds(max(0.0,map_cap))*CLOCKS_PER_SEC);
        const clock_t map_deadline=absolute_deadline>0?min(absolute_deadline,local_map_deadline):local_map_deadline;
        auto map_expired=[&](){return clock()>=map_deadline;};
        auto log_map_abort=[&](const char* stage) {
            cerr<<"MAP_ABORT W="<<W<<" stage="<<stage<<" cpu="<<double(clock()-map_started)/CLOCKS_PER_SEC<<"\n";
        };
        // The lambda may discard incomplete evaluation without discarding the legal map.
        auto improve_map=[&]() {
        if(map_iters<=0 || map_expired()){log_map_abort("entry");return;}
        struct Target {int a,b,weight;uint32_t forward,reverse;};
        vector<Target> target;
        uint64_t prefix=(1ULL<<T)-1;
        for(int a=0;a<(int)A.size();a++) {
        if(map_expired()){log_map_abort("targets");return;}
        for(int b=a+1;b<(int)A.size();b++) {
            uint64_t diff=A[a]^A[b];if(!(~diff&prefix))continue;
            uint32_t forward=0,reverse=0;bool possible=true;
            for(int i=0;i<M;i++) {
                const auto &g=groups[i];int need=0;
                if(diff>>(T+g.x)&1)need|=(A[a]>>(T+g.x)&1)?2:1;
                if(g.y>=0 && (diff>>(T+g.y)&1)) {
                    bool bit=(A[a]>>(T+g.y)&1)^(!same_cost[g.x][g.y]);need|=bit?2:1;
                }
                if(need==3){possible=false;break;}
                if(need==1)forward|=1u<<i;if(need==2)reverse|=1u<<i;
            }
            if(!possible)continue;
            int hd=__builtin_popcountll(diff&prefix),near=max(1,T/2+2-hd);
            target.push_back({a,b,near*near,forward,reverse});
        }
        }
        const int e=target.size();
        array<uint64_t,128> rows{};
        for(int mask=0;mask<(1<<B);mask++)for(int v=0;v<B;v++)if(mask>>v&1)rows[mask]|=uint64_t((1<<B)-1)<<(8*v);
        auto acyclic=[&](uint64_t g) {
            unsigned alive=(1u<<B)-1;
            while(g) {
                uint64_t incoming=g;incoming|=incoming>>32;incoming|=incoming>>16;incoming|=incoming>>8;
                unsigned zero=alive&~unsigned(incoming&255);
                if(!zero)return false;
                g&=~rows[zero];alive&=~zero;
            }
            return true;
        };
        // Same source-removal test, additionally retaining one concrete
        // directed cycle when infeasible. The residual graph at failure has
        // positive indegree at every remaining vertex, so predecessor walking
        // necessarily repeats a vertex within B steps.
        auto analyze_cycle=[&](uint64_t g,uint64_t& cycle) {
            cycle=0;unsigned alive=(1u<<B)-1;
            while(g) {
                uint64_t incoming=g;incoming|=incoming>>32;incoming|=incoming>>16;incoming|=incoming>>8;
                unsigned zero=alive&~unsigned(incoming&255);
                if(!zero) {
                    int position[8];fill(position,position+8,-1);
                    uint64_t path[8];int depth=0,vertex=__builtin_ctz(alive);
                    while(position[vertex]<0) {
                        position[vertex]=depth;
                        int predecessor=0;
                        while(predecessor<B && !(g&(1ULL<<(8*predecessor+vertex))))predecessor++;
                        assert(predecessor<B && depth<B);
                        path[depth++]=1ULL<<(8*predecessor+vertex);
                        vertex=predecessor;
                    }
                    for(int i=position[vertex];i<depth;i++)cycle|=path[i];
                    assert(cycle && (cycle&g)==cycle);
                    return false;
                }
                g&=~rows[zero];alive&=~zero;
            }
            return true;
        };

        auto direction=[&](int i,bool reverse) {
            const auto &g=groups[i];int u=g.u,v=g.v;
            if((g.sign<0)^reverse)swap(u,v);
            return 1ULL<<(8*u+v);
        };
        auto group_edges=[&](int i){return direction(i,false)|direction(i,true);};
        auto render_target=[&](const Target &t) {
            uint64_t g=0;uint32_t bits=t.forward;
            while(bits){int i=__builtin_ctz(bits);bits&=bits-1;g|=direction(i,false);}
            bits=t.reverse;while(bits){int i=__builtin_ctz(bits);bits&=bits-1;g|=direction(i,true);}
            return g;
        };
        auto balance_cost=[&](){vector<int> d(B);for(auto&g:groups){d[g.u]-=g.sign*g.weight;d[g.v]+=g.sign*g.weight;}int x=0;for(int q:d)x+=q*q;return x;};
        vector<uint64_t> graph(e),next_graph(e),cycle(e),next_cycle(e);vector<uint8_t> feasible(e),next_feasible(e);
        size_t cycle_reuses=0,cycle_retests=0,cycle_builds=0;
        vector<int> degree(A.size()),next_degree(A.size());int weighted=0,feasible_count=0;
        for(int k=0;k<e;k++) {
            if((k&255)==0 && map_expired()){log_map_abort("initial_graphs");return;}
            graph[k]=render_target(target[k]);feasible[k]=analyze_cycle(graph[k],cycle[k]);cycle_retests++;cycle_builds++;
#ifdef MAP_FAST_VERIFY
            assert(bool(feasible[k])==acyclic(graph[k]));
            assert(bool(feasible[k])==(cycle[k]==0));
#endif
            if(feasible[k]){auto&t=target[k];weighted+=t.weight;degree[t.a]+=t.weight;degree[t.b]+=t.weight;feasible_count++;}
        }
        auto objective_map=[&](const vector<int>& d,int weight,int bal) {
            double score=map_mode==0?weight:0.;if(map_mode==1)for(int q:d)score+=sqrt(1.0+q);
            return score-balance_weight*bal;
        };
        double score=objective_map(degree,weighted,balance_cost()),first_score=score,best_score=score;
        const int first_feasible=feasible_count;int best_feasible=feasible_count,first_weight=weighted,best_weight=weighted;
        auto map_best=groups;int done=0,accepted=0;
        for(int it=0;it<map_iters;it++) {
            if(map_expired())break;
            done++;
            int x=rnd()%M,y=-1;auto oldx=groups[x],oldy=oldx;
            uint64_t clear=group_edges(x);uint32_t changed=1u<<x;
            if(rnd()%2) {
                y=rnd()%M;if(y==x)continue;oldy=groups[y];clear|=group_edges(y);changed|=1u<<y;
                swap(groups[x].u,groups[y].u);swap(groups[x].v,groups[y].v);
            }else groups[x].sign=-groups[x].sign;
            uint64_t xf=direction(x,false),xr=direction(x,true),yf=0,yr=0;
            if(y>=0){yf=direction(y,false);yr=direction(y,true);}
            next_degree=degree;int next_weight=weighted,next_count=feasible_count;
            for(int k=0;k<e;k++) {
                if((k&255)==0 && map_expired()) {
                    // x/y may already have changed. Restore the entire last complete best map.
                    groups=map_best;log_map_abort("iteration");return;
                }
                const auto&t=target[k];
                if(!((t.forward|t.reverse)&changed)){
                    next_graph[k]=graph[k];next_feasible[k]=feasible[k];next_cycle[k]=cycle[k];continue;
                }
                uint64_t g=graph[k]&~clear;
                if(t.forward>>x&1)g|=xf;else if(t.reverse>>x&1)g|=xr;
                if(y>=0){if(t.forward>>y&1)g|=yf;else if(t.reverse>>y&1)g|=yr;}
                next_graph[k]=g;
                const uint64_t witness=cycle[k];
                if(witness && (g&witness)==witness) {
                    next_feasible[k]=0;next_cycle[k]=witness;cycle_reuses++;
                }else {
                    // Most proposals are rejected. Record exact feasibility
                    // now and defer certificate extraction until acceptance.
                    next_feasible[k]=acyclic(g);next_cycle[k]=0;cycle_retests++;
                }
#ifdef MAP_FAST_VERIFY
                assert(bool(next_feasible[k])==acyclic(g));
                // A rejected proposal need not own a cycle certificate.
                assert(!next_feasible[k] || next_cycle[k]==0);
                assert((g&next_cycle[k])==next_cycle[k]);
#endif
                int sign=int(next_feasible[k])-int(feasible[k]);
                if(sign){int add=sign*t.weight;next_weight+=add;next_count+=sign;next_degree[t.a]+=add;next_degree[t.b]+=add;}
            }
            double next_score=objective_map(next_degree,next_weight,balance_cost());
            double temp=(map_mode==0?25.0:2.0)*pow(.01,double(it)/max(1,map_iters));
            if(next_score<score && (rnd()+.5)/4294967296.0>=exp((next_score-score)/temp)) {
                groups[x]=oldx;if(y>=0)groups[y]=oldy;continue;
            }
            accepted++;graph.swap(next_graph);feasible.swap(next_feasible);degree.swap(next_degree);cycle.swap(next_cycle);
            // Only accepted states pay for finding a new directed cycle.
            // Reused certificates already belong to the accepted graph.
            for(int k=0;k<e;k++)if(!feasible[k] && cycle[k]==0) {
                const bool is_dag=analyze_cycle(graph[k],cycle[k]);
                assert(!is_dag && cycle[k] && (graph[k]&cycle[k])==cycle[k]);
                (void)is_dag;cycle_builds++;
            }
            weighted=next_weight;feasible_count=next_count;score=next_score;
            if(score>best_score){best_score=score;map_best=groups;best_feasible=feasible_count;best_weight=weighted;}
        }
        groups=map_best;
        cerr<<"MAP_FAST targets="<<e<<" cycle_reuses="<<cycle_reuses<<" cycle_retests="<<cycle_retests
            <<" cycle_builds="<<cycle_builds<<" lazy=1\n";
        cerr<<"MAP W="<<W<<" targets="<<e<<" mode="<<map_mode<<" score="<<first_score<<"->"<<best_score<<" feasible="<<first_feasible<<"->"<<best_feasible<<" weighted="<<first_weight<<"->"<<best_weight<<" iterations="<<done<<" accepted="<<accepted<<" cpu="<<double(clock()-map_started)/CLOCKS_PER_SEC<<"\n";
        };
        improve_map();
    }

    vector<pair<int,int>> result(W);
    for(auto g:groups) {
        int u=g.u,v=g.v;if(g.sign<0)swap(u,v);
        result[g.x]={u,v};
        if(g.y>=0)result[g.y]=same_cost[g.x][g.y]?pair<int,int>{u,v}:pair<int,int>{v,u};
    }
    cerr<<"DATA_EDGES W="<<W<<" pair_cost="<<best<<" initial="<<initial<<" balance="<<best_energy<<"\n";
    return result;
}

// Select the duplicated-edge columns from all 64 windows. A returned entry j
// is the original window displayed at internal coordinate j.
inline vector<int> choose_global_columns(const vector<uint64_t>& catalog,int B,int W,int offset,
                                         clock_t absolute_deadline=0) {
    vector<int> identity(64);for(int j=0;j<64;j++)identity[j]=j;
    if(construction_deadline_expired(absolute_deadline))return identity;
    int M=B*(B-1)/2,need=W-M;
    if(need<0 || W>2*M || 2*need>64 || W>64)return identity;
    int cost[64][64]={};
    vector<tuple<int,int,int>> candidates;
    for(int x=0;x<64;x++) {
    if(construction_deadline_expired(absolute_deadline))return identity;
    for(int y=x+1;y<64;y++) {
        int count[4]={};
        for(uint64_t a:catalog)count[((a>>x&1)<<1)|(a>>y&1)]++;
        cost[x][y]=cost[y][x]=min(count[1]*count[2],count[0]*count[3]);
        candidates.emplace_back(cost[x][y],x,y);
    }
    }
    sort(candidates.begin(),candidates.end());
    vector<int> mate(64,-1);int paired=0;
    for(auto [value,u,v]:candidates)if(mate[u]<0 && mate[v]<0 && paired<need) {
        mate[u]=v;mate[v]=u;paired++;
    }
    assert(paired==need);
    int score=0;for(int u=0;u<64;u++)if(mate[u]>u)score+=cost[u][mate[u]];
    int initial=score,best=score;auto best_mate=mate;
    uint64_t state=catalog[0]^(0xa0761d6478bd642fULL*(offset+1));if(!state)state=1;
    auto rnd=[&](){state^=state<<7;state^=state>>9;return uint32_t(state);};
    int iterations=getenv("GLOBAL_COLUMN_ITERS")?atoi(getenv("GLOBAL_COLUMN_ITERS")):150000;
    for(int it=0;it<iterations;it++) {
        if((it&255)==0 && construction_deadline_expired(absolute_deadline))break;
        int u=rnd()%64,v=mate[u];if(v<0)continue;
        int x=rnd()%64;if(x==u || x==v)continue;int y=mate[x];
        int delta=cost[u][x]-cost[u][v];if(y>=0)delta+=cost[v][y]-cost[x][y];
        double temp=250.0*pow(.01,double(it)/iterations);
        if(delta>0 && (rnd()+.5)/4294967296.0>=exp(-delta/temp))continue;
        mate[u]=x;mate[x]=u;mate[v]=y;if(y>=0)mate[y]=v;score+=delta;
        if(score<best){best=score;best_mate=mate;}
    }
    mate=best_mate;
    vector<int> unused;vector<bool> suffix(64,false);
    for(int j=0;j<64;j++)if(mate[j]>=0)suffix[j]=true;else unused.push_back(j);
    for(int i=int(unused.size())-1;i>0;i--)swap(unused[i],unused[rnd()%(i+1)]);
    int singles=W-2*need;
    assert(singles>=0 && singles<=int(unused.size()));
    for(int i=0;i<singles;i++)suffix[unused[i]]=true;
    vector<int> order;
    for(int j=0;j<64;j++)if(!suffix[j])order.push_back(j);
    assert(int(order.size())==64-W);
    for(int j=0;j<64;j++)if(suffix[j])order.push_back(j);
    assert(int(order.size())==64);
    cerr<<"GLOBAL_COLUMNS W="<<W<<" pairs="<<need<<" singles="<<singles
        <<" initial="<<initial<<" best="<<best<<"\n";
    return order;
}

inline vector<uint64_t> permute_catalog_columns(const vector<uint64_t>& catalog,const vector<int>& order) {
    vector<uint64_t> answer(catalog.size(),0);
    for(int a=0;a<int(catalog.size());a++)for(int j=0;j<64;j++)
        answer[a]|=((catalog[a]>>order[j])&1ULL)<<j;
    return answer;
}

static array<int,64> catalogue_column_penalty(const vector<uint64_t>& products){
    array<int,64> bias{};
    constexpr int price=3;
    for(int w=0;w<64;w++){
        int count=0;for(uint64_t a:products)count+=(a>>w)&1;
        bias[w]=price*abs(count-int(products.size())/2);
    }
    return bias;
}

// Optimize the split between prefix and suffix on the disjoint pairs that
// will actually share main sheets. A trial exchanges one prefix window
// with one window of one backing comparison. Only this edge can change.
template<int B>
static void refine_window_split(const vector<uint64_t>& catalogue,vector<int>& order,
                                const vector<pair<int,int>>& layout,int W,int offset,clock_t deadline,int iters=1024){
    const int T = 64 - W;
    const auto colbias=catalogue_column_penalty(catalogue);
    constexpr int needed = P - (K - B);
    const double cap = search_budget::seconds(0.14);
    const clock_t start=clock(),stop=min(deadline,start+clock_t(cap*CLOCKS_PER_SEC));
    if(clock()>=stop)return;
    constexpr int limit = 28;  // Hamming-distance screen for the partition search only.
    struct Edge{uint64_t pos=0,neg=0,xy=0,yx=0;};
    vector<Edge> es;int eid[B][B];for(auto&row:eid)fill(row,row+B,-1);
    vector<int> index(W);
    for(int j=0;j<W;j++) {
        auto [u,v]=layout[j]; bool forward=u<v;if(!forward)swap(u,v);
        if(eid[u][v]<0){eid[u][v]=es.size();es.push_back({0,0,1ULL<<(8*u+v),1ULL<<(8*v+u)});}
        auto& e=es[index[j]=eid[u][v]];(forward?e.pos:e.neg)|=1ULL<<order[T+j];
    }
    uint64_t pfx=0;for(int j=0;j<T;j++)pfx|=1ULL<<order[j];
    struct Pair{uint64_t z,o;unsigned char a,b;};
    vector<Pair> pq;
    for(int a=0;a<P;a++)for(int b=a+1;b<P;b++){
        uint64_t d=catalogue[a]^catalogue[b];if(__builtin_popcountll(d)>limit)continue;
        pq.push_back({d&~catalogue[a],d&catalogue[a],(unsigned char)a,(unsigned char)b});
    }
    auto directions=[](const Pair& p,const Edge& e){
        uint64_t g=0;
        if((p.z&e.pos)||(p.o&e.neg))g|=e.xy;
        if((p.o&e.pos)||(p.z&e.neg))g|=e.yx;
        return g;
    };
    const int n=pq.size();vector<uint64_t> graph(n);
    for(int i=0;i<n;i++)for(auto& e:es)graph[i]|=directions(pq[i],e);
    uint64_t rows[1<<B]={};for(int mask=1;mask<(1<<B);mask++)rows[mask]=rows[mask&(mask-1)]|(((1ULL<<B)-1)<<(8*__builtin_ctz(mask)));
#ifdef SPLIT_CERT_VERIFY
    auto acyclic=[&](uint64_t g){
        unsigned alive=(1u<<B)-1;
        while(g){uint64_t in=g;in|=in>>32;in|=in>>16;in|=in>>8;
            unsigned free=alive&~unsigned(in&255);if(!free)return false;
            g&=~rows[free];alive&=~free;}
        return true;
    };
#endif
    static constexpr uint64_t column=(0x0101010101010101ULL>>(8*(8-B)));
    auto closure=[](uint64_t g){
        for(int k=0;k<B;k++)g|=(((g>>k)&column)*255)&(((g>>(8*k))&((1<<B)-1))*column);
        return g;
    };
    int costs[1+B*(B-1)/2];for(int c=0;c<=B*(B-1)/2;c++)costs[c]=round(4*16*.72*sqrt(double(c)));
    const int ne=es.size(),words=(n+63)/64;
    const unsigned all=(1u<<ne)-1;
    // Bit i describes a catalogue pair. Replacing a window changes only
    // two possible directions of one comparison for each affected pair.
    vector<vector<uint64_t>> zero(64,vector<uint64_t>(words)),one(64,vector<uint64_t>(words));
    for(int i=0;i<n;i++){
        uint64_t b=uint64_t(1)<<(i&63);
        for(uint64_t z=pq[i].z;z;z&=z-1)zero[__builtin_ctzll(z)][i>>6]|=b;
        for(uint64_t z=pq[i].o;z;z&=z-1)one[__builtin_ctzll(z)][i>>6]|=b;
    }
    auto orientations=[&](const Edge& e,vector<uint64_t>& up,vector<uint64_t>& down){
        fill(up.begin(),up.end(),0);fill(down.begin(),down.end(),0);
        for(int flip=0;flip<2;flip++)for(uint64_t z=flip?e.neg:e.pos;z;z&=z-1){
            int j=__builtin_ctzll(z);
            const auto& a=flip?one[j]:zero[j];const auto& b=flip?zero[j]:one[j];
            for(int h=0;h<words;h++){up[h]|=a[h];down[h]|=b[h];}
        }
    };
    vector<vector<uint64_t>> ups(ne,vector<uint64_t>(words)),downs=ups;
    for(int h=0;h<ne;h++)orientations(es[h],ups[h],downs[h]);
    vector<uint64_t> up(words),down(words),change(words);
    // A feasible pair participates in every single-edge trial. For an
    // infeasible pair, remember one cycle and revisit it only when the trial
    // can remove an edge of that cycle.
    vector<vector<uint64_t>> active(ne,vector<uint64_t>(words));
    vector<unsigned> membership(n),next_membership(n);
    vector<int8_t> dag(n),next_dag(n);
    auto classify=[&](uint64_t g,unsigned &which){
        // Transpose the byte-strided 7x7 graph to catch 2-cycles directly.
        uint64_t x=g,t=(x^(x>>7))&0x00aa00aa00aa00aaULL;x^=t^(t<<7);
        t=(x^(x>>14))&0x0000cccc0000ccccULL;x^=t^(t<<14);
        t=(x^(x>>28))&0x00000000f0f0f0f0ULL;x^=t^(t<<28);
        uint64_t mutual=x&g;
        if(mutual){
            int b=__builtin_ctzll(mutual),u=b>>3,v=b&7;
            which=1u<<eid[min(u,v)][max(u,v)];return -1;
        }
        uint64_t residual=g; unsigned alive=(1u<<B)-1;
        while(residual){
            uint64_t in=residual;in|=in>>32;in|=in>>16;in|=in>>8;
            unsigned free=alive&~unsigned(in&255);
            if(free){residual&=~rows[free];alive&=~free;continue;}
            // All residual vertices have predecessors. Following those
            // predecessors until one repeats recovers a directed cycle.
            int v=__builtin_ctz(alive),k=0;int positions[B];fill(positions,positions+B,-1);
            unsigned path[B];
            while(positions[v]<0){
                positions[v]=k;
                int u=__builtin_ctzll(residual&(column<<v))>>3;
                path[k++]=1u<<eid[min(u,v)][max(u,v)];v=u;
            }
            which=0;for(int h=positions[v];h<k;h++)which|=path[h];
            return -1;
        }
        which=all;return __builtin_popcountll(closure(g));
    };
    // 16*hd plus the route term is at most 16*64+211. Append candidates
    // in index order so ties match sorting by (cost, pair index).
    constexpr int key_words=20;
    vector<int> links(n);array<int,64*key_words> head,tail;array<uint64_t,key_words> present{};
    auto add=[&](int i,int k,uint64_t pref){
        if(k<0)return;
        int c=16*__builtin_popcountll((pq[i].z|pq[i].o)&pref)+costs[k];
        uint64_t mask=1ULL<<(c&63);auto& seen=present[c>>6];
        if(seen&mask)links[tail[c]]=i;else{head[c]=i;seen|=mask;}
        tail[c]=i;links[i]=-1;
    };
    auto score_edges=[&](){
        int selected=0,cost=0;bool used[P]={};
        for(int b=0;b<int(present.size());b++)for(uint64_t f=present[b];f;f&=f-1){
            int c=64*b+__builtin_ctzll(f);
            for(int i=head[c];i>=0;i=links[i]){
                const auto&p=pq[i];if(used[p.a]||used[p.b])continue;
                cost+=c;used[p.a]=used[p.b]=true;if(++selected==needed)return cost;
            }
        }
        return cost+(needed-selected)*1024;
    };
    auto set_members=[&](int i,unsigned diff){
        while(diff){int e=__builtin_ctz(diff);diff&=diff-1;active[e][i>>6]^=1ULL<<(i&63);}
    };
    // Two distinct mutual edges are two independent cycles. Changing one
    // comparison cannot repair both. Re-enable a pair when an accepted move
    // reduces their count to one.
    vector<unsigned> mutual_edges(n);
    for(int h=0;h<ne;h++)for(int z=0;z<words;z++)
        for(uint64_t bits=ups[h][z]&downs[h][z];bits;bits&=bits-1)
            mutual_edges[64*z+__builtin_ctzll(bits)]|=1u<<h;
    auto multiple=[](unsigned m){return bool(m&(m-1));};
    for(int i=0;i<n;i++){
        if(multiple(mutual_edges[i])){dag[i]=-1;membership[i]=0;continue;}
        dag[i]=classify(graph[i],membership[i]);set_members(i,membership[i]);add(i,dag[i],pfx);
    }
    int cur_bias=0; for(int w=T;w<64;w++)cur_bias+=colbias[order[w]];
    int score=score_edges()+cur_bias,best=score,init=score,done=0;
    auto best_order=order;
    uint64_t state=catalogue[0]^(0x69d2bc9e8af73e5dULL*(offset+1));
    auto rnd=[&](){state^=state<<7;state^=state>>9;return unsigned(state);};
    vector<int> modified;modified.reserve(n);
    for(int it=0;it<iters && clock()<stop;it++){
        int x=rnd()%T,y=rnd()%W,id=index[y];uint64_t sx=1ULL<<order[x],sy=1ULL<<order[T+y],sw=sx|sy;
        auto& e=es[id];auto old=e;
        (e.pos&sy?e.pos:e.neg)^=sw;
        uint64_t clear=~(e.xy|e.yx),pref=pfx^sw;
        orientations(e,up,down);
        for(int z=0;z<words;z++)change[z]=(up[z]^ups[id][z])|(down[z]^downs[id][z]);
        present.fill(0);modified.clear();
        for(int z=0;z<words;z++)for(uint64_t bits=active[id][z];bits;bits&=bits-1){
            int i=64*z+__builtin_ctzll(bits);
            uint64_t b=bits&-bits;int k=dag[i];
            if(change[z]&b){
                uint64_t g=(graph[i]&clear)|((up[z]&b)?e.xy:0)|((down[z]&b)?e.yx:0);
                modified.push_back(i);next_dag[i]=k=classify(g,next_membership[i]);
            }
            add(i,k,pref);
        }
        int db=colbias[order[x]]-colbias[order[T+y]];int ns=score_edges()+cur_bias+db;done++;
#ifdef SPLIT_CERT_VERIFY
        vector<pair<int,int>> reference;
        for(int i=0;i<n;i++){
            uint64_t exact=0;for(auto&edge:es)exact|=directions(pq[i],edge);
            if(!acyclic(exact))continue;
            int c=16*__builtin_popcountll((pq[i].z|pq[i].o)&pref)+costs[__builtin_popcountll(closure(exact))];
            reference.emplace_back(c,i);
        }
        sort(reference.begin(),reference.end());
        int used_count=0,total=0;bool taken[P]={};
        for(auto edge:reference){
            auto p=pq[edge.second];if(taken[p.a]||taken[p.b])continue;
            taken[p.a]=taken[p.b]=true;total+=edge.first;if(++used_count==needed)break;
        }
        assert(ns==total+(needed-used_count)*1024+cur_bias+db);
#endif
        double t=36*pow(.03,double(it)/iters);
        if(ns>score && (rnd()+.5)/4294967296.0>=exp((score-ns)/t)){e=old;continue;}
        // Even a skipped pair may have a changed graph. Its persistent
        // witness still proves infeasibility, so only its graph needs updating.
        for(int z=0;z<words;z++)for(uint64_t bits=change[z];bits;bits&=bits-1){
            int i=64*z+__builtin_ctzll(bits);uint64_t b=bits&-bits;
            graph[i]=(graph[i]&clear)|((up[z]&b)?e.xy:0)|((down[z]&b)?e.yx:0);
            unsigned old_edges=mutual_edges[i];
            unsigned remaining=old_edges&~(1u<<id);
            mutual_edges[i]=remaining|((up[z]&down[z]&b)?(1u<<id):0);
            if(multiple(old_edges) && !multiple(mutual_edges[i])){
                membership[i]=mutual_edges[i];set_members(i,membership[i]);
            }else if(!multiple(old_edges) && multiple(mutual_edges[i]) && !(active[id][z]&b)){
                set_members(i,membership[i]);membership[i]=0;
            }
        }
        up.swap(ups[id]);down.swap(downs[id]);
        for(int i:modified){
            unsigned next=multiple(mutual_edges[i])?0:next_membership[i];
            set_members(i,membership[i]^next);membership[i]=next;dag[i]=next_dag[i];
        }
        std::swap(order[x],order[T+y]);pfx=pref;score=ns;cur_bias+=db;
        if(score<best){best=score;best_order=order;}
    }
    order=move(best_order);
    cerr<<"SPLIT before="<<init<<" after="<<best<<" it="<<done<<" cpu="<<double(clock()-start)/CLOCKS_PER_SEC<<"\n";
}


// Eject one duplicate comparison column. Its removal leaves one DAG per
// candidate product pair. Reachability in that DAG decides feasibility for
// every possible replacement direction, independently of the chosen column.
template<int B>
static void fit_comparison_columns(const vector<uint64_t>& catalogue,vector<int>& order,
        vector<pair<int,int>>& layout,int W,int offset,clock_t deadline,int sweeps=2){
    constexpr int M=B*(B-1)/2, D=2*M;
    int T=64-W;
    const auto colbias=catalogue_column_penalty(catalogue);
    constexpr int screens = 3, hdmax = 28;
    const clock_t started=clock(),stop=min(deadline,started+clock_t(search_budget::seconds(.11)*CLOCKS_PER_SEC));
    if(clock()>=stop)return;
    int e2[B][B];pair<int,int> ends[M];int na=0;
    for(int a=0;a<B;a++)for(int b=a+1;b<B;b++){ends[na]={a,b};e2[a][b]=e2[b][a]=na++;}
    uint64_t dirbits[D]={},cols[D]={},pref=0;
    for(int a=0;a<M;a++){auto[u,v]=ends[a];dirbits[2*a]=1ULL<<(8*u+v);dirbits[2*a+1]=1ULL<<(8*v+u);}
    for(int j=0;j<T;j++)pref|=1ULL<<order[j];
    vector<int> code(W);
    for(int j=0;j<W;j++){auto[u,v]=layout[j];code[j]=2*e2[u][v]+int(u>v);cols[code[j]]|=1ULL<<order[T+j];}
    struct Raw {uint64_t d,white,graph;int a,b;};
    vector<Raw> pairs;
    constexpr int prefix_filter = 18;
    auto render=[](const Raw&p,uint64_t c0,uint64_t c1,int e,const uint64_t* bits){
        const uint64_t black=p.d^p.white;uint64_t g=0;
        if((p.white&c0)||(black&c1))g|=bits[2*e];
        if((p.white&c1)||(black&c0))g|=bits[2*e+1];
        return g;
    };
    for(int a=0;a<P;a++)for(int b=a+1;b<P;b++){
        uint64_t d=catalogue[a]^catalogue[b];if(__builtin_popcountll(d)>hdmax||__builtin_popcountll(d&pref)>prefix_filter)continue;
        Raw p{d,d&~catalogue[a],0,a,b};
        for(int e=0;e<M;e++)p.graph|=render(p,cols[2*e],cols[2*e+1],e,dirbits);
        pairs.push_back(p);
    }
    constexpr uint64_t one=(0x0101010101010101ULL>>(8*(8-B))),diag=0x40201008040201ULL;
    auto closure=[](uint64_t g){
        for(int v=0;v<B;v++)g|=(((g>>v)&one)*255)&(((g>>(8*v))&((1<<B)-1))*one);
        return g;
    };
    constexpr double route_w = 4.0;
    int rc[M+1];for(int i=0;i<=M;i++)rc[i]=round(route_w*16*.72*sqrt(double(i)));
    struct Option{int index,basecost,hd;unsigned char edgecost[D];};
    vector<Option> opts;opts.reserve(1200);
    vector<int> ys(W),xlist,links(pairs.size()),hc(pairs.size()),side(pairs.size());
    for(int j=0;j<W;j++)ys[j]=j;
    uint64_t st=catalogue[0]^((offset+1)*0x4da6f95146c0c0b5ULL);
    auto rnd=[&](){st^=st<<7;st^=st>>9;return unsigned(st);};
    int changes=0,done=0,drop=0;
    for(int r=0;r<sweeps;r++){
        for(int j=W-1;j>0;j--)swap(ys[j],ys[rnd()%(j+1)]);
        for(int y:ys){
            if(clock()>=stop)break;
            int oldcode=code[y],olde=oldcode/2;uint64_t sy=1ULL<<order[T+y];
            if(__builtin_popcountll(cols[2*olde]|cols[2*olde+1])<2)continue;
            uint64_t clear=~(dirbits[2*olde]|dirbits[2*olde+1]);cols[oldcode]^=sy;
            opts.clear();
            for(int id=0;id<int(pairs.size());id++){
                const auto& p=pairs[id];uint64_t g=(p.graph&clear)|render(p,cols[2*olde],cols[2*olde+1],olde,dirbits);
                // Fast rejection on the most common duplicate-column conflict.
                uint64_t x=g,t=(x^(x>>7))&0x00aa00aa00aa00aaULL;x^=t^(t<<7);
                t=(x^(x>>14))&0x0000cccc0000ccccULL;x^=t^(t<<14);
                t=(x^(x>>28))&0x00000000f0f0f0f0ULL;x^=t^(t<<28);
                if(x&g)continue;
                uint64_t cl=closure(g);if(cl&diag)continue;
                int cnt=__builtin_popcountll(cl);
                Option opt{id,rc[cnt],__builtin_popcountll(p.d&pref),{}};
                for(int k=0;k<D;k++){
                    uint64_t q=dirbits[k];
                    if(cl&dirbits[k^1]){opt.edgecost[k]=255;continue;}
                    if(cl&q){opt.edgecost[k]=rc[cnt];continue;}
                    int bit=__builtin_ctzll(q),u=bit/8,v=bit%8;
                    uint64_t pred=(((cl>>u)&one)|(1ULL<<(8*u)))*255;
                    uint64_t desc=(((cl>>(8*v))&((1<<B)-1))|(1ULL<<v))*one;
                    opt.edgecost[k]=rc[__builtin_popcountll(cl|(pred&desc))];
                }
                opts.push_back(opt);
            }
            int no=opts.size(),best=999999,bc=oldcode,bx=-1,prior=0;
            xlist.clear();xlist.push_back(-1);
            for(int t=0;t<screens;t++){int j=rnd()%T;if(find(xlist.begin(),xlist.end(),j)==xlist.end())xlist.push_back(j);}
            for(int x:xlist){
                uint64_t s=x<0?sy:1ULL<<order[x];
                for(int i=0;i<no;i++){
                    const auto&o=opts[i];const auto&p=pairs[o.index];
                    hc[i]=16*(o.hd+int(bool(p.d&sy))-int(bool(p.d&s)));
                    side[i]=!(p.d&s)?-1:!(p.white&s);
                }
                for(int turn=0;turn<D;turn++){
                    int k=(turn+oldcode)%D;
                    int head[1024];uint64_t exists[16]={};int count=0;
                    for(int i=no-1;i>=0;i--){
                        const auto&o=opts[i];int v=side[i]<0?o.basecost:o.edgecost[k^side[i]];
                        if(v==255)continue;count++;
                        int c=hc[i]+v;uint64_t mask=1ULL<<(c&63);
                        links[i]=(exists[c>>6]&mask)?head[c]:-1;head[c]=i;exists[c>>6]|=mask;
                    }
                    bool used[256]={};int total=0,nmatch=0;
                    for(int block=0;block<16&&nmatch<64+B;block++)for(uint64_t q=exists[block];q&&nmatch<64+B;q&=q-1){
                        int c=block*64+__builtin_ctzll(q);
                        for(int i=head[c];i>=0;i=links[i]){
                            const auto&p=pairs[opts[i].index];if(used[p.a]||used[p.b])continue;
                            used[p.a]=used[p.b]=true;total+=c;if(++nmatch==64+B)break;
                        }
                    }
                    total+=1024*(64+B-nmatch)-int(count * 1.5)+(x<0?0:colbias[order[x]]-colbias[order[T+y]]);if(x<0&&k==oldcode)prior=total;
                    if(total<best){best=total;bc=k;bx=x;}
                }
            }
            uint64_t replacement=sy;if(bx>=0){replacement=1ULL<<order[bx];swap(order[bx],order[T+y]);pref^=sy|replacement;}
            cols[bc]|=replacement;
            int nu=bc/2;auto[u,v]=ends[nu];if(bc&1)swap(u,v);layout[y]={u,v};code[y]=bc;
            if(bc!=oldcode||bx>=0){
                changes++;drop+=prior-best;
                clear&=~(dirbits[2*nu]|dirbits[2*nu+1]);
                for(auto&p:pairs)p.graph=(p.graph&clear)|render(p,cols[2*olde],cols[2*olde+1],olde,dirbits)
                    |render(p,cols[2*nu],cols[2*nu+1],nu,dirbits);
            }
            done++;
        }
    }
    cerr<<"EDGE_SWEEP done="<<done<<" changes="<<changes<<" gain="<<drop<<" cpu="<<double(clock()-started)/CLOCKS_PER_SEC<<"\n";
}

// Shorten an already legal trace while preserving its complete final deck.
// Only a repeated paper is omitted. All other papers keep the original
// relative order at each step, leaving one paper position as the DP state.
namespace trace_shortener {
using Move=std::pair<int,int>;
struct Parent {int16_t previous=-1,depth=-1;bool emit=false;};

inline std::vector<int> after_move(std::vector<int> order,Move op) {
    auto it=std::find(order.begin(),order.end(),op.first);
    order.erase(it);order.insert(order.begin()+op.second,op.first);return order;
}
inline bool visible(const std::vector<int>& order,int paper,const std::vector<uint64_t>& opaque) {
    uint64_t above=0;
    for(int k:order) {if(k==paper)return bool(opaque[k]&~above);above|=opaque[k];}
    return false;
}
inline std::vector<Move> omit_one(const std::vector<int>& start,
                                  const std::vector<Move>& moves,
                                  const std::vector<uint64_t>& opaque,int omitted,clock_t optional_deadline=0) {
    if(routing_budget::expired(optional_deadline)) {
        routing_budget::counters().trace_dp_aborts++;return moves;
    }
    const int m=moves.size(),paper=moves[omitted].first;
    std::vector<std::vector<int>> states{start},others;
    for(auto op:moves)states.push_back(after_move(states.back(),op));
    others.reserve(m+1);
    for(auto s:states) {s.erase(std::find(s.begin(),s.end(),paper));others.push_back(std::move(s));}
    constexpr int unreachable=32767;
    std::array<int,K> dp;dp.fill(unreachable);
    dp[std::find(start.begin(),start.end(),paper)-start.begin()]=0;
    std::vector<std::array<Parent,K>> parent(m+1);
    for(int i=0;i<m;i++) {
        if(routing_budget::expired(optional_deadline)) {
            routing_budget::counters().trace_dp_aborts++;return moves;
        }
        std::array<int,K> next;next.fill(unreachable);
        int q=moves[i].first;
        int old_index=q==paper?-1:int(std::find(others[i].begin(),others[i].end(),q)-others[i].begin());
        int target_index=q==paper?-1:int(std::find(others[i+1].begin(),others[i+1].end(),q)-others[i+1].begin());
        // Inserting `paper` changes another paper's visibility only when
        // inserted above it. The relative order of all other papers is fixed.
        std::array<uint64_t,K> prefix{};
        for(int p=0;p<K-1;p++)prefix[p+1]=prefix[p]|opaque[others[i][p]];
        uint64_t q_visible=q==paper?0:opaque[q]&~prefix[old_index];
        for(int p=0;p<K;p++)if(dp[p]<unreachable) {
            auto add=[&](int position,int depth,bool emit) {
                int cost=dp[p]+int(emit);
                if(cost<next[position]) {
                    next[position]=cost;
                    parent[i+1][position]={int16_t(p),int16_t(depth),emit};
                }
            };
            if(q==paper) {
                if(i==omitted || p==moves[i].second)add(p,-1,false);
                else if(opaque[paper]&~prefix[p])add(moves[i].second,moves[i].second,true);
            } else {
                int old_depth=old_index+int(p<=old_index);
                int remaining_depth=p-int(old_depth<p);
                int destination=target_index+int(remaining_depth<target_index);
                bool can_move=bool(q_visible & (p<=old_index ? ~opaque[paper] : ~uint64_t(0)));
                auto put=[&](int depth) {
                    bool emit=old_depth!=depth;
                    if(!emit || can_move)add(remaining_depth+int(depth<=remaining_depth),depth,emit);
                };
                put(destination);
                if(remaining_depth==target_index)put(destination+1);
            }
        }
        dp=next;
    }
    int p=std::find(states.back().begin(),states.back().end(),paper)-states.back().begin();
    if(dp[p]>=m)return moves;
    std::vector<Move> result;
    for(int i=m;i>0;i--) {
        Parent step=parent[i][p];assert(step.previous>=0);
        if(step.emit)result.emplace_back(moves[i-1].first,step.depth);
        p=step.previous;
    }
    std::reverse(result.begin(),result.end());
#ifndef NDEBUG
    auto order=start;
    for(auto op:result) {assert(visible(order,op.first,opaque));order=after_move(std::move(order),op);}
    assert(order==states.back());
#endif
    return result;
}
inline std::vector<Move> optimize(const std::vector<int>& start,std::vector<Move> moves,
                                  const std::vector<uint64_t>& opaque,clock_t optional_deadline=0) {
    if(routing_budget::expired(optional_deadline)) {
        routing_budget::counters().trace_skipped_calls++;return moves;
    }
    while(true) {
        std::array<int,K> counts{};
        for(auto op:moves)counts[op.first]++;
        auto best=moves;
        for(int i=0;i<int(moves.size());i++)if(counts[moves[i].first]>1) {
            if(routing_budget::expired(optional_deadline)) {
                routing_budget::counters().trace_skipped_calls++;return best;
            }
            auto candidate=omit_one(start,moves,opaque,i,optional_deadline);
            if(candidate.size()<best.size())best=std::move(candidate);
        }
        if(best.size()==moves.size())return moves;
        moves=std::move(best);
    }
}
} // namespace trace_shortener

// Remove the sampled order's unusually easy or difficult retrieval from the
// design estimate and add the conditional average in the current deck.
static double order_adjustment(const OnlineRouter& r,int item,int previous){
    array<int,64> count{};array<uint8_t,9> back{};int seen=0;
    uint64_t background=0;int total_depth=0,actual_depth=0,prev_depth=0;
    const int main=r.prod_to_paper[item],last=previous<0?-1:r.prod_to_paper[previous];
    for(int k:r.sim.order){
        if(k<r.tot_B){back[seen++]=k;continue;}
        if(k==r.active_main)continue;
        int w=r.paper_bucket[k],d=count[w]++;
        if(d==0)background|=r.black[k]&(1ULL<<w);
        total_depth+=d*(1+int(r.paper_is_paired[k]));
        if(k==main)actual_depth=d;
        if(k==last)prev_depth=d;
    }
    const int denom=P-(previous>=0);
    double z=double(total_depth-(previous>=0?prev_depth:0))/denom-actual_depth;
    int mismatches=0;
    for(int w=r.C;w<r.T;w++)mismatches+=r.hole_request_count[w][1^int(background>>w&1)];
    auto exposed=[&](int a){return __builtin_popcountll((background^r.A[a])&r.index_main_mask&~r.opaque[r.prod_to_paper[a]]);};
    z+=.6*(double(mismatches-(previous<0?0:exposed(previous)))/denom-exposed(item));
    if(r.graph && seen==r.tot_B){
        const auto& g=*r.graph;int state=g.table.rank(back);
        auto distance=[&](int a){const auto& p=g.req[a];return p.cached_distance?int(p.cached_distance->distance[state]):0;};
        if(g.future_value[state]>0)z+=(P*g.future_value[state]-(previous<0?0:distance(previous)))/denom-distance(item);
    }
    return z;
}

// Grow a comparison map from an empty suffix. Keep two distinct
// partial layouts, each evaluated by a matching on the cheapest viable pairs.
// Uniform mean of the exact legal insertion distance to a partial order.
// Relabeling topologically makes every requirement an upper-triangular mask.
template<int B> struct OrderMetric {
    static constexpr int M=B*(B-1)/2;
    array<uint16_t,1<<M> price{};
    array<array<int,B>,B> bit{};
    unordered_map<uint64_t,int> memo;
    OrderMetric(){
        const auto& tab=ExactBackingPermutations::get(B);const int S=tab.states;
        int m=0;for(int i=0;i<B;i++)for(int j=i+1;j<B;j++)bit[i][j]=1<<m++;
        vector<int> transitive;
        for(int mask=0;mask<(1<<M);mask++){
            bool good=true;
            for(int j=1;j+1<B && good;j++)for(int i=0;i<j;i++)if(mask&bit[i][j])
                for(int k=j+1;k<B;k++)if((mask&bit[j][k])&&!(mask&bit[i][k]))good=false;
            if(good)transitive.push_back(mask);
        }
        // Each linear extension relabels this poset into another upper-
        // triangular mask with exactly the same mean legal route length.
        vector<int> representative(1<<M,-1);
        vector<int> distinct;
        for(int mask:transitive)if(representative[mask]<0){
            distinct.push_back(mask);
            array<unsigned,B> pred{};
            for(int i=0;i<B;i++)for(int j=i+1;j<B;j++)if(mask&bit[i][j])pred[j]|=1u<<i;
            array<int,B> order{};
            auto relabel=[&](auto&& self,unsigned used,int depth,int code)->void{
                if(depth==B){representative[code]=mask;return;}
                for(int v=0;v<B;v++)if(!(used>>v&1) && !(pred[v]&~used)){
                    int next=code;
                    for(int i=0;i<depth;i++)if(pred[v]>>order[i]&1)next|=bit[i][depth];
                    order[depth]=v;self(self,used|1u<<v,depth+1,next);
                }
            };
            relabel(relabel,0,0,0);
        }
        vector<int> inv(S);for(int id=0;id<S;id++){
            auto& p=tab.permutations[id];
            for(int i=0;i<B;i++)for(int j=i+1;j<B;j++)if(p[i]>p[j])inv[id]|=bit[p[j]][p[i]];
        }
        vector<uint64_t> front(S),seen(S),spread(S);
        for(int t=0;t<int(distinct.size());t+=64){
            int qty=min(64,int(distinct.size())-t);uint64_t all=qty==64?~0ULL:(1ULL<<qty)-1;
            array<int,64> sum{};
            for(int id=0;id<S;id++){
                uint64_t v=0;for(int q=0;q<qty;q++)if(!(distinct[t+q]&inv[id]))v|=1ULL<<q;
                front[id]=seen[id]=v;
            }
            for(int distance=1;;distance++){
                fill(spread.begin(),spread.end(),0);
                for(int id=0;id<S;id++)if(front[id])
                    for(int prev:tab.reverse_edges[id])spread[prev]|=front[id];
                bool complete=true;
                for(int id=0;id<S;id++){
                    uint64_t fresh=spread[id]&~seen[id];front[id]=fresh;seen[id]|=fresh;complete&=seen[id]==all;
                    for(uint64_t f=fresh;f;f&=f-1)sum[__builtin_ctzll(f)]+=distance;
                }
                if(complete)break;
            }
            for(int q=0;q<qty;q++)price[distinct[t+q]]=lround(64.*sum[q]/S);
        }
        for(int mask:transitive)price[mask]=price[representative[mask]];
        const auto directed=price;
        for(int mask:transitive){
            int reverse=0;
            for(int i=0;i<B;i++)for(int j=i+1;j<B;j++)if(mask&bit[i][j])reverse|=bit[B-1-j][B-1-i];
            price[mask]=(int(directed[mask])+directed[reverse]+1)/2;
        }
        memo.reserve(B==6?12000:1000);memo.emplace(0,0);
    }
    int operator()(uint64_t graph){
        auto found=memo.find(graph);if(found!=memo.end())return found->second;
        array<uint8_t,B> seq{},degree{};
        for(int i=0;i<B;i++){
            degree[i]=__builtin_popcount((graph>>(8*i))&255);int at=i;
            while(at && degree[seq[at-1]]>degree[i]){seq[at]=seq[at-1];--at;}seq[at]=i;
        }
        int mask=0;for(int j=1;j<B;j++)for(int i=0;i<j;i++)if(graph>>(8*seq[j]+seq[i])&1)mask|=bit[i][j];
        int score=price[mask];memo.emplace(graph,score);return score;
    }
};

template<int B> struct CoreGenesis{
    static constexpr int M=B*(B-1)/2,D=2*M;
    struct Pair {int a,b;uint64_t delta,zero;};
    struct Live {int id,price;uint64_t pred;};
    struct Node {int score=0;uint64_t prefix=~uint64_t(0);uint16_t covered=0;vector<Live> live;vector<int> path;};
    struct Move {int total=0,from=-1,col=-1,arc=0;};
    static constexpr uint64_t one=0x0101010101010101ULL>>(8*(8-B));
    static uint64_t add(uint64_t g,int u,int v){
        auto target=(((g>>v)&one)|(1ULL<<(8*v)))*255;
        auto preds=(((g>>(8*u))&((1u<<B)-1))|(1ULL<<u))*one;
        return g|(target&preds);
    }
    static bool generate(const vector<uint64_t>&cat,vector<int>& order,vector<pair<int,int>>&layout,int W,int seed,clock_t deadline){
        const auto colbias=catalogue_column_penalty(cat);
        vector<Pair> pairs;
        static OrderMetric<B> uniform_metric;
        const int blend=clamp(search_budget::integer("METRIC_BLEND",4),0,8);
        array<int,M+1> route{};
        for(int i=0;i<=M;i++)route[i]=round(46*sqrt(double(i)));
        auto cost=[&](uint64_t g){
            int old=route[__builtin_popcountll(g)];
            return (old*(8-blend)+blend*uniform_metric(g)+4)/8;
        };
        array<int,D> u{},v{};int code=0;
        for(int i=0;i<B;i++)for(int j=i+1;j<B;j++){u[code]=i;v[code++]=j;u[code]=j;v[code++]=i;}
        Node initial;
        for(int a=0;a<P;a++)for(int b=a+1;b<P;b++){
            uint64_t d=cat[a]^cat[b];int hd=__builtin_popcountll(d);if(hd>30)continue;
            initial.live.push_back({int(pairs.size()),16*hd,0});pairs.push_back({a,b,d,d&~cat[a]});
        }
        vector<Node> frontier{move(initial)};uint64_t st=cat[0]^(0x305fa1d15cf744d3ULL*(seed+1));auto rnd=[&](){st^=st<<7;st^=st>>9;return unsigned(st);};
        constexpr int width=2,top=600,screen=12;
        constexpr int required=P-(K-B);
        struct PairMove {int a,b,c;};
        struct Adjustment{int rm1=-1,rm2=-1,add1=-1,add2=-1,gain=0;};
        auto polish=[&](vector<PairMove>& es,int valid){
            sort(es.begin(),es.end(),[](auto p,auto q){return p.c<q.c||(p.c==q.c&&pair{p.a,p.b}<pair{q.a,q.b});});
            array<int,P> used;used.fill(-1);int sum=0,n=0;
            for(int i=0;i<int(es.size())&&n<required;i++){
                auto e=es[i];if(used[e.a]>=0||used[e.b]>=0)continue;
                used[e.a]=used[e.b]=i;n++;sum+=e.c;
            }
            static array<int16_t,P*P> lookup{};for(int i=0;i<int(es.size());i++)lookup[es[i].a*P+es[i].b]=i+1;
            auto remove=[&](int i){if(i<0)return;auto e=es[i];used[e.a]=used[e.b]=-1;sum-=e.c;n--;};
            auto insert=[&](int i){if(i<0)return;auto e=es[i];used[e.a]=used[e.b]=i;sum+=e.c;n++;};
            for(int it=0;it<8;it++){
                int w=-1,w2=-1;
                for(int a=0;a<P;a++){int i=used[a];if(i<0||es[i].a!=a)continue;
                    if(w<0||es[i].c>es[w].c){w2=w;w=i;}else if(w2<0||es[i].c>es[w2].c)w2=i;
                }
                array<int,P> f1,f2;f1.fill(-1);f2.fill(-1);Adjustment best;
                auto offer=[&](int gain,int r,int s,int a,int b){
                    if(gain>best.gain)best={r,s,a,b,gain};
                };
                for(int i=0;i<int(es.size());i++){
                    auto e=es[i];int x=used[e.a],y=used[e.b];if(x==i)continue;
                    if(x<0 && y<0){
                        int j=n<required?-1:w;offer((j<0?768:es[j].c)-e.c,j,-1,i,-1);continue;
                    }
                    if(x<0||y<0){
                        int a=x<0?e.b:e.a,j=max(x,y);
                        if(f1[a]<0)f1[a]=i;else if(f2[a]<0)f2[a]=i;
                        offer(es[j].c-e.c,j,-1,i,-1);continue;
                    }
                    int a=es[x].a^es[x].b^e.a,b=es[y].a^es[y].b^e.b;
                    int j=lookup[min(a,b)*P+max(a,b)]-1;
                    if(j>=0)offer(es[x].c+es[y].c-e.c-es[j].c,x,y,i,j);
                }
                for(int a=0;a<P;a++)if(used[a]>=0&&es[used[a]].a==a){
                    int x=used[a],b=es[x].b,j=n<required?-1:(w==x?w2:w);
                    for(int l:{f1[a],f2[a]})for(int r:{f1[b],f2[b]})if(l>=0&&r>=0 && (es[l].a^es[l].b^a)!=(es[r].a^es[r].b^b))
                        offer(es[x].c+(j<0?768:es[j].c)-es[l].c-es[r].c,x,j,l,r);
                }
                if(best.gain<=0)break;
                remove(best.rm1);remove(best.rm2);insert(best.add1);insert(best.add2);
            }
            for(auto e:es)lookup[e.a*P+e.b]=0;
            return sum+(required-n)*768-valid;
        };
        for(int step=0;step<W;step++){
            if(construction_deadline_expired(deadline))return false;
            vector<Move> trials;
            for(int i=0;i<int(frontier.size());i++){
                auto& node=frontier[i];int no=min(top,int(node.live.size()));
                // The viable pairs stay in pair-ID order. Bucket sorting their
                // integer prices gives the same cheapest prefix as partial_sort.
                array<int,1536> heads;heads.fill(-1);
                vector<int> links(node.live.size());
                for(int j=int(node.live.size())-1;j>=0;--j){int cost=node.live[j].price;links[j]=heads[cost];heads[cost]=j;}
                vector<int> ranked;ranked.reserve(no);
                for(int c=0;int(ranked.size())<no;c++)for(int j=heads[c];j>=0 && int(ranked.size())<no;j=links[j])ranked.push_back(j);
                struct Price{int a,b,c;uint64_t d,z;array<uint8_t,D> inc;};
                vector<Price> prices;prices.reserve(no);
                for(int h=0;h<no;h++){
                    auto &l=node.live[ranked[h]];auto &p=pairs[l.id];Price row{p.a,p.b,l.price,p.delta,p.zero,{}};
                    // A reachable reverse edge means a cycle. Otherwise the
                    // closure increment depends only on this pair and direction.
                    int old=cost(l.pred);
                    for(int k=0;k<D;k++)row.inc[k]=(l.pred>>(8*u[k]+v[k]))&1?255:cost(add(l.pred,u[k],v[k]))-old;
                    prices.push_back(row);
                }
                int start=trials.size();
                const int words=(no+63)/64;
                vector<array<uint64_t,D>> blocked(words);
                for(int h=0;h<no;h++)for(int k=0;k<D;k++)if(prices[h].inc[k]==255)
                    blocked[h>>6][k]|=1ULL<<(h&63);
                vector<uint64_t> zeros(words),ones(words);
                vector<int> cols;for(uint64_t a=node.prefix;a;a&=a-1)cols.push_back(__builtin_ctzll(a));
                for(int j=0;j<min(screen,int(cols.size()));j++){
                    swap(cols[j],cols[j+rnd()%(cols.size()-j)]);int col=cols[j];uint64_t bit=1ULL<<col;
                    fill(zeros.begin(),zeros.end(),0);fill(ones.begin(),ones.end(),0);
                    for(int h=0;h<no;h++)if(prices[h].d&bit)
                        ((prices[h].z&bit)?zeros:ones)[h>>6]|=1ULL<<(h&63);
                    for(int k=0;k<D;k++){
                        // Reserve one distinct witness for each backing pair.
                        if(step<M&&(node.covered>>(k/2)&1))continue;
                        array<bool,P> used{};int money=0,valid=0,n=0;
                        for(int b=0;b<words;b++){
                            uint64_t ok=~((zeros[b]&blocked[b][k])|(ones[b]&blocked[b][k^1]));
                            if(b==words-1 && (no&63))ok&=(1ULL<<(no&63))-1;
                            valid+=__builtin_popcountll(ok);
                            while(ok && n<required){
                                const auto& row=prices[b*64+__builtin_ctzll(ok)];ok&=ok-1;
                                if(used[row.a]||used[row.b])continue;
                                used[row.a]=used[row.b]=true;++n;
                                bool d=bit&row.d;money+=row.c-16*d+(d?row.inc[k^int(!(row.z&bit))]:0);
                            }
                        }
                        trials.push_back({money+(required-n)*768-valid+colbias[col],i,col,k});
                    }
                }
                if(step>=21){
                    constexpr int cap=5; int nend=trials.size();
                    if(nend-start>cap){
                        nth_element(trials.begin()+start,trials.begin()+start+cap,trials.end(),[](auto a,auto b){return a.total<b.total;});
                        trials.resize(start+cap);
                    }
                    vector<PairMove> eligible;eligible.reserve(no);
                    for(int idx=start;idx<int(trials.size());idx++){
                        auto& candidate=trials[idx];uint64_t bit=1ULL<<candidate.col;eligible.clear();
                        for(const auto& r:prices){
                            int c=r.c;if(r.d&bit){int p=r.inc[candidate.arc^int(!(r.z&bit))];if(p==255)continue;c+=p-16;}
                            eligible.push_back({r.a,r.b,c});
                        }
                        candidate.total=polish(eligible,eligible.size())+colbias[candidate.col];
                    }
                }
            }
            sort(trials.begin(),trials.end(),[](auto& a,auto& b){return a.total<b.total;});
            vector<Node> next;
            for(auto m:trials){
                auto& par=frontier[m.from];uint64_t bit=1ULL<<m.col;uint64_t pref=par.prefix^bit;
                // Avoid spending both branches on the same physical split
                // before enough comparisons exist to distinguish the layouts.
                bool dup=false;for(auto& n:next)if(n.prefix==pref && (step<3||m.total==n.score))dup=true;
                if(dup)continue;
                Node child;child.prefix=pref;child.covered=par.covered|(1u<<(m.arc/2));child.path=par.path;child.path.push_back(m.col*32+m.arc);child.score=m.total;child.live.reserve(par.live.size());
                for(auto q:par.live){auto& p=pairs[q.id];if(p.delta&bit){
                        int k=m.arc^int(!(p.zero&bit));if(q.pred>>(8*u[k]+v[k])&1)continue;
                        uint64_t g=add(q.pred,u[k],v[k]);q.price+=cost(g)-cost(q.pred)-16;q.pred=g;
                    }
                    child.live.push_back(q);
                }
                next.push_back(move(child));
                if(int(next.size())==width || step+1==W)break;
            }
            if(next.empty())return false;
            frontier=move(next);
        }
        auto& best=frontier[0];order.clear();layout.clear();
        for(uint64_t bits=best.prefix;bits;bits&=bits-1)order.push_back(__builtin_ctzll(bits));
        for(int op:best.path){order.push_back(op/32);layout.emplace_back(u[op%32],v[op%32]);}
        return true;
    }
};

// Minimum-cost cardinality matching on the actual crossing product pairs.
// Each shortest augmenting path preserves optimality for its cardinality.
struct SparsePairEdge {int right;double cost;};
static bool pair_flow(const vector<vector<SparsePairEdge>>& edges,int want,vector<int>& owner){
    const int n=int(edges.size());
    vector<int> mate(n,-1);owner.assign(n,-1);
    vector<double> picked(n),dist(2*n);
    vector<int> previous(n),queue(2*n+1);
    vector<uint8_t> active(2*n);
    for(int flow=0;flow<want;flow++){
        fill(dist.begin(),dist.end(),1e30);fill(active.begin(),active.end(),0);
        int head=0,tail=0;
        auto push=[&](int v){if(!active[v]){active[v]=1;queue[tail++]=v;if(tail==int(queue.size()))tail=0;}};
        for(int i=0;i<n;i++)if(mate[i]<0 && !edges[i].empty()){dist[i]=0;push(i);}
        while(head!=tail){
            int v=queue[head++];if(head==int(queue.size()))head=0;active[v]=0;
            if(v<n){
                for(const auto& e:edges[v])if(e.right!=mate[v]){
                    int u=n+e.right;double x=dist[v]+e.cost;
                    if(x<dist[u]-1e-9){dist[u]=x;previous[e.right]=v;push(u);}
                }
            }else{
                int j=v-n,i=owner[j];if(i<0)continue;
                double x=dist[v]-picked[j];
                if(x<dist[i]-1e-9){dist[i]=x;push(i);}
            }
        }
        int j=-1;double best=1e29;
        for(int k=0;k<n;k++)if(owner[k]<0 && dist[n+k]<best){best=dist[n+k];j=k;}
        if(j<0)return false;
        while(j>=0){
            int i=previous[j],next=mate[i];
            for(const auto& e:edges[i])if(e.right==j){picked[j]=e.cost;break;}
            owner[j]=i;mate[i]=j;j=next;
        }
    }
    return true;
}

// Estimate how often a missing window is actually supplied by its home bank
// when that bank starts the order with the wrong front color.
struct BankFeedback {
    vector<array<int,2>> base,seen,used;
    explicit BankFeedback(const OnlineRouter& r):base(r.hole_request_count),seen(r.T),used(r.T){}
    uint64_t background(const OnlineRouter& r) const {
        array<uint8_t,K> borrowed{};for(int k:r.active_supports)borrowed[k]=1;
        uint64_t printed=0,black=0;
        for(int k:r.sim.order)if(k>=r.tot_B && k!=r.active_main && !borrowed[k]){
            uint64_t bit=1ULL<<r.paper_bucket[k];
            black|=r.black[k]&bit&~printed;printed|=bit;
        }
        return black;
    }
    void observe(OnlineRouter& r,int a,uint64_t before,int round){
        uint64_t open=(before^r.A[a])&r.index_main_mask&~r.opaque[r.active_main];
        array<uint8_t,K> borrowed{};for(int k:r.active_supports)borrowed[k]=1;
        for(int k:r.sim.order){
            uint64_t fresh=r.opaque[k]&open;
            if(k>=r.tot_B && k!=r.active_main && !borrowed[k]){
                int w=r.paper_bucket[k];if(fresh>>w&1)used[w][r.A[a]>>w&1]++;
            }
            for(uint64_t bits=fresh;bits;bits&=bits-1){int w=__builtin_ctzll(bits);seen[w][r.A[a]>>w&1]++;}
            open&=~fresh;if(!open)break;
        }
        if((round&31)!=31)return;
        constexpr double mix=.45;
        for(int w=r.C;w<r.T;w++)for(int c=0;c<2;c++){
            double p=(6.+used[w][c])/(12.+seen[w][c]);
            r.hole_request_count[w][c]=lround(base[w][c]*(1+mix*(2*p-1)));
        }
        r.joint_extract_cache.clear();
    }
};

struct HeadSample{
    uint64_t black=0;
    array<uint8_t,K> depth{};
};
static void capture_heads(const OnlineRouter& t, HeadSample& sample){
    unsigned char rank[64]{};uint64_t seen=0;
    for(int k:t.sim.order){
        if(k<t.tot_B)continue;
        int w=t.paper_bucket[k];uint64_t b=1ull<<w;
        if(k==t.active_main||find(t.active_supports.begin(),t.active_supports.end(),k)!=t.active_supports.end()){
            sample.depth[k]=0;
        }else{
            if(!(seen&b))seen|=b,sample.black|=t.black[k]&b;
            sample.depth[k]=rank[w]++;
        }
    }
}
static void history_holes(Design& d,const vector<HeadSample>& hist){
    if(hist.empty()||d.C!=0)return;
    const int H=hist.size(),B=d.tot_B,T=d.T;uint64_t prefix=(1ULL<<T)-1;
    const auto& A=d.internal_catalog;const auto& main=d.prod_to_paper;
    vector<uint64_t> ink(K),black(K),holes(P);
    vector<vector<int>> own(K);
    for(int k=B;k<K;k++)for(int j=0;j<T;j++){
        ink[k]|=uint64_t(d.S_init[k][j]!='.')<<j;black[k]|=uint64_t(d.S_init[k][j]=='1')<<j;
    }
    for(int a=0;a<P;a++){own[main[a]].push_back(a);holes[a]=prefix&~ink[main[a]];}
    auto cover=[&](int k,int a,uint64_t mask,uint64_t missing){
        const int w=d.paper_bucket[k];if(!(missing>>w&1))return uint64_t(0);
        uint64_t bad=(black[k]^A[a])&mask&missing;
        uint64_t left=bad?~uint64_t(0)<<(64-__builtin_clzll(bad)):~uint64_t(0);
        if(!(left>>w&1))return uint64_t(0);
        return mask&missing&left;
    };
    auto value=[&](int a,int k,int h,uint64_t cov){
        return max(0,__builtin_popcountll((hist[h].black^A[a])&cov)-int(hist[h].depth[k])-2);
    };
    int budget=search_budget::integer("HIST_BUDGET",12),lookup=search_budget::integer("HIST_LOOKUP",100);
    double threshold=search_budget::real("HIST_THRESH",.15),own_price=search_budget::real("HIST_OWN",1.0);
    bool used[K]={};int cuts=0;double predicted=0;
    int loops=search_budget::integer("HIST_LOOPS",2);
    for(int iteration=0;iteration<loops;iteration++){
        struct Best{short max=0,second=0,k=-1;};
        vector<Best> top(P*H);
        for(int a=0;a<P;a++)for(int k=B;k<K;k++)if(k!=main[a]){
            uint64_t cov=cover(k,a,ink[k],holes[a]);if(!cov)continue;
            for(int h=0;h<H;h++){
                int v=value(a,k,h,cov);auto& b=top[a*H+h];
                if(v>b.max){b.second=b.max;b.max=v;b.k=k;}else if(v>b.second)b.second=v;
            }
        }
        vector<array<int,64>> delta(K);
        for(int k=B;k<K;k++)if(!used[k])for(int a=0;a<P;a++)if(main[a]!=k){
            uint64_t missing=holes[a],b=1ull<<d.paper_bucket[k],bad=(black[k]^A[a])&ink[k]&missing;
            if(!(missing&b))continue;
            int cut=bad?63-__builtin_clzll(bad):-1;
            uint64_t cov=cover(k,a,ink[k],missing),ext=0;
            if(cut>=0 && cut<d.paper_bucket[k])ext=cover(k,a,ink[k]^(1ull<<cut),missing);
            if(!(cov|ext))continue;
            for(int h=0;h<H;h++){
                const auto& b0=top[a*H+h];uint64_t mismatch=hist[h].black^A[a];
                if(ext){
                    int gain=value(a,k,h,ext)-b0.max;
                    if(gain>0)delta[k][cut]+=gain;
                }
                if(b0.k==k && b0.max>b0.second){
                    uint64_t loss=mismatch&cov&(b-1);
                    for(;loss;loss&=loss-1)delta[k][__builtin_ctzll(loss)]--;
                }
            }
        }
        struct Candidate{int k,j;double score;};vector<Candidate> local;
        for(int k=B;k<K;k++)if(!used[k])for(int j=0;j<d.paper_bucket[k];j++)if(ink[k]>>j&1){
            double estimate=double(delta[k][j])/H;
            if(estimate<.01)continue;
            uint64_t b=1ull<<j;
            for(int a:own[k])for(const auto &s:hist)if((s.black^A[a])&b)estimate-=own_price/H;
            local.push_back({k,j,estimate});
        }
        sort(local.begin(),local.end(),[](auto a,auto b){return a.score>b.score;});
        if(int(local.size())>lookup)local.resize(lookup);
        for(auto &c:local){
            uint64_t bit=1ull<<c.j;
            for(int a:own[c.k]){
                vector<int> best(H);
                uint64_t miss=holes[a]|bit;
                for(int k=B;k<K;k++)if(k!=c.k){
                    uint64_t cov=cover(k,a,ink[k],miss);if(!cov)continue;
                    for(int h=0;h<H;h++)best[h]=max(best[h],value(a,k,h,cov));
                }
                for(int h=0;h<H;h++)c.score+=double(best[h]-top[a*H+h].max)/H;
            }
        }
        sort(local.begin(),local.end(),[](auto a,auto b){return a.score>b.score;});
        int changed=0;
        for(auto c:local)if(!used[c.k]&&c.score>=threshold&&changed*loops<budget&&cuts<budget){
            uint64_t bit=1ull<<c.j;d.S_init[c.k][c.j]='.';ink[c.k]&=~bit;
            for(int a:own[c.k])holes[a]|=bit;
            used[c.k]=true;predicted+=c.score;cuts++;changed++;
        }
        if(!changed)break;
    }
    cerr<<"HEAD_ERASE samples="<<H<<" cuts="<<cuts<<" proxy="<<predicted<<'\n';
}

static void empirical_utility(OnlineRouter& t,const vector<HeadSample>& hist){
    if(hist.empty()||t.C!=0)return;
    int H=hist.size();
    double mix=search_budget::real("UTIL_MIX",.5);
    vector<array<int,65>> counts(K);
    struct Competitor{short k=-1,score=0,second=0;};
    for(int a=0;a<P;a++){
        vector<Competitor> best(H);
        int main=t.prod_to_paper[a];
        uint64_t holes=t.index_main_mask&~t.opaque[main],aim=t.A[a];
        vector<uint64_t> cover(K);
        for(int k=t.tot_B;k<K;k++)if(k!=main){
            int w=t.paper_bucket[k];if(!(holes>>w&1))continue;
            uint64_t bad=(t.black[k]^aim)&t.opaque[k]&holes;int cut=bad?63-__builtin_clzll(bad):-1;
            if(cut>=w)continue;
            uint64_t cov=cover[k]=t.opaque[k]&holes&(~0ULL<<(cut+1))&~(1ull<<w);
            if(!cov)continue;
            for(int h=0;h<H;h++){
                auto& out=best[h];int s=__builtin_popcountll((hist[h].black^aim)&cov)-2-hist[h].depth[k];
                if(s>out.score)out.second=out.score,out.score=s,out.k=k;
                else out.second=max<int>(out.second,s);
            }
        }
        for(int k=t.tot_B;k<K;k++)if(cover[k]){
            for(int h=0;h<H;h++){
                const auto& b=best[h];
                int d=__builtin_popcountll((hist[h].black^aim)&cover[k])-2-(b.k==k?b.second:b.score);
                if(d>0)counts[k][d]++;
            }
        }
    }
    for(int k=t.tot_B;k<K;k++){double s=0,n=0;for(int r=64;r>=0;r--){
        if(r<32){double v=s/H;t.support_utility[k][r]+=mix*(v-t.support_utility[k][r]);}
        n+=counts[k][r];s+=n;
    }}
}

struct BankTemplates {
    uint64_t prefix;
    array<int,K> owner{},other{};
    array<uint64_t,K> bits{},printable{};
    explicit BankTemplates(const Design& d):prefix((1ULL<<d.T)-1) {
        owner.fill(-1);other.fill(-1);
        for(int a=0;a<P;a++){int k=d.prod_to_paper[a];(owner[k]<0?owner[k]:other[k])=a;}
        for(int k=d.tot_B;k<K;k++){
            int w=d.paper_bucket[k];bits[k]=d.internal_catalog[owner[k]];printable[k]=prefix;
            if(other[k]>=0)printable[k]&=~(bits[k]^d.internal_catalog[other[k]]);
            for(int j=0;j<w;j++)if(d.S_init[k][j]=='.')printable[k]&=~(1ULL<<j);
        }
    }
};
static void sampled_bank_swaps(Design& d,const vector<HeadSample>& history){
    if(d.C!=0||d.T>44||history.empty())return;
    BankTemplates b(d);const int B=d.tot_B,T=d.T;
    int H=min<int>(history.size(),search_budget::integer("SW_SAMPLES",24));
    const int span=search_budget::integer("SW_SPAN",7),limit=search_budget::integer("SW_CAP",5);
    const double drift=search_budget::real("SW_DRIFT",.04),minimum=search_budget::real("SW_MIN",.75);
    const auto& A=d.internal_catalog;
    vector<HeadSample> hist;for(int h=0;h<H;h++)hist.push_back(history[h*history.size()/H]);
    array<uint64_t,K> ink{};array<uint64_t,P> holes{};using Pair=array<double,2>;
    array<Pair,64> prob{};int count[64][2]{},head[64][2]{};
    for(int k=B;k<K;k++)ink[k]=b.printable[k]&((1ull<<(d.paper_bucket[k]+1))-1);
    for(int a=0;a<P;a++){holes[a]=b.prefix&~ink[d.prod_to_paper[a]];
        for(uint64_t v=holes[a];v;v&=v-1){int j=__builtin_ctzll(v);count[j][A[a]>>j&1]++;}}
    for(const auto& h:hist)for(int j=0;j<T;j++)head[j][h.black>>j&1]++;
    for(int j=0;j<T;j++)for(int c=0;c<2;c++){
        double flip=double(count[j][1-c]+3)/(count[j][0]+count[j][1]+6);
        prob[j][c]=search_budget::real("SW_ADAPT",.65)*(2*flip*flip-double(head[j][1-c])/H);
    }
    auto coverage=[&](int a,int k,int w,uint64_t op,uint64_t missing){
        if(!(missing>>w&1))return uint64_t(0);
        uint64_t cov=op&missing,bad=(b.bits[k]^A[a])&cov;
        if(bad){int cut=63-__builtin_clzll(bad);if(cut>=w)return uint64_t(0);cov&=~0ull<<(cut+1);}
        return cov;
    };
    struct Top{uint8_t value=0,second=0;short id=-1;};vector<Top> top(P*H);
    for(int a=0;a<P;a++)for(int k=B;k<K;k++)if(k!=d.prod_to_paper[a]){
        uint64_t cov=coverage(a,k,d.paper_bucket[k],ink[k],holes[a]);
        if(__builtin_popcountll(cov)<3)continue;
        for(int h=0;h<H;h++){int val=__builtin_popcountll((hist[h].black^A[a])&cov)-hist[h].depth[k]-2;
            Top& t=top[a*H+h];if(val>t.value)t.second=t.value,t.value=val,t.id=k;
            else if(val>t.second)t.second=val;}
    }
    array<double,K> olddonor{};for(const auto& t:top)if(t.id>=0)olddonor[t.id]+=double(t.value-t.second)/H;
    using Matrix=array<double,64>;
    vector<Matrix> target(K);for(int k=B;k<K;k++){
        int old=d.paper_bucket[k];for(int w=max(0,old-span);w<=min(T-1,old+span);w++)if(b.printable[k]>>w&1){
            uint64_t missing=b.prefix&~(b.printable[k]&((1ull<<(w+1))-1));double cost=0;
            for(int a:{b.owner[k],b.other[k]})if(a>=0){
                for(uint64_t v=missing;v;v&=v-1){int j=__builtin_ctzll(v);cost+=prob[j][A[a]>>j&1];}
                vector<int> best(H);
                for(int donor=B;donor<K;donor++)if(donor!=k){
                    uint64_t cov=coverage(a,donor,d.paper_bucket[donor],ink[donor],missing);
                    if(__builtin_popcountll(cov)<3)continue;
                    for(int h=0;h<H;h++)best[h]=max(best[h],__builtin_popcountll((hist[h].black^A[a])&cov)-hist[h].depth[donor]-2);
                }
                int many=0;for(int h=0;h<H;h++)many+=__builtin_popcountll((hist[h].black^A[a])&missing)-best[h];
                cost+=double(many)/H;
            }target[k][w]=cost;
        }
    }
    // For a fixed new bank, all slots give this card the same covered
    // windows. Accumulate the counterfactual gain at each possible depth,
    // then look up each slot's observed depth in the same sampled deck.
    vector<array<double,K>> marginal(K);
    for(int k=B;k<K;k++){
        int old=d.paper_bucket[k];
        for(int w=max(0,old-span);w<=min(T-1,old+span);w++)if(w!=old && (b.printable[k]>>w&1)){
            vector<int> slots;
            for(int y=B;y<K;y++)if(d.paper_bucket[y]==w && d.paper_is_paired[k]==d.paper_is_paired[y] &&
                !((b.bits[k]^b.bits[y])>>w&1))slots.push_back(y);
            if(slots.empty())continue;
            vector<array<int,65>> gains(H);
            uint64_t op=b.printable[k]&((1ull<<(w+1))-1);
            for(int a=0;a<P;a++)if(d.prod_to_paper[a]!=k){
                uint64_t cov=coverage(a,k,w,op,holes[a]);if(__builtin_popcountll(cov)<3)continue;
                for(int h=0;h<H;h++){
                    const Top& t=top[a*H+h];int alt=t.id==k?t.second:t.value;
                    int benefit=__builtin_popcountll((hist[h].black^A[a])&cov)-2-alt;
                    for(int r=0;r<benefit;r++)gains[h][r]+=benefit-r;
                }
            }
            for(int y:slots){int u=0;for(int h=0;h<H;h++)u+=gains[h][min<int>(64,hist[h].depth[y])];
                marginal[k][y]=target[k][w]-target[k][old]+olddonor[k]-double(u)/H+drift*abs(w-old);
            }
        }
    }
    struct Cycle{array<int,4> cards;int n;double gain;};vector<Cycle> choices;
    array<array<float,K>,K> distance{};
    vector<vector<int>> edges(K);
    for(int x=B;x<K;x++)for(int y=B;y<K;y++){
        int u=d.paper_bucket[x],v=d.paper_bucket[y];
        distance[x][y]=1e15;
        if(u==v||abs(u-v)>span||d.paper_is_paired[x]!=d.paper_is_paired[y]||
            !(b.printable[x]>>v&1)||((b.bits[x]^b.bits[y])>>v&1))continue;
        distance[x][y]=marginal[x][y];
        if(distance[x][y]<4.0)edges[x].push_back(y);
    }
    double risk=search_budget::real("SW_RISK",.3);
    auto save=[&](int x,int y,int z,int t,int n,double sum){
        double value=-sum-minimum*.5*n-risk*(n-2);
        if(value>0)choices.push_back({{x,y,z,t},n,value/sqrt(n)});
    };
    const int four=search_budget::integer("SW_FOUR",0);
    for(int x=B;x<K;x++)for(int y:edges[x])if(y>x){
        save(x,y,0,0,2,distance[x][y]+distance[y][x]);
        for(int z:edges[y])if(z>x&&z!=y){
            save(x,y,z,0,3,distance[x][y]+distance[y][z]+distance[z][x]);
            if(four && distance[x][y]+distance[y][z]<-1)
                for(int t:edges[z])if(t>x&&t!=y&&t!=z)
                    save(x,y,z,t,4,distance[x][y]+distance[y][z]+distance[z][t]+distance[t][x]);
        }
    }
    sort(choices.begin(),choices.end(),[](auto a,auto b){return a.gain>b.gain;});
    array<bool,K> used{};array<int,64> count_used{};int n=0,m=0;
    auto orig=d.S_init;
    for(auto& cyc:choices){
        bool valid=m+cyc.n<=2*limit;
        for(int j=0;j<cyc.n;j++){int k=cyc.cards[j];valid&=!used[k]&&count_used[d.paper_bucket[k]]<2;}
        if(!valid)continue;
        for(int j=0;j<cyc.n;j++){
            int k=cyc.cards[j],slot=cyc.cards[(j+1)%cyc.n],w=d.paper_bucket[slot];
            used[k]=true;count_used[w]++;d.S_init[slot]=orig[k];
            for(int i=0;i<T;i++)d.S_init[slot][i]=(i<=w&&(b.printable[k]>>i&1))?char('0'+int(b.bits[k]>>i&1)):'.';
            for(int a:{b.owner[k],b.other[k]})if(a>=0)d.prod_to_paper[a]=slot;
            if(b.other[k]>=0 && !d.pairs.empty()){auto& p=d.pairs[d.prod_to_pair_idx[b.owner[k]]];p.paper_id=slot;p.bucket_w=w;}
        }
        n++;m+=cyc.n;if(m>=2*limit-1)break;
    }
    if(getenv("SW_LOG"))cerr<<"BANK_CYCLES candidates="<<choices.size()<<" selected="<<n<<" cards="<<m<<"\n";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    const clock_t design_started = clock();
    cerr<<"NOV8_BUDGET scale="<<search_budget::scale()<<" optional_end="
        <<search_budget::seconds(search_budget::real("NOV8_RELAX_END",3.00))<<"\n";
    auto design_elapsed = [&]() { return double(clock()-design_started)/CLOCKS_PER_SEC/search_budget::scale(); };
    auto absolute_deadline = [&](double seconds) { return design_started+clock_t(search_budget::seconds(seconds)*CLOCKS_PER_SEC); };
    int n_in, k_in, p_in, r_in;
    if (!(cin >> n_in >> k_in >> p_in >> r_in)) return 0;

    vector<uint64_t> A(P, 0);
    for (int i = 0; i < P; ++i) {
        string s;
        cin >> s;
        uint64_t mask = 0;
        for (int j = 0; j < N; ++j) {
            if (s[j] == '1') mask |= (1ULL << j);
        }
        A[i] = mask;
    }

    // A universally feasible fallback, selected before any design is printed.
    auto fallback = [&]() -> int {
        vector<string> sheets(K, string(N, '.'));
        for (int j=0;j<N;++j) {
            sheets[2*j][j]='0'; sheets[2*j+1][j]='1';
        }
        for (const auto &s:sheets) cout << s << '\n';
        cout.flush();
        vector<int> order(K); for(int k=0;k<K;++k) order[k]=k;
        uint64_t display=0;
        for(int q=0;q<R;++q) {
            int a; if(!(cin>>a)||a==-1) return 0;
            vector<pair<int,int>> moves;
            uint64_t diff=display^A[a];
            while(diff) {
                int j=__builtin_ctzll(diff); diff&=diff-1;
                int k=2*j+((display>>j)&1ULL), other=k^1;
                int from=find(order.begin(),order.end(),k)-order.begin();
                int to=find(order.begin(),order.end(),other)-order.begin();
                order.erase(order.begin()+from); order.insert(order.begin()+to,k);
                moves.emplace_back(k,to);
            }
            display=A[a];
            cout << moves.size() << '\n';
            for(auto [k,d]:moves) cout<<k<<' '<<d<<'\n';
            cout.flush();
        }
        return 0;
    };


    const double pack_deadline=search_budget::real("NOV8_PACK_END",1.95),
        matching_deadline=search_budget::real("NOV8_MATCH_END",2.00),
        layout_deadline=search_budget::real("NOV8_LAYOUT_END",2.05);
    int graph_seed_offset=0;
    const Design* parent_design=nullptr;


    struct PackSnapshot {
        int W, B;
        vector<uint64_t> a, domains, masks, bits;
        vector<int> columns, route;
        vector<PairInfo> pairs;
        double price;
    };
    vector<PackSnapshot> snapshots;
    auto build_design = [&](int pack_bf,int pack_bl,int pack_br,int pack_start,const PackSnapshot* replay=nullptr) -> optional<Design> {
    const vector<uint64_t> original_catalog=A;
    vector<uint64_t> local_A=A;
    auto& A=local_A;
    vector<int> column_order(64);for(int j=0;j<64;j++)column_order[j]=j;
    // try_pack selects the suffix width and backing allocation below.
    int W = 34;
    int B_full = 8, B_left = 4, B_right = 4;
    int T = 64 - W;
    int tot_B = B_full + B_left + B_right;
    int C = tot_B / 2;
    int needed_pairs = P - (K - tot_B);

    vector<uint64_t> b_domain(tot_B, 0), b_mask(tot_B, 0), b_bits(tot_B, 0);
    vector<PairInfo> pairs;
    vector<bool> is_paired(P, false);
    vector<int> pair_route_cost(P*P,1000000);

    auto try_pack = [&](int w_try,int bf,int bl,int br)->bool {
        if(design_elapsed()>pack_deadline)return false;
        W=w_try;T=64-W;tot_B=bf+bl+br;B_full=tot_B;B_left=B_right=0;
        C=0;needed_pairs=64+tot_B;
        const clock_t preprocessing_deadline=absolute_deadline(pack_deadline);
        const bool reuse=parent_design && parent_design->W<=W && parent_design->W+2>=W && parent_design->tot_B==tot_B;
        column_order=reuse?parent_design->column_order:choose_global_columns(original_catalog,tot_B,W,graph_seed_offset,preprocessing_deadline);

        if(!reuse && tot_B<=6 && W>tot_B*(tot_B-1) && graph_seed_offset!=0){
            // Retain separate deterministic metric samples for the two roots.
            uint64_t state=original_catalog[0]^(0x8dfb7af86ea1d759ULL*(graph_seed_offset+1));
            for(int j=63;j>0;j--){state^=state<<7;state^=state>>9;swap(column_order[j],column_order[unsigned(state)%(j+1)]);}
        }
        // Keep two independent starting points. A child inherits its parent's
        // physical columns and comparison pairs. To add a comparison window,
        // first price its direction against the parent's feasible product pairs.
        vector<pair<int,int>> inherited;
        if(reuse) {
            int oldT=parent_design->T;
            for(int j=oldT;j<N;++j) {
                int z=-1,o=-1;
                for(int k=0;k<tot_B;++k) {
                    char c=parent_design->S_init[k][j];
                    if(c=='0')z=k;
                    if(c=='1')o=k;
                }
                inherited.push_back({z,o});
            }
            while(oldT>T){
                int bestj=0,zbest=0,obest=1;double best=1e30;
                vector<array<int,7>> follows;
                for(const auto& p:parent_design->pairs){
                    array<int,7> req{};
                    uint64_t a=original_catalog[p.u],diff=a^original_catalog[p.v];
                    for(int j=oldT;j<N;++j)if(diff>>column_order[j]&1){
                        auto [z,o]=inherited[j-oldT];
                        if(a>>column_order[j]&1)swap(z,o);
                        req[z]|=1<<o;
                    }
                    for(int k=0;k<tot_B;++k)for(int h=0;h<tot_B;++h)if(req[h]>>k&1)req[h]|=req[k];
                    follows.push_back(req);
                }
                const auto cb=catalogue_column_penalty(original_catalog);
                for(int j=0;j<oldT;++j)for(int z=0;z<tot_B;++z)for(int o=0;o<tot_B;++o)if(z!=o){
                    double score=.06*cb[column_order[j]];
                    for(int idx=0;idx<int(follows.size());++idx){
                        const auto &p=parent_design->pairs[idx];
                        int a=original_catalog[p.u]>>column_order[j]&1,b=original_catalog[p.v]>>column_order[j]&1;
                        if(a==b)continue;
                        int up=a?o:z,down=a?z:o;const auto &g=follows[idx];
                        if(g[down]>>up&1)score+=10;
                        else if(g[up]>>down&1)score-=.8;
                        else score+=.5;
                    }
                    if(score<best){best=score;bestj=j;zbest=z;obest=o;}
                }
                int col=column_order[bestj];column_order.erase(column_order.begin()+bestj);
                column_order.push_back(col);inherited.push_back({zbest,obest});--oldT;
            }
        }
        local_A=permute_catalog_columns(original_catalog,column_order);
        const int metric_kind=3;
        const double metric_weight=4.0;
        const double control_weight=0.0;
        const int sample_count=128;
        const int restarts=1;
        const int SCALE=32;
        auto samples=metric_kind==3?graph_sample_orders(A[0],tot_B,sample_count):vector<array<int,16>>{};
        vector<PairInfo> chosen_pairs;vector<uint64_t> chosen_domains,chosen_masks,chosen_bits;
        int chosen_score=1000000000,chosen_restart=-1;
        for(int restart=0;restart<restarts;restart++) {
        if(restart && design_elapsed()>matching_deadline-.04)break;
        const bool full=true;
        bool use_data=!getenv("DATA_EDGES") || atoi(getenv("DATA_EDGES"));
        vector<pair<int,int>> gedges;
        if(reuse)gedges=inherited;
        else if(tot_B<=6 && W>tot_B*(tot_B-1)){
            bool ok=tot_B==5?CoreGenesis<5>::generate(original_catalog,column_order,gedges,W,graph_seed_offset,preprocessing_deadline):
                CoreGenesis<6>::generate(original_catalog,column_order,gedges,W,graph_seed_offset,preprocessing_deadline);
            if(!ok)return false;
            A=permute_catalog_columns(original_catalog,column_order);
        }else{
            gedges=use_data?make_data_edges(A,tot_B,W,restart+graph_seed_offset,preprocessing_deadline):
                make_comparison_edges(A[0]^(0x632be59bd9b4e019ULL*(restart+graph_seed_offset)),tot_B,W,full);
        }
        const int polarity=2;
        if(polarity==1)random_comparison_polarities(gedges,A[0]^(0x632be59bd9b4e019ULL*(restart+graph_seed_offset)));
        if(polarity==2 && !use_data)balance_comparison_polarities(gedges,tot_B);
        if(tot_B==5 || tot_B==6){
            // Alternating between the split and its comparisons permits a
            // previously rejected physical column to enter a modified map.
            const int passes=graph_seed_offset<2?1:2;
            auto refine=[&](auto count){
                constexpr int sheets=decltype(count)::value;
                for(int pass=0;pass<passes;pass++){
                    int seed=graph_seed_offset+31*pass;
                    refine_window_split<sheets>(original_catalog,column_order,gedges,W,seed,preprocessing_deadline,1024/passes);
                    fit_comparison_columns<sheets>(original_catalog,column_order,gedges,W,seed,preprocessing_deadline,2/passes);
                }
            };
            if(tot_B==5)refine(std::integral_constant<int,5>{});
            else refine(std::integral_constant<int,6>{});
            A=permute_catalog_columns(original_catalog,column_order);
        }
        else if(tot_B==7){
            refine_window_split<7>(original_catalog,column_order,gedges,W,graph_seed_offset,preprocessing_deadline);
            fit_comparison_columns<7>(original_catalog,column_order,gedges,W,graph_seed_offset,preprocessing_deadline);
            A=permute_catalog_columns(original_catalog,column_order);
        }
        nov8_flexible_edges(A,tot_B,W,restart+graph_seed_offset,gedges,preprocessing_deadline);
        b_domain.assign(tot_B,0);b_mask.assign(tot_B,0);b_bits.assign(tot_B,0);
        if(C>0)for(int b=0;b<tot_B;b++) {b_mask[b]=1ULL<<(b/2);if(b&1)b_bits[b]=b_mask[b];}
        for(int i=0;i<W;i++) {
            auto [zero,one]=gedges[i];uint64_t bit=1ULL<<(T+i);
            b_domain[zero]|=bit;b_domain[one]|=bit;b_bits[one]|=bit;
        }
        for(int b=0;b<tot_B;b++)b_mask[b]|=b_domain[b];
        // Complete the cheap feasibility graph first; optional metric work must not
        // discard the first constructible design or leave mixed edge-cost scales.
        const clock_t cheap_started=clock();
        vector<int> cost(P*P,1000000);
        vector<tuple<int,int,int>> cand;
        Blossom bm(P);uint16_t before[16];int edge_count=0;
        for(int u=0;u<P;u++)for(int v=u+1;v<P;v++) {
            uint64_t diff=A[u]^A[v];
            if(!(~diff&(((1ULL<<T)-1)^((1ULL<<C)-1))))continue;
            if(!comparison_pair_dag(A[u],A[v],tot_B,W,gedges,before,C))continue;
            int hd=__builtin_popcountll(diff&(((1ULL<<T)-1)^((1ULL<<C)-1)));
            int ctrl=__builtin_popcountll(diff&((1ULL<<C)-1));
            int c=llround(SCALE*(hd+control_weight*ctrl));
            cost[u*P+v]=cost[v*P+u]=c;
            cand.emplace_back(c,u,v);bm.g[u].push_back(v);bm.g[v].push_back(u);edge_count++;
        }
        sort(cand.begin(),cand.end());int count=0;
        for(auto [c,u,v]:cand)if(bm.match[u]<0&&bm.match[v]<0) {
            bm.match[u]=v;bm.match[v]=u;if(++count==needed_pairs)break;
        }
        if(count<needed_pairs)count=bm.solve();
        const int cheap_matching_count=count;
        const double cheap_cpu=double(clock()-cheap_started)/CLOCKS_PER_SEC;
        if(count<needed_pairs) {
            cerr<<"EXACT_COST W="<<W<<" complete=0 count=0 total="<<cand.size()
                <<" cheap_pairs="<<count<<" cheap_cpu="<<cheap_cpu<<" metric_cpu=0 reason=infeasible\n";
            continue;
        }
        // Keep the cheap matching intact while evaluating a separate full cost matrix.
        const clock_t metric_started=clock();
        vector<int> exact_cost=cost;size_t exact_count=0;
        // Local lifetime is important: every graph map has its own sampled ranks.
        std::unordered_map<uint64_t,double> metric_memo;
        metric_memo.reserve(cand.size());size_t metric_hits=0;
        auto memoized_metric=[&](const uint16_t* before) {
            if(tot_B>8)return graph_dag_metric(before,tot_B,metric_kind,samples);
            uint16_t closure[16];std::copy(before,before+tot_B,closure);
            for(int k=0;k<tot_B;k++)for(int v=0;v<tot_B;v++)
                if(closure[v]&(1u<<k))closure[v]|=closure[k];
            uint64_t key=0,reverse=0;
            for(int v=0;v<tot_B;v++) {
                key|=uint64_t(closure[v])<<(tot_B*v);
                unsigned row=closure[v];
                while(row){int u=__builtin_ctz(row);row&=row-1;reverse|=1ULL<<(tot_B*u+v);}
            }
            // samples are r, B-1-r pairs. Transposing the DAG exchanges these
            // samples' undirected inversion graphs, preserving the integer sum.
            key=std::min(key,reverse);
            auto found=metric_memo.find(key);
            if(found!=metric_memo.end()) {
                metric_hits++;
#ifdef NOV3_METRIC_VERIFY
                assert(found->second==graph_dag_metric(before,tot_B,metric_kind,samples));
#endif
                return found->second;
            }
            double value=graph_dag_metric(before,tot_B,metric_kind,samples);
            metric_memo.emplace(key,value);return value;
        };

        bool exact_complete=true;
        for(const auto &entry:cand) {
            if(construction_deadline_expired(preprocessing_deadline)){exact_complete=false;break;}
            auto [cheap,u,v]=entry;(void)cheap;
            bool dag_ok=comparison_pair_dag(A[u],A[v],tot_B,W,gedges,before,C);assert(dag_ok);
            uint64_t diff=A[u]^A[v];
            int hd=__builtin_popcountll(diff&(((1ULL<<T)-1)^((1ULL<<C)-1)));
            int ctrl=__builtin_popcountll(diff&((1ULL<<C)-1));
            double m=memoized_metric(before);
            int c=llround(SCALE*(hd+control_weight*ctrl+metric_weight*m));
            exact_cost[u*P+v]=exact_cost[v*P+u]=c;exact_count++;
        }
        if(exact_complete) {
            cost.swap(exact_cost);
            for(auto &entry:cand){auto &[c,u,v]=entry;c=cost[u*P+v];}
            sort(cand.begin(),cand.end());bm.match.assign(P,-1);count=0;
            for(auto [c,u,v]:cand)if(bm.match[u]<0&&bm.match[v]<0) {
                bm.match[u]=v;bm.match[v]=u;if(++count==needed_pairs)break;
            }
            if(count<needed_pairs)count=bm.solve();
            assert(count>=needed_pairs); // The graph is identical to the cheap pass.
        }
        cerr<<"EXACT_COST W="<<W<<" complete="<<int(exact_complete)<<" count="<<exact_count
            <<" total="<<cand.size()<<" cheap_pairs="<<cheap_matching_count<<" cheap_cpu="<<cheap_cpu
            <<" metric_cpu="<<double(clock()-metric_started)/CLOCKS_PER_SEC<<" metric_cache_hits="<<metric_hits<<" metric_unique="<<metric_memo.size()<<"\n";
        auto &mate=bm.match;
        if(count>needed_pairs) {
            vector<pair<int,int>> selected;
            for(int u=0;u<P;u++)if(mate[u]>u)selected.push_back({cost[u*P+mate[u]],u});
            sort(selected.rbegin(),selected.rend());
            for(int i=0;i<count-needed_pairs;i++){int u=selected[i].second,v=mate[u];mate[u]=mate[v]=-1;}
        }
        int score=0;for(int u=0;u<P;u++)if(mate[u]>u)score+=cost[u*P+mate[u]];
        auto greedy=[&]() {
            while(true) {
                if(design_elapsed()>matching_deadline)return;
                int bd=0,bu=-1,bv=-1,bx=-1,by=-1;
                for(int u=0;u<P;u++)if(mate[u]>u) {
                    int v=mate[u],old=cost[u*P+v];
                    for(int x=0;x<P;x++)if(mate[x]<0) {
                        int d=cost[u*P+x]-old;if(d<bd)bd=d,bu=u,bv=v,bx=x,by=-1;
                        d=cost[v*P+x]-old;if(d<bd)bd=d,bu=v,bv=u,bx=x,by=-1;
                    }else if(mate[x]>x&&x>u) {
                        int y=mate[x],old2=old+cost[x*P+y];
                        int d=cost[u*P+x]+cost[v*P+y]-old2;if(d<bd)bd=d,bu=u,bv=v,bx=x,by=y;
                        d=cost[u*P+y]+cost[v*P+x]-old2;if(d<bd)bd=d,bu=u,bv=v,bx=y,by=x;
                    }
                }
                if(bu<0)break;
                mate[bu]=bx;mate[bx]=bu;mate[bv]=by;if(by>=0)mate[by]=bv;score+=bd;
            }
        };
        greedy();
        uint64_t random_state=123456789ULL^A[0];
        auto random32=[&](){random_state^=random_state<<7;random_state^=random_state>>9;return uint32_t(random_state);};
        auto bestmate=mate;int bestscore=score;
        const int iterations=max(1,search_budget::integer("NOV8_MATCH_ITERS",200000));
        for(int it=0;it<iterations;it++) {
            if((it&1023)==0&&design_elapsed()>matching_deadline)break;
            int u=random32()%P;if(mate[u]<0)continue;
            int v=mate[u],x=random32()%P;if(x==u||x==v)continue;
            int y=mate[x];if(cost[u*P+x]>=1000000)continue;
            int d=cost[u*P+x]-cost[u*P+v];
            if(y>=0){if(cost[v*P+y]>=1000000)continue;d+=cost[v*P+y]-cost[x*P+y];}
            double temp=SCALE*(2*(1-double(it)/iterations)+.025);
            if(d<=0 || (random32()+.5)/4294967296.0<exp(-d/temp)) {
                mate[u]=x;mate[x]=u;mate[v]=y;if(y>=0)mate[y]=v;score+=d;
                if(score<bestscore)bestscore=score,bestmate=mate;
            }
        }
        mate=bestmate;score=bestscore;greedy();
        cerr<<"MATCH W="<<W<<" cost="<<double(score)/SCALE<<" feasible_edges="<<edge_count<<"\n";
        pairs.clear();is_paired.assign(P,false);
        for(int u=0;u<P;u++)if(mate[u]>u) {
            int v=mate[u];pairs.push_back({u,v,0,-1,1,-1,-1,-1});
            is_paired[u]=is_paired[v]=true;
        }
        if(score<chosen_score) {
            chosen_score=score;chosen_restart=restart;chosen_pairs=pairs;
            chosen_domains=b_domain;chosen_masks=b_mask;chosen_bits=b_bits;
            fill(pair_route_cost.begin(),pair_route_cost.end(),1000000);
            for(int u=0;u<P;u++)for(int v=u+1;v<P;v++)if(cost[u*P+v]<1000000) {
                int hd=__builtin_popcountll((A[u]^A[v])&(((1ULL<<T)-1)^((1ULL<<C)-1)));
                pair_route_cost[u*P+v]=pair_route_cost[v*P+u]=cost[u*P+v]-SCALE*hd;
            }
        }
        }
        if(chosen_restart<0)return false;
        pairs=move(chosen_pairs);b_domain=move(chosen_domains);b_mask=move(chosen_masks);b_bits=move(chosen_bits);
        is_paired.assign(P,false);for(const auto &pi:pairs)is_paired[pi.u]=is_paired[pi.v]=true;
        return true;
    };

    if(replay){
        W=replay->W;T=64-W;C=0;tot_B=replay->B;
        pairs=replay->pairs;b_domain=replay->domains;b_bits=replay->bits;b_mask=replay->masks;
        A=replay->a;column_order=replay->columns;pair_route_cost=replay->route;
        for(const auto &p:pairs)is_paired[p.u]=is_paired[p.v]=true;
    }else{
        bool packed=false;
        for(int w=pack_start;w>=20&&!packed;w-=2)packed=try_pack(w,pack_bf,pack_bl,pack_br);
        if(!packed)return nullopt;
        double price=0;
        for(const auto &p:pairs)
            price+=.7*__builtin_popcountll((A[p.u]^A[p.v])&((1ULL<<T)-1))+pair_route_cost[p.u*P+p.v]/32.;
        snapshots.push_back({W,tot_B,A,b_domain,b_mask,b_bits,column_order,pair_route_cost,pairs,price});
    }

    // Rebuild the staircase in a catalogue-dependent column order.
    // Prefix columns where the shared mains disagree less often come first.
    // Among equal disagreement counts, prefer neighbouring columns whose
    // short patterns repeat across products; these can share repair supports.
    // The backing suffix stays fixed, so its DAGs and route costs remain valid.
    if(T-C>=2) {
        vector<int> order,perm(64),new_columns(64);
        int disagree[64]={};
        for(const auto&pi:pairs)for(int j=C;j<T;j++)disagree[j]+=int((A[pi.u]^A[pi.v])>>j&1);
        for(int j=C;j<T;j++)order.push_back(j);
        stable_sort(order.begin(),order.end(),[&](int x,int y){return disagree[x]<disagree[y];});
        vector<unsigned> context(A.size(),0);
        int fixed=0;
        for(int at=int(order.size())-1;at>=0;at--,fixed++) {
            int left=at;
            while(left>0 && disagree[order[left-1]]==disagree[order[at]])left--;
            int chosen=at,best=-1;
            unsigned context_mask=(1u<<min(3,fixed))-1;
            // Reverse enumeration keeps the stable-sort choice on ties.
            for(int z=at;z>=left;z--) {
                int frequency[16]={};
                for(int a=0;a<int(A.size());a++)
                    frequency[((context[a]&context_mask)<<1)|unsigned(A[a]>>order[z]&1)]++;
                int score=0;for(int count:frequency)score+=count*count;
                if(score>best){best=score;chosen=z;}
            }
            int column=order[chosen];
            order.erase(order.begin()+chosen);order.insert(order.begin()+at,column);
            for(int a=0;a<int(A.size());a++)context[a]=((context[a]<<1)|unsigned(A[a]>>column&1))&7u;
        }
        if(replay) {
            // Later anchors are often exposed. A predictable color at such a
            // window can be left at the bucket head for many consecutive orders.
            int bias[64]={};double proximity[64][64]={};
            for(int i=C;i<T;i++){
                int ones=0;
                for(uint64_t a:A)ones+=int((a>>i)&1);
                bias[i]=abs(ones-P/2);
                for(int j=C;j<i;j++){
                    int different=0,both=0;
                    for(uint64_t a:A)different+=int(((a>>i)^(a>>j))&1);
                    for(const auto& p:pairs){
                        uint64_t d=A[p.u]^A[p.v];both+=int((d>>i)&(d>>j)&1);
                    }
                    double corr=(different-P/2)/8.;
                    proximity[i][j]=proximity[j][i]=.5*corr*corr
                        + .5*(both-double(disagree[i])*disagree[j]/pairs.size());
                }
            }
            constexpr double bw=1.4;
            auto price=[&](){
                double score=0;
                for(int i=0;i<int(order.size());i++){
                    int x=order[i];
                    double u=double(i)/(order.size()-1);
                    score+=u*disagree[x]+bw*(.4*u+.6*u*u*u*u)*bias[x];
                    for(int h=1;h<=3 && h<=i;h++)
                        score+=(.17+.30*u)*proximity[x][order[i-h]]/h;
                }
                return score;
            };
            uint64_t rng=A[0]^0x72174b675a49d313ULL^uint64_t(graph_seed_offset*1823u);
            auto next=[&](){rng^=rng<<7;rng^=rng>>9;return unsigned(rng);};
            double score=price();
            for(int it=0;it<1500;it++){
                int x=next()%order.size(),y=next()%order.size();if(x==y)continue;
                swap(order[x],order[y]);double value=price();
                if(value>score)score=value;
                else swap(order[x],order[y]);
            }
        }
        for(int j=0;j<64;j++)perm[j]=j;
        for(int j=C;j<T;j++)perm[j]=order[j-C];
        for(int j=0;j<64;j++)new_columns[j]=column_order[perm[j]];
        A=permute_catalog_columns(A,perm);column_order=move(new_columns);
        cerr<<"PREFIX_COLUMN_ORDER min="<<disagree[order.front()]<<" max="<<disagree[order.back()]<<"\n";
    }

    vector<int> singletons;
    for (int i = 0; i < P; ++i) if (!is_paired[i]) singletons.push_back(i);

    vector<vector<int>> bucket_pairs(T), bucket_sings(T);
    vector<int> cnt0(T, 0), cnt1(T, 0);

    for (int idx = 0; idx < (int)pairs.size(); ++idx) {
        int u = pairs[idx].u, v = pairs[idx].v;
        int best_w = -1, best_cost = 999999;
        for (int w = C; w < T; ++w) {
            int bu = (A[u] >> w) & 1;
            int bv = (A[v] >> w) & 1;
            if (bu != bv) continue;
            if ((int)bucket_pairs[w].size() >= 6) continue;
            uint64_t diff_prefix = (A[u] ^ A[v]) & ((1ULL << w) - 1);
            int holes = __builtin_popcountll(diff_prefix) + (T - 1 - w);
            int cost = (int)bucket_pairs[w].size() * 4 + holes * 2;
            if (bu == 0 && cnt0[w] < 2) cost -= 200;
            if (bu == 1 && cnt1[w] < 2) cost -= 200;
            if (cost < best_cost) { best_cost = cost; best_w = w; }
        }
        if (best_w < 0) return nullopt;
        pairs[idx].bucket_w = best_w;
        bucket_pairs[best_w].push_back(idx);
        int bit = (A[u] >> best_w) & 1;
        if (bit == 0) cnt0[best_w]++; else cnt1[best_w]++;
    }

    vector<bool> sing_used(singletons.size(), false);
    for (int pass = 1; pass <= 2; ++pass) {
        for (int w = C; w < T; ++w) {
            if (cnt0[w] < pass) {
                for (int si = 0; si < (int)singletons.size(); ++si) if (!sing_used[si]) {
                    int x = singletons[si];
                    if (((A[x] >> w) & 1) == 0) {
                        sing_used[si] = true; bucket_sings[w].push_back(x); cnt0[w]++; break;
                    }
                }
            }
            if (cnt1[w] < pass) {
                for (int si = 0; si < (int)singletons.size(); ++si) if (!sing_used[si]) {
                    int x = singletons[si];
                    if (((A[x] >> w) & 1) == 1) {
                        sing_used[si] = true; bucket_sings[w].push_back(x); cnt1[w]++; break;
                    }
                }
            }
        }
    }
    for (int si = 0; si < (int)singletons.size(); ++si) if (!sing_used[si]) {
        int x = singletons[si];
        int best_w = -1, best_sc = 999999;
        for (int w = C; w < T; ++w) {
            int cur_sz = bucket_pairs[w].size() + bucket_sings[w].size();
            int sc = cur_sz * 2 + (T - 1 - w);
            if (sc < best_sc) { best_sc = sc; best_w = w; }
        }
        sing_used[si] = true;
        bucket_sings[best_w].push_back(x);
    }

    if (tot_B + P - int(pairs.size()) > K) return nullopt;
    for (int w=C; w<T; ++w) {
        int colors=0;
        for (int pidx:bucket_pairs[w]) colors |= 1 << ((A[pairs[pidx].u]>>w)&1ULL);
        for (int x:bucket_sings[w]) colors |= 1 << ((A[x]>>w)&1ULL);
        if (colors != 3) return nullopt;
    }

    // Optimize the bucket assignment jointly for expected retrieval and repairs.
    {
        struct LItem { int type, id, weight; uint64_t bits, diff; };
        vector<LItem> li;
        vector<int> at;
        for(int w=C;w<T;w++) {
            for(int i:bucket_pairs[w]) {li.push_back({1,i,2,A[pairs[i].u],A[pairs[i].u]^A[pairs[i].v]});at.push_back(w);}
            for(int i:bucket_sings[w]) {li.push_back({0,i,1,A[i],0});at.push_back(w);}
        }
        const int n=li.size();
        // Colored windows of a main sheet are already right. Charge exposed
        // windows by how likely their requested color is to need a bucket flip.
        constexpr double background_bias = 0.70;
        uint64_t background = 0;
        for (int w = C; w < T; ++w) {
            int ones = 0;
            for (uint64_t bits : A) ones += (bits >> w) & 1;
            if (2 * ones > P || (2 * ones == P && ((A[0] >> w) & 1)))
                background |= uint64_t{1} << w;
        }

        // Charge the holes that actually remain expensive after support moves.
        // The outermost anchor windows and holes just below a main's anchor
        // require individual repairs more often in simulated order streams.
        using RowWeights = std::array<std::array<float, 256>, 8>;
        vector<RowWeights> prices(T);
        constexpr double curve_strength = 0.70;
        for (int w = C; w < T; ++w) {
            for (int j = C; j < T; ++j) {
                double effect = -0.12 + 0.75 * exp(-0.50 * (T-1-j))
                                      + 0.20 * exp(-0.33 * (j-C));
                if (j < w) effect += 0.34 * exp(-0.27 * (w-j));
                const double price = 1 + curve_strength * effect;
                auto& block = prices[w][j / 8];
                int bit = 1 << (j % 8);
                for (int subset = 0; subset < bit; ++subset)
                    block[subset | bit] = block[subset] + price;
            }
        }
        vector<RowWeights> zeros_price(T),ones_price(T);
        auto adjust_prices=[&](const double* incoming,const double* outgoing,double mix){
            for(int w=C;w<T;w++)for(int j=C;j<T;j++){
                double z=incoming[j],o=outgoing[j],p=o/(z+o);
                double b=(background>>j)&1;
                double oldz=1+background_bias*(2*b-1),oldo=1+background_bias*(1-2*b);
                double nz=(1-mix)*oldz+mix*(4*p*p),no=(1-mix)*oldo+mix*(4*(1-p)*(1-p));
                int bit=1<<(j%8);double cell=prices[w][j/8][bit];
                auto& zr=zeros_price[w][j/8];auto& orow=ones_price[w][j/8];
                for(int t=0;t<bit;t++){
                    zr[t|bit]=zr[t]+cell*nz;orow[t|bit]=orow[t]+cell*(no-nz);
                }
            }
        };
        double initial[64];fill(initial,initial+64,1.);adjust_prices(initial,initial,0.);
        auto exposure = [&](uint64_t bits, uint64_t diff, int weight, int w) {
            const uint64_t index_mask = (uint64_t{1} << T) - 1;
            const uint64_t printable = (uint64_t{1} << (w + 1)) - 1;
            const uint64_t exposed = index_mask & (~printable | diff);
            auto sum = [&](uint64_t mask,const RowWeights& row) {
                double value = row[0][mask & 255] + row[1][(mask >> 8) & 255]
                             + row[2][(mask >> 16) & 255];
                for (int j = 24; j < T; j += 8) value += row[j / 8][(mask >> j) & 255];
                return value;
            };
            double cost=weight*sum(exposed,zeros_price[w])+sum(exposed&bits,ones_price[w]);
            if(weight==2)cost+=sum(exposed&(bits^diff),ones_price[w]);
            return cost;
        };
        vector<vector<double>> hole(n,vector<double>(T,1e9));
        for(int x=0;x<n;x++)for(int w=C;w<T;w++) {
            if((li[x].diff>>w)&1)continue;
            hole[x][w]=exposure(li[x].bits,li[x].diff,li[x].weight,w);
        }
        vector<int> pc(T),sc(T),co0(T),co1(T);
        for(int x=0;x<n;x++){int w=at[x];(li[x].type?pc[w]:sc[w])++;(((li[x].bits>>w)&1)?co1[w]:co0[w])++;}
        double alpha=1.0,beta=.5;
        auto F=[&](int p,int s) {return alpha*(p*(p+1.0)+s*p+s*(s+1.0)/2);};
        double energy=0;
        for(int w=C;w<T;w++)energy+=F(pc[w],sc[w]);
        for(int x=0;x<n;x++)energy+=beta*hole[x][at[x]];
        double best=energy;auto best_at=at;
        uint64_t state=923741293^A[0];
        auto rng=[&](){state^=state<<7;state^=state>>9;return state;};
        int iterations=max(1,search_budget::integer("NOV8_ANCHOR_ITERS",200000));
        for(int it=0;it<iterations;it++) {
            if ((it & 1023) == 0 && design_elapsed() > layout_deadline) break;
            double temp=4.0*pow(.005,double(it)/iterations);
            int x=rng()%n,u=at[x],v=C+rng()%(T-C),y=-1;
            bool swap=(rng()%2)==0;
            if(swap){y=rng()%n;v=at[y];}
            if(u==v||hole[x][v]>1e8||(swap&&hole[y][u]>1e8))continue;
            int cxu=(li[x].bits>>u)&1,cxv=(li[x].bits>>v)&1;
            int cyu=swap?((li[y].bits>>u)&1):0,cyv=swap?((li[y].bits>>v)&1):0;
            int z0u=co0[u]-(cxu==0)+(swap&&cyu==0),z1u=co1[u]-(cxu==1)+(swap&&cyu==1);
            int z0v=co0[v]+(cxv==0)-(swap&&cyv==0),z1v=co1[v]+(cxv==1)-(swap&&cyv==1);
            if(z0u<1||z1u<1||z0v<1||z1v<1)continue;
            int pu=pc[u]-li[x].type+(swap?li[y].type:0),su=sc[u]-(1-li[x].type)+(swap?1-li[y].type:0);
            int pv=pc[v]+li[x].type-(swap?li[y].type:0),sv=sc[v]+(1-li[x].type)-(swap?1-li[y].type:0);
            double delta=F(pu,su)+F(pv,sv)-F(pc[u],sc[u])-F(pc[v],sc[v])+beta*(hole[x][v]-hole[x][u]);
            if(swap)delta+=beta*(hole[y][u]-hole[y][v]);
            if(delta<=0||double(rng()>>11)*0x1.0p-53<exp(-delta/temp)) {
                at[x]=v;if(swap)at[y]=u;
                pc[u]=pu;sc[u]=su;pc[v]=pv;sc[v]=sv;co0[u]=z0u;co1[u]=z1u;co0[v]=z0v;co1[v]=z1v;
                energy+=delta;
                if(energy<best){best=energy;best_at=at;}
            }
        }
        // Among orders which expose this window, equal successive colors
        // can reuse the bucket head. With zero/one demands z,o the expected
        // flip count is 2*z*o/(z+o). Fit its marginal costs to this design.
        {
            double z[64]{},o[64]{};
            for(int j=C;j<T;j++){z[j]=o[j]=4.;}
            for(int x=0;x<n;x++){
                auto item=li[x];int w=best_at[x];
                for(int j=C;j<T;j++)if(j>w || (item.diff>>j&1)){
                    double delta=prices[w][j/8][1<<(j%8)];
                    ((item.bits>>j&1)?o[j]:z[j])+=delta;
                    if(item.type)(((item.bits^item.diff)>>j&1)?o[j]:z[j])+=delta;
                }
            }
            if(replay || graph_seed_offset>=2)adjust_prices(z,o,1.25);
        }
        // Jointly change pair endpoints and the anchors of main slots.
        {
            const int joint_iters=max(1,search_budget::integer("NOV8_PAIR_ITERS",300000));
            const double route_scale=1.0;
            const double cap=search_budget::seconds(search_budget::real("NOV8_PAIR_CAP",.08));
            const double initial_temp=1.0;
            const clock_t joint_started=clock();
            struct JSlot {int type,id,a,b,w;};
            vector<JSlot> slots(n);vector<int> pair_slots,owner(P,-1);
            vector<vector<int>> adj(P);
            const uint64_t imask=((1ULL<<T)-1)^((1ULL<<C)-1);
            for(int a=0;a<P;a++)for(int b=0;b<P;b++)if(pair_route_cost[a*P+b]<1000000)adj[a].push_back(b);
            vector<int> jp(T),js(T),j0(T),j1(T);
            for(int x=0;x<n;x++) {
                if(li[x].type) {
                    auto &p=pairs[li[x].id];slots[x]={1,li[x].id,p.u,p.v,best_at[x]};pair_slots.push_back(x);
                }else slots[x]={0,li[x].id,li[x].id,-1,best_at[x]};
                auto &s=slots[x];owner[s.a]=x;if(s.b>=0)owner[s.b]=x;
                (s.type?jp[s.w]:js[s.w])++;(((A[s.a]>>s.w)&1)?j1[s.w]:j0[s.w])++;
            }
            auto cost=[&](const JSlot &s) {
                if(!s.type)return beta*exposure(A[s.a],0,1,s.w);
                return beta*exposure(A[s.a],A[s.a]^A[s.b],2,s.w)
                    +route_scale*pair_route_cost[s.a*P+s.b]/32.0;
            };
            auto feasible=[&](const JSlot &s) {
                return !s.type || (pair_route_cost[s.a*P+s.b]<1000000 && !(((A[s.a]^A[s.b])>>s.w)&1));
            };
            uint64_t state=A[0]^(0x9bb149d76ca1e705ULL+0x632be59bd9b4e019ULL*graph_seed_offset);
            if(!state)state=0x9bb149d76ca1e705ULL;
            auto rand32=[&](){state^=state<<7;state^=state>>9;return uint32_t(state);};
            auto pick_anchor=[&](const JSlot &s) {
                uint64_t allowed=imask;if(s.type)allowed&=~(A[s.a]^A[s.b]);
                if(!allowed)return -1;
                int nth=rand32()%__builtin_popcountll(allowed);
                while(nth--)allowed&=allowed-1;
                return __builtin_ctzll(allowed);
            };
            auto front_cost = [](int pairs, int one_pairs, int zeros, int ones) {
                // Keep both colors available near the head. An intervening
                // single costs one additional step per displaced pair.
                const bool one_sided = pairs > 0 && (one_pairs == 0 || one_pairs == pairs);
                double loss = one_sided ? pairs - 0.5 : 0.0;
                loss += 0.5 * max(0, 2 - min(zeros, ones)) * max(0, zeros + ones - 3);
                return loss;
            };
            constexpr double head_weight = 1.0;
            vector<int> pair_ones(T);
            for (const auto& item:slots)
                if (item.type) pair_ones[item.w] += (A[item.a]>>item.w)&1;
            double energy=0;for(int w=C;w<T;w++)energy+=F(jp[w],js[w])+head_weight*front_cost(jp[w],pair_ones[w],j0[w],j1[w]);for(auto &s:slots)energy+=cost(s);
            double best_energy=energy;auto best_slots=slots;
            for(int it=0;it<joint_iters && n>0;it++) {
                if((it&1023)==0 && (design_elapsed()>layout_deadline || double(clock()-joint_started)/CLOCKS_PER_SEC>cap))break;
                int kind=rand32()%4,x,y=-1;JSlot nx{},ny{};
                bool endpoint=kind<2;
                if(endpoint) {
                    if(pair_slots.empty())continue;
                    x=pair_slots[rand32()%pair_slots.size()];nx=slots[x];
                    int side=rand32()%2,partner=side?nx.a:nx.b,old=side?nx.b:nx.a;
                    if(adj[partner].empty())continue;
                    int replacement=adj[partner][rand32()%adj[partner].size()];y=owner[replacement];
                    if(y==x)continue;
                    ny=slots[y];
                    if(side)nx.b=replacement;else nx.a=replacement;
                    if(ny.a==replacement)ny.a=old;else ny.b=old;
                    if(pair_route_cost[nx.a*P+nx.b]>=1000000 || (ny.type && pair_route_cost[ny.a*P+ny.b]>=1000000))continue;
                    if(!feasible(nx)||(rand32()&3)==0)nx.w=pick_anchor(nx);
                    if(!feasible(ny)||(rand32()&3)==0)ny.w=pick_anchor(ny);
                    if(nx.w<0||ny.w<0)continue;
                }else {
                    x=rand32()%n;nx=slots[x];
                    if(kind==2) {nx.w=pick_anchor(nx);if(nx.w<0||nx.w==slots[x].w)continue;}
                    else {y=rand32()%n;if(y==x)continue;
                    ny=slots[y];swap(nx.w,ny.w);if(nx.w==slots[x].w)continue;}
                }
                if(!feasible(nx)||(y>=0&&!feasible(ny)))continue;
                int bw[4],dpc[4]={},dsc[4]={},dc0[4]={},dc1[4]={},dp1[4]={},nb=0;
                auto change_bucket=[&](const JSlot &s,int sign) {
                    int k=0;while(k<nb&&bw[k]!=s.w)k++;
                    if(k==nb)bw[nb++]=s.w;
                    (s.type?dpc[k]:dsc[k])+=sign; if(s.type && (A[s.a]>>s.w&1))dp1[k]+=sign;
                    (((A[s.a]>>s.w)&1)?dc1[k]:dc0[k])+=sign;
                };
                change_bucket(slots[x],-1);change_bucket(nx,1);
                if(y>=0){change_bucket(slots[y],-1);change_bucket(ny,1);}
                double delta=cost(nx)-cost(slots[x]);if(y>=0)delta+=cost(ny)-cost(slots[y]);
                bool valid=true;
                for(int k=0;k<nb;k++) {
                    int w=bw[k];if(j0[w]+dc0[k]<1||j1[w]+dc1[k]<1){valid=false;break;}
                    delta+=F(jp[w]+dpc[k],js[w]+dsc[k])-F(jp[w],js[w]);
                    delta+=head_weight*(front_cost(jp[w]+dpc[k],pair_ones[w]+dp1[k],j0[w]+dc0[k],j1[w]+dc1[k])-front_cost(jp[w],pair_ones[w],j0[w],j1[w]));
                }
                if(!valid)continue;
                double temp=initial_temp*pow(.01,double(it)/max(1,joint_iters));
                if(delta>0&&(rand32()+.5)/4294967296.0>=exp(-delta/temp))continue;
                slots[x]=nx;owner[nx.a]=x;if(nx.b>=0)owner[nx.b]=x;
                if(y>=0){slots[y]=ny;owner[ny.a]=y;if(ny.b>=0)owner[ny.b]=y;}
                for(int k=0;k<nb;k++){int w=bw[k];jp[w]+=dpc[k];pair_ones[w]+=dp1[k];js[w]+=dsc[k];j0[w]+=dc0[k];j1[w]+=dc1[k];}
                energy+=delta;
                if(energy<best_energy-1e-8) {best_energy=energy;best_slots=slots;}
            }
            // Alternate a priced product matching with an exact assignment
            // of complete papers to the anchor slots.
            if (design_elapsed() < layout_deadline) {
                constexpr double inf=1.e6;
                struct Solution { vector<int> row;vector<double> price;double objective=0; };
                auto assign=[&](const vector<double>& mat,int n)->Solution{
                    vector<double> u(n+1),v(n+1),remaining(n+1);
                    vector<int> owner(n+1),way(n+1),pool(n),visited(n+1);
                    for(int i=1;i<=n;i++){
                        int j0=0,live=n,used=0;owner[0]=i;
                        for(int j=1;j<=n;j++){pool[j-1]=j;remaining[j]=1.e12;}
                        do{
                            visited[used++]=j0;int a=owner[j0],next=0;double least=1.e12;
                            const double *row=&mat[(a-1)*n];
                            for(int k=0;k<live;k++){
                                int j=pool[k];double d=row[j-1]-u[a]-v[j];
                                if(d<remaining[j]){remaining[j]=d;way[j]=j0;}
                                if(remaining[j]<least){least=remaining[j];next=k;}
                            }
                            for(int k=0;k<used;k++){int j=visited[k];u[owner[j]]+=least;v[j]-=least;}
                            for(int k=0;k<live;k++)remaining[pool[k]]-=least;
                            j0=pool[next];pool[next]=pool[--live];
                        }while(owner[j0]);
                        do{int j=way[j0];owner[j0]=owner[j];j0=j;}while(j0);
                    }
                    Solution out;out.row.resize(n);out.price.resize(n);
                    for(int j=1;j<=n;j++){
                        out.row[j-1]=owner[j]-1;out.price[j-1]=v[j];
                        out.objective+=mat[(owner[j]-1)*n+j-1];
                    }
                    return out;
                };
                vector<array<float,64>> unit(P),duo;
                vector<int> eindex(P*P,-1);
                vector<pair<int,int>> edges;
                for(int a=0;a<P;a++){
                    for(int w=C;w<T;w++)unit[a][w]=beta*exposure(A[a],0,1,w);
                    for(int b:adj[a])if(a<b){
                        int id=duo.size();duo.emplace_back();edges.push_back({a,b});
                        eindex[a*P+b]=eindex[b*P+a]=id;
                        for(int w=C;w<T;w++)duo[id][w]=beta*exposure(A[a],A[a]^A[b],2,w)+route_scale*pair_route_cost[a*P+b]/32.;
                    }
                }
                vector<int> which[2],required(n,-1);
                int counts[2][64][2]={};
                for(int x=0;x<n;x++){auto &p=best_slots[x];int c=(A[p.a]>>p.w)&1;which[p.type].push_back(x);counts[p.type][p.w][c]++;}
                for(int x=0;x<n;x++){auto &p=best_slots[x];int c=(A[p.a]>>p.w)&1;
                    int &quantity=counts[p.type][p.w][c];if(quantity<=2)required[x]=c;quantity--;}
                double slotprice[2][64][2]{};
                auto place=[&](const vector<pair<int,int>> (&items)[2],vector<JSlot>& result,double (&prices2)[2][64][2]){
                    result=best_slots;double objective=0;
                    for(int t=0;t<2;t++){
                        for(int w=C;w<T;w++)for(int c=0;c<2;c++)prices2[t][w][c]=-inf;
                        int m=which[t].size();vector<double> mat(m*m);
                        for(int i=0;i<m;i++){
                            auto [a,b]=items[t][i];
                            for(int j=0;j<m;j++){
                                int x=which[t][j],w=best_slots[x].w,c=(A[a]>>w)&1;
                                mat[i*m+j]=(required[x]>=0&&required[x]!=c)||(t&&((A[a]^A[b])>>w&1))?inf:
                                    t?duo[eindex[a*P+b]][w]:unit[a][w];
                            }
                        }
                        auto sol=assign(mat,m);objective+=sol.objective;
                        if(sol.objective>inf*.5)return inf;
                        for(int j=0;j<m;j++){
                            int x=which[t][j];auto [a,b]=items[t][sol.row[j]];
                            result[x].a=a;result[x].b=b;
                            for(int c=0;c<2;c++)if(required[x]<0 || required[x]==c)
                                prices2[t][result[x].w][c]=max(prices2[t][result[x].w][c],sol.price[j]);
                        }
                    }
                    int p[64]={},o[64]={},z[64]={},po[64]={};
                    for(auto &v:result){int c=(A[v.a]>>v.w)&1;(c?o[v.w]:z[v.w])++;p[v.w]+=v.type;po[v.w]+=c*v.type;}
                    for(int w=C;w<T;w++)objective+=head_weight*front_cost(p[w],po[w],z[w],o[w]);
                    return objective;
                };
                vector<pair<int,int>> items[2];for(auto &p:best_slots)items[p.type].push_back({p.a,p.b});
                vector<JSlot> placed;
                double best=place(items,placed,slotprice);if(best<inf/2)best_slots=placed;
                constexpr int rounds=4;
                constexpr double penalty=1.5;
                for(int step=0;step<rounds && best<inf/2;step++){
                    if(design_elapsed()>layout_deadline-.004)break;
                    vector<int> left,right,singles,old_mate(P,-1);
                    for(auto &p:best_slots)if(p.type){
                        int a=p.a,b=p.b;if(rand32()&1)swap(a,b);
                        left.push_back(a);right.push_back(b);old_mate[a]=b;old_mate[b]=a;
                    }else singles.push_back(p.a);
                    for(int i=singles.size()-1;i>0;i--)swap(singles[i],singles[rand32()%(i+1)]);
                    for(int i=0;i<int(singles.size());i++)(i&1?right:left).push_back(singles[i]);
                    int m=left.size();
                    vector<double> charge(P,inf),paircharge(edges.size(),inf);
                    for(int a=0;a<P;a++)for(int w=C;w<T;w++)
                        charge[a]=min(charge[a],unit[a][w]-slotprice[0][w][A[a]>>w&1]);
                    for(int i=0;i<int(edges.size());i++){
                        auto [a,b]=edges[i];
                        for(int w=C;w<T;w++)if(!((A[a]^A[b])>>w&1))
                            paircharge[i]=min(paircharge[i],duo[i][w]-slotprice[1][w][A[a]>>w&1]);
                    }
                    if(replay){
                        // Keep the current matching across the cut, then swap
                        // orientations to expose more inexpensive replacement edges.
                        array<int,P> side{};for(int v:right)side[v]=1;
                        vector<vector<pair<int,double>>> connected(P);
                        vector<double> reference(P,inf);
                        auto weight=[&](int i){auto[a,b]=edges[i];return paircharge[i]-charge[a]-charge[b];};
                        for(int i=0;i<int(edges.size());i++)if(paircharge[i]<inf/2){
                            auto[a,b]=edges[i];double w=weight(i);reference[a]=min(reference[a],w);reference[b]=min(reference[b],w);
                        }
                        for(int i=0;i<int(edges.size());i++)if(paircharge[i]<inf/2){
                            auto[a,b]=edges[i];if(old_mate[a]==b)continue;
                            double w=.2+1/(1+.1*(2*weight(i)-reference[a]-reference[b]));
                            connected[a].emplace_back(b,w);connected[b].emplace_back(a,w);
                        }
                        int locked=pair_slots.size();
                        for(int sweep=0;sweep<4;sweep++)for(int j=0;j<m;j++){
                            int x=rand32()%m,y=x<locked?x:locked+rand32()%(m-locked);
                            int a=left[x],b=right[y];double gain=0;
                            for(auto[v,w]:connected[a])if(v!=b)gain+=(side[v]==0?w:-w);
                            for(auto[v,w]:connected[b])if(v!=a)gain+=(side[v]==1?w:-w);
                            if(gain>0){swap(left[x],right[y]);side[a]=1;side[b]=0;}
                        }
                    }
                    vector<int> right_id(P,-1);
                    for(int j=0;j<m;j++)right_id[right[j]]=j;
                    vector<vector<SparsePairEdge>> crossing(m);
                    for(int i=0;i<m;i++){
                        int a=left[i];
                        for(int b:adj[a])if(right_id[b]>=0){
                            int e=eindex[a*P+b];if(paircharge[e]>=inf*.5)continue;
                            double cost=clamp(paircharge[e]-charge[a]-charge[b]+(old_mate[a]!=b)*penalty,-1000.,1000.);
                            crossing[i].push_back({right_id[b],cost});
                        }
                    }
                    vector<int> owner;
                    if(!pair_flow(crossing,int(pair_slots.size()),owner))continue;
#ifdef PAIR_FLOW_VERIFY
                    {
                        int N=2*m-int(pair_slots.size());
                        vector<double> ref(N*N,0);
                        for(int i=0;i<m;i++){
                            fill(ref.begin()+i*N,ref.begin()+i*N+m,inf);
                            for(auto e:crossing[i])ref[i*N+e.right]=e.cost;
                        }
                        for(int i=m;i<N;i++)fill(ref.begin()+i*N+m,ref.begin()+(i+1)*N,inf);
                        double value=0;for(int j=0;j<m;j++)if(owner[j]>=0)value+=ref[owner[j]*N+j];
                        assert(abs(value-assign(ref,N).objective)<1e-5);
                    }
#endif
                    items[0].clear();items[1].clear();vector<bool> matched(P);
                    for(int j=0;j<m;j++)if(owner[j]>=0){
                        int a=left[owner[j]],b=right[j];items[1].push_back({a,b});matched[a]=matched[b]=true;
                    }
                    for(int a=0;a<P;a++)if(!matched[a])items[0].push_back({a,-1});
                    if(items[1].size()!=pair_slots.size())continue;
                    if(std::any_of(items[1].begin(),items[1].end(),[&](auto edge){
                        return eindex[edge.first*P+edge.second]<0;
                    }))continue;
                    double newprice[2][64][2]{},objective=place(items,placed,newprice);
                    if(objective<best-.00001){
                        best=objective;best_slots=placed;
                        for(int t=0;t<2;t++)for(int w=C;w<T;w++)for(int c=0;c<2;c++)slotprice[t][w][c]=newprice[t][w][c];
                    }
                }
            }
            for(int x=0;x<n;x++) {
                const auto &s=best_slots[x];best_at[x]=s.w;
                if(s.type){pairs[s.id].u=s.a;pairs[s.id].v=s.b;li[x].bits=A[s.a];li[x].diff=A[s.a]^A[s.b];}
                else {li[x].id=s.a;li[x].bits=A[s.a];}
                for(int w=C;w<T;w++)hole[x][w]=((li[x].diff>>w)&1)?1e9:
                    exposure(li[x].bits,li[x].diff,li[x].weight,w);
            }
        }

        // Average over possible bucket heads instead of fitting a small set of
        // sampled head identities. Each anchor color has stationary mass 1/2.
        const double joint_gamma=1.0;
        const int joint_iterations=max(0,search_budget::integer("NOV8_SUPPORT_ITERS",3000));
        if(joint_gamma>0 && joint_iterations>0 && design_elapsed()<layout_deadline-.03) {
            const clock_t joint_start=clock();
            const double joint_cap=search_budget::seconds(search_budget::real("NOV8_SUPPORT_CAP",.10));
            const int L=6;
            double tail[65][L];
            for(int g=0;g<=64;g++) {
                double prob=ldexp(1.0,-g);for(int l=0;l<L;l++)tail[g][l]=0;
                for(int k=0;k<=g;k++) {
                    for(int l=0;l<L;l++)if(k>=l+3)tail[g][l]+=prob;
                    if(k<g)prob*=double(g-k)/(k+1);
                }
            }
            at=best_at;
            fill(pc.begin(),pc.end(),0);fill(sc.begin(),sc.end(),0);
            fill(co0.begin(),co0.end(),0);fill(co1.begin(),co1.end(),0);
            vector<int> cw0(T),cw1(T);
            for(int x=0;x<n;x++) {
                int w=at[x],color=(li[x].bits>>w)&1;
                (li[x].type?pc[w]:sc[w])++;(color?co1[w]:co0[w])++;
                (color?cw1[w]:cw0[w])+=li[x].weight;
            }
            double base_energy=0;
            for(int w=C;w<T;w++)base_energy+=F(pc[w],sc[w]);
            for(int x=0;x<n;x++)base_energy+=beta*hole[x][at[x]];
            vector<int> owner(P,-1);
            vector<vector<int>> products(n);
            for(int x=0;x<n;x++) {
                if(li[x].type)products[x]={pairs[li[x].id].u,pairs[li[x].id].v};
                else products[x]={li[x].id};
                for(int a:products[x])owner[a]=x;
            }
            const uint64_t imask=((1ULL<<T)-1)^((1ULL<<C)-1);
            vector<uint64_t> prefix(T),hm(P);
            for(int w=C;w<T;w++)prefix[w]=((1ULL<<(w+1))-1)&imask;
            auto hole_mask=[&](int a){int x=owner[a];return imask&~(prefix[at[x]]&~li[x].diff);};
            for(int a=0;a<P;a++)hm[a]=hole_mask(a);
            auto coverage=[&](int a,int s,int w) {
                if(!(hm[a]&(1ULL<<w)))return 0;
                uint64_t op=prefix[w]&~li[s].diff;
                uint64_t bad=(li[s].bits^A[a])&op&hm[a];
                int cut=bad?63-__builtin_clzll(bad):C-1;
                if(cut>=w)return 0;
                uint64_t cover=op&hm[a]&(~0ULL<<(cut+1));
                // This paper is already the front of its bucket. Its own
                // anchor is correct before promotion, and gives no repair gain.
                cover&=~(1ULL<<w);
                return __builtin_popcountll(cover);
            };
            vector<double> sums(P*T*L),probs(P*L),next_sums(P*T*L),next_probs(P*L);
            auto row_build=[&](int a,vector<double>& ss,vector<double>& qq,const vector<int>& w0,const vector<int>& w1) {
                fill(ss.begin()+a*T*L,ss.begin()+(a+1)*T*L,0.0);
                for(int s=0;s<n;s++) {
                    int w=at[s],g=coverage(a,s,w);if(g<3)continue;
                    for(int l=0;l<L;l++)ss[(a*T+w)*L+l]+=li[s].weight*tail[g][l];
                }
                for(int l=0;l<L;l++) {
                    double pnone=1;
                    for(int w=C;w<T;w++) {
                        int denom=2*(((A[a]>>w)&1)?w1[w]:w0[w]);
                        pnone*=1-ss[(a*T+w)*L+l]/denom;
                    }
                    qq[a*L+l]=pnone;
                }
            };
            double gain_sum=0;
            for(int a=0;a<P;a++){row_build(a,sums,probs,cw0,cw1);for(int l=0;l<L;l++)gain_sum+=1-probs[a*L+l];}
            double joint_energy=base_energy-joint_gamma*gain_sum;
            double joint_best=joint_energy;
            auto joint_best_at=at;
            vector<bool> target_changed(P);
            for(int it=0;it<joint_iterations;it++) {
                if((it&63)==0 && (design_elapsed()>layout_deadline || double(clock()-joint_start)/CLOCKS_PER_SEC>joint_cap))break;
                int x=rng()%n,u=at[x],v=C+rng()%(T-C),y=-1;
                bool exchange=(rng()%2)==0;
                if(exchange){y=rng()%n;v=at[y];}
                if(u==v||hole[x][v]>1e8||(exchange&&hole[y][u]>1e8))continue;
                int cxu=(li[x].bits>>u)&1,cxv=(li[x].bits>>v)&1;
                int cyu=exchange?((li[y].bits>>u)&1):0,cyv=exchange?((li[y].bits>>v)&1):0;
                int z0u=co0[u]-(cxu==0)+(exchange&&cyu==0),z1u=co1[u]-(cxu==1)+(exchange&&cyu==1);
                int z0v=co0[v]+(cxv==0)-(exchange&&cyv==0),z1v=co1[v]+(cxv==1)-(exchange&&cyv==1);
                if(z0u<1||z1u<1||z0v<1||z1v<1)continue;
                int pu=pc[u]-li[x].type+(exchange?li[y].type:0),su=sc[u]-(1-li[x].type)+(exchange?1-li[y].type:0);
                int pv=pc[v]+li[x].type-(exchange?li[y].type:0),sv=sc[v]+(1-li[x].type)-(exchange?1-li[y].type:0);
                double delta_base=F(pu,su)+F(pv,sv)-F(pc[u],sc[u])-F(pc[v],sc[v])+beta*(hole[x][v]-hole[x][u]);
                if(exchange)delta_base+=beta*(hole[y][u]-hole[y][v]);
                vector<int> nw0=cw0,nw1=cw1;
                nw0[u]-=li[x].weight*(cxu==0);nw1[u]-=li[x].weight*(cxu==1);
                nw0[v]+=li[x].weight*(cxv==0);nw1[v]+=li[x].weight*(cxv==1);
                if(exchange) {
                    nw0[v]-=li[y].weight*(cyv==0);nw1[v]-=li[y].weight*(cyv==1);
                    nw0[u]+=li[y].weight*(cyu==0);nw1[u]+=li[y].weight*(cyu==1);
                }
                at[x]=v;if(exchange)at[y]=u;
                vector<int> changed_products=products[x];
                if(exchange)changed_products.insert(changed_products.end(),products[y].begin(),products[y].end());
                fill(target_changed.begin(),target_changed.end(),false);
                for(int a:changed_products){hm[a]=hole_mask(a);target_changed[a]=true;}
                double delta_gain=0;
                for(int a=0;a<P;a++) {
                    if(target_changed[a])row_build(a,next_sums,next_probs,nw0,nw1);
                    else {
                        int old_x=coverage(a,x,u),new_x=coverage(a,x,v);
                        int old_y=exchange?coverage(a,y,v):0,new_y=exchange?coverage(a,y,u):0;
                        int du=2*(((A[a]>>u)&1)?cw1[u]:cw0[u]);
                        int dv=2*(((A[a]>>v)&1)?cw1[v]:cw0[v]);
                        int ndu=2*(((A[a]>>u)&1)?nw1[u]:nw0[u]);
                        int ndv=2*(((A[a]>>v)&1)?nw1[v]:nw0[v]);
                        for(int l=0;l<L;l++) {
                            int iu=(a*T+u)*L+l,iv=(a*T+v)*L+l;
                            double su2=sums[iu]+li[x].weight*(-tail[old_x][l]);
                            double sv2=sums[iv]+li[x].weight*tail[new_x][l];
                            if(exchange){su2+=li[y].weight*tail[new_y][l];sv2-=li[y].weight*tail[old_y][l];}
                            next_sums[iu]=su2;next_sums[iv]=sv2;
                            double qu=1-sums[iu]/du,qv=1-sums[iv]/dv;
                            double nqu=1-su2/ndu,nqv=1-sv2/ndv;
                            next_probs[a*L+l]=probs[a*L+l]*(nqu/qu)*(nqv/qv);
                        }
                    }
                    for(int l=0;l<L;l++)delta_gain+=probs[a*L+l]-next_probs[a*L+l];
                }
                double delta=delta_base-joint_gamma*delta_gain;
                double temp=.25*pow(.01,double(it)/joint_iterations);
                if(delta<=0||double(rng()>>11)*0x1.0p-53<exp(-delta/temp)) {
                    pc[u]=pu;sc[u]=su;pc[v]=pv;sc[v]=sv;co0[u]=z0u;co1[u]=z1u;co0[v]=z0v;co1[v]=z1v;
                    cw0=move(nw0);cw1=move(nw1);
                    base_energy+=delta_base;gain_sum+=delta_gain;joint_energy+=delta;
                    probs=next_probs;
                    for(int a=0;a<P;a++) {
                        if(target_changed[a])copy(next_sums.begin()+a*T*L,next_sums.begin()+(a+1)*T*L,sums.begin()+a*T*L);
                        else for(int l=0;l<L;l++){int iu=(a*T+u)*L+l,iv=(a*T+v)*L+l;sums[iu]=next_sums[iu];sums[iv]=next_sums[iv];}
                    }
                    if(joint_energy<joint_best){joint_best=joint_energy;joint_best_at=at;}
                } else {
                    at[x]=u;if(exchange)at[y]=v;
                    for(int a:changed_products)hm[a]=hole_mask(a);
                }
            }
            best_at=joint_best_at;
        }

        for(int w=C;w<T;w++){bucket_pairs[w].clear();bucket_sings[w].clear();}
        for(int x=0;x<n;x++) {
            int w=best_at[x];
            if(li[x].type){bucket_pairs[w].push_back(li[x].id);pairs[li[x].id].bucket_w=w;}
            else bucket_sings[w].push_back(li[x].id);
        }
    }

    vector<string> S_init(K, string(N, '.'));
    for (int b = 0; b < tot_B; ++b) {
        int c = b / 2;
        if(C>0)S_init[b][c] = (b % 2 == 0) ? '0' : '1';
        for (int j = T; j < N; ++j) {
            if ((b_domain[b] >> j) & 1) {
                if ((b_mask[b] >> j) & 1) S_init[b][j] = ((b_bits[b] >> j) & 1) ? '1' : '0';
                else S_init[b][j] = '0';
            }
        }
    }

    vector<int> prod_to_paper(P, -1);
    vector<int> prod_to_pair_idx(P, -1);
    vector<int> paper_bucket(K, -1);
    vector<bool> paper_is_paired(K, false);
    for (int b = 0; b < tot_B; ++b) paper_bucket[b] = C>0?b/2:-1;

    int cur_pid = tot_B;
    for (int w = C; w < T; ++w) {
        struct Item { int is_p; int id; int bit_w; };
        vector<Item> items;
        for (int pidx : bucket_pairs[w]) {
            int u = pairs[pidx].u;
            items.push_back({1, pidx, (int)((A[u] >> w) & 1)});
        }
        for (int x : bucket_sings[w]) {
            items.push_back({0, x, (int)((A[x] >> w) & 1)});
        }
        for (int i = 1; i < (int)items.size(); ++i) {
            if (items[i].bit_w != items[0].bit_w) {
                Item tmp = items[i];
                items.erase(items.begin() + i);
                items.insert(items.begin() + 1, tmp);
                break;
            }
        }
        for (auto& it : items) {
            if (cur_pid >= K) return nullopt;
            int pid = cur_pid++;
            paper_bucket[pid] = w;
            if (it.is_p) {
                paper_is_paired[pid] = true;
                int pidx = it.id;
                pairs[pidx].paper_id = pid;
                int u = pairs[pidx].u, v = pairs[pidx].v;
                prod_to_paper[u] = prod_to_paper[v] = pid;
                prod_to_pair_idx[u] = prod_to_pair_idx[v] = pidx;
                for (int j = 0; j <= w; ++j) {
                    int bu = (A[u] >> j) & 1, bv = (A[v] >> j) & 1;
                    if (bu == bv) S_init[pid][j] = bu ? '1' : '0';
                }
                for (int j = T; j < N; ++j) {
                    int bu = (A[u] >> j) & 1, bv = (A[v] >> j) & 1;
                    if (bu == bv) S_init[pid][j] = bu ? '1' : '0';
                }
            } else {
                int x = it.id;
                prod_to_paper[x] = pid;
                for (int j = 0; j <= w; ++j) S_init[pid][j] = ((A[x] >> j) & 1) ? '1' : '0';
                for (int j = T; j < N; ++j) S_init[pid][j] = ((A[x] >> j) & 1) ? '1' : '0';
            }
        }
    }

    for (int x=0;x<P;++x) if(prod_to_paper[x]<0 || prod_to_paper[x]>=K) return nullopt;


        return Design{W,T,C,tot_B,move(S_init),move(pairs),move(prod_to_paper),move(prod_to_pair_idx),move(paper_bucket),move(paper_is_paired),move(local_A),move(column_order)};
    };

    vector<Design> candidates;
    const int pool=max(1,search_budget::integer("NOV8_POOL",4));
    const int width=getenv("PACK_W")?atoi(getenv("PACK_W")):38;

    candidates.reserve(pool);
    for(int offset=0;offset<pool;offset++) {
        if(!candidates.empty() && design_elapsed()>search_budget::real("NOV8_POOL_END",1.75))break;
        graph_seed_offset=offset;
        parent_design=((offset & 1) && !candidates.empty())?&candidates.back():nullptr;

        const int backing=(offset<2)?5:6;
        if(parent_design && parent_design->tot_B!=backing)parent_design=nullptr;
        int w=(backing==5?36:width)+(parent_design!=nullptr);
        auto d=build_design(backing,0,0,w);
        if(d)candidates.push_back(move(*d));
    }
    // Preserve the original candidates. For up to three promising comparison
    // maps, rebuild only the staircase and main-sheet assignments.
    sort(snapshots.begin(),snapshots.end(),[](const auto& a,const auto& b){return a.price<b.price;});
    for(int i=0;i<min(int(snapshots.size()),3);i++){
        if(design_elapsed()>layout_deadline-.08)break;
        graph_seed_offset=pool+i;
        auto d=build_design(snapshots[i].B,0,0,snapshots[i].W,&snapshots[i]);
        if(d)candidates.push_back(move(*d));
    }
    if(candidates.empty())return fallback();

    vector<vector<array<double,32>>> rank_hist(candidates.size(),vector<array<double,32>>(K));
    vector<unique_ptr<OnlineRouter>> trials;
    if(candidates.size()>1)for(const auto &d:candidates){
        trials.push_back(make_unique<OnlineRouter>(d,d.internal_catalog,absolute_deadline(search_budget::real("NOV8_SELECT_END",2.18))));
        trials.back()->route_deadline=absolute_deadline(search_budget::real("NOV8_SELECT_END",2.18));
    }
    vector<double> scores(candidates.size(),0);
    vector<bool> feasible(candidates.size(),true);
    uint64_t random_state=0x4d595df4d0f33173ULL;
    // The sequence is independent of the actual future orders.
    // A catalog hash only gives independent deterministic sequences across instances.
    for(uint64_t x:A){random_state^=x+0x9e3779b97f4a7c15ULL+(random_state<<6)+(random_state>>2);}
    // Zero is an absorbing state for this xorshift generator.
    if(random_state==0)random_state=0x4d595df4d0f33173ULL;
    auto random32=[&](){random_state^=random_state<<7;random_state^=random_state>>9;return uint32_t(random_state);};
    // Race layouts on the same orders. Eliminating a layout compares only
    // the queries for which it and the current leader have both been scored.
    vector<double> one(candidates.size());
    vector<vector<double>> variance(candidates.size(),vector<double>(candidates.size()));
    int requested=max(256,search_budget::integer("NOV8_QUERIES",8192)), previous=-1, synthetic_queries=0;
    if(candidates.size()==1)requested=0;
    // Sample all 256 equally likely products once per block. This reduces
    // noise when one layout pairs a given product and another stores it singly.
    array<int,P> evaluation_order{};
    for(int a=0;a<P;++a)evaluation_order[a]=a;
    for(int q=0;q<requested;q++) {
        // Compare a useful common sample even if router initialization consumed
        // the soft budget; the hard ceiling bounds the extra design work.
        if(design_elapsed()>search_budget::real("NOV8_SELECT_HARD",2.29) || (q>=256 && design_elapsed()>search_budget::real("NOV8_SELECT_END",2.18)))break;
        synthetic_queries++;
        int index=q%P;
        if(index==0) {
            for(int j=P-1;j>0;--j)swap(evaluation_order[j],evaluation_order[random32()%(j+1)]);
            if(evaluation_order[0]==previous)swap(evaluation_order[0],evaluation_order[1+random32()%(P-1)]);
        }
        int a=evaluation_order[index];
        const int last=previous;
        previous=a;
        for(int i=0;i<(int)candidates.size();i++) {
            if(!feasible[i])continue;
            const double previous_score=scores[i];
            scores[i]+=.9*order_adjustment(*trials[i],a,last);
            auto moves=trials[i]->query(a);
            scores[i]+=moves.size();
            one[i]=scores[i]-previous_score;
            if(moves.size()>2*K || trials[i]->sim.current_display()!=trials[i]->A[a])feasible[i]=false;
            array<int,64> rank_count{};
            for(int k:trials[i]->sim.order) {
                int w=candidates[i].paper_bucket[k];
                if(w<candidates[i].C || k==trials[i]->active_main)continue;
                rank_hist[i][k][min(31,rank_count[w]++)]++;
            }

        }
        for(int i=0;i<int(trials.size());i++)if(feasible[i])
            for(int j=0;j<i;j++)if(feasible[j]){
                double delta=one[j]-one[i];variance[i][j]+=delta*delta;
            }
        if((q+1)%128==0 && q>=255){
            int best=-1;for(int i=0;i<int(trials.size());i++)
                if(feasible[i] && (best<0 || scores[i]<scores[best]))best=i;
            if(best<0)break;
            int live=0;
            for(int i=0;i<int(trials.size());i++)if(feasible[i]){
                double delta=scores[i]-scores[best];
                double sum=variance[max(best,i)][min(best,i)];
                double threshold=1.4*sqrt(max(0.,sum-delta*delta/(q+1)));
                if(delta>threshold)feasible[i]=false;
                else live++;
            }
            if(live==1 || q+1>=2048)break;
        }
    }
    int selected=0;
    for(int i=1;i<(int)candidates.size();i++) {
        if(feasible[i] && (!feasible[selected] || scores[i]<scores[selected]))selected=i;
    }
    if(!feasible[selected])return fallback();

    Design &chosen=candidates[selected];
    vector<HeadSample> heads;
    if(!trials.empty()){
        auto& t=*trials[selected];t.sim.init(chosen.S_init);t.active_main=-1;t.active_supports.clear();
        fill(t.support_cut.begin(),t.support_cut.end(),-1);

        const int n=search_budget::integer("HIST_SAMPLES",42);
        uint64_t state=A[0]|1;
        int last=-1;
        for(int z=0;z<n*7+32;z++){
            int sample;
            do{state^=state<<7;state^=state>>9;sample=int(state%P);}while(sample==last);
            t.query(sample);last=sample;
            if(z>=32 && (z-32)%7==0){heads.emplace_back();capture_heads(t,heads.back());}
        }
        sampled_bank_swaps(chosen,heads);
        if(search_budget::integer("HIST_REDESIGN",1))history_holes(chosen,heads);
    }
    if(trials.empty()||!search_budget::integer("HIST_REDESIGN",1)){
        prefix_holes::Options opt;
        opt.max_total=16;
        opt.max_per_paper=1;
        opt.rank_mode=4;
        opt.rank_weights=&rank_hist[selected];
        opt.objective_mode=1;
        opt.own_hole_cost=.5;
        opt.minimum_gain=0;
        clock_t hole_started=clock();
        opt.deadline=std::min(absolute_deadline(search_budget::real("NOV8_HOLES_END",2.39)),hole_started+clock_t(search_budget::seconds(.06)*CLOCKS_PER_SEC));
        auto result=prefix_holes::optimize(chosen.S_init,chosen.internal_catalog,chosen.prod_to_paper,chosen.paper_bucket,chosen.tot_B,chosen.T,chosen.C,opt);
        cerr<<"PREFIX_HOLES_AFTER objective="<<opt.objective_mode<<" rank="<<opt.rank_mode<<" cap="<<opt.max_total<<" per="<<opt.max_per_paper<<" erased="<<result.erasures.size()<<" utility="<<result.utility_gain/P<<" own="<<result.own_cost/P<<" net="<<result.net_gain/P<<" cpu="<<double(clock()-hole_started)/CLOCKS_PER_SEC<<" timeout="<<result.timed_out<<"\n";
    }

    cerr<<"CHOSEN W="<<chosen.W<<" index="<<selected<<" candidates="<<candidates.size()<<"\n";
    for(const auto &s:chosen.S_init) {
        string out(N,'.');for(int j=0;j<N;j++)out[chosen.column_order[j]]=s[j];
        cout<<out<<'\n';
    }
    cout.flush();
    const double design_ready=design_elapsed();
    OnlineRouter actual(chosen,chosen.internal_catalog,absolute_deadline(search_budget::real("NOV8_INIT_END",2.47)));
    empirical_utility(actual,heads);
    actual.prepare_bucket_mdp(absolute_deadline(search_budget::real("NOV8_MDP_END",2.54)));
    const double online_started=design_elapsed();
    actual.route_deadline=absolute_deadline(search_budget::real("NOV8_ROUTE_END",2.98));
    actual.route_branching=true;
    const bool portfolio_configured=search_budget::integer("ROUTE_PORTFOLIO",1)!=0;
    const int portfolio_accept_mode=clamp(search_budget::integer("PORTFOLIO_ACCEPT_MODE",0),0,2);
    actual.portfolio_enabled=false;
    const double portfolio_cap=search_budget::seconds(max(0.0,search_budget::real("PORTFOLIO_CPU",.4)));
    const double portfolio_reserve=max(0.0,search_budget::real("PORTFOLIO_RESERVE",1.10));
    const clock_t portfolio_margin=clock_t(.01*CLOCKS_PER_SEC); // Native CPU seconds.
    double portfolio_cpu=0;
    int portfolio_trials=0,portfolio_wins=0,portfolio_saved=0,portfolio_collected=0;
    int portfolio_warmup_skips=0,portfolio_reserve_skips=0,portfolio_cpu_skips=0,portfolio_late_skips=0;

    int trace_saved=0,trace_queries=0;
    BankFeedback feedback(actual);
    clock_t core_ticks=0,primary_ticks=0;
    for(int q=0;q<R;q++) {
        int a;
        if(!(cin>>a)||a==-1)return 0;
        auto trace_start=actual.sim.order;uint64_t previous_heads=feedback.background(actual);
        const clock_t gate_time=clock();
        actual.portfolio_enabled=false;
        if(portfolio_configured) {
            if(q<16)portfolio_warmup_skips++;
            else if(portfolio_cpu>=portfolio_cap)portfolio_cpu_skips++;
            else {
                const clock_t reserve=clock_t(ceil(double(primary_ticks)*(R-q)/q*portfolio_reserve))+portfolio_margin;
                if(actual.route_deadline>gate_time && reserve<actual.route_deadline-gate_time) {
                    actual.portfolio_enabled=true;portfolio_collected++;
                }else portfolio_reserve_skips++;
            }
        }
        clock_t before_route=clock();
        auto moves=actual.query(a);
        clock_t now=clock();
        core_ticks+=now-before_route;
        // Reserve time for the remaining orders' primary routing. Use the
        // observed mean in this case; each extra suffix search is optional.
        clock_t deadline=absolute_deadline(search_budget::real("NOV8_RELAX_END",3.00));
        if(q>=15 && q+1<R){
            auto reserve=clock_t(double(core_ticks)*(R-1-q)/(q+1));
            deadline=min(deadline,max(clock_t(1),actual.route_deadline-reserve));
        }

        vector<pair<int,int>> raw_moves;
        double raw_score=0;
        if(actual.portfolio_enabled) {
            raw_moves=moves;raw_score=moves.size()+actual.route_state_value();
        }
        moves=nov5_relaxed_trace::optimize(actual,a,trace_start,move(moves),deadline);
        // Reserve the observed cost of actual.query plus its incumbent
        // relaxation. Alternative work is excluded from this running mean.
        const clock_t primary_done=clock();
        primary_ticks+=primary_done-before_route;
        const clock_t future_reserve=clock_t(ceil(double(primary_ticks)*(R-1-q)/(q+1)*portfolio_reserve))+portfolio_margin;
        const clock_t reserve_deadline=actual.route_deadline-future_reserve;
        const clock_t cpu_deadline=primary_done+clock_t(max(0.0,portfolio_cap-portfolio_cpu)*CLOCKS_PER_SEC);
        const clock_t extra_deadline=min(deadline,min(reserve_deadline,cpu_deadline));
        if(actual.portfolio_enabled && portfolio_cpu<portfolio_cap && primary_done<extra_deadline) {
            const clock_t began=clock();
            auto best_state=actual.save_route_state();
            double best_score=moves.size()+actual.route_state_value();
            int tried=0;
            const double slack=search_budget::real("PORTFOLIO_SLACK",1.5);
            const int limit=clamp(search_budget::integer("PORTFOLIO_TRIES",1),1,16);
            for(const auto& alternative:actual.portfolio) {
                if(tried>=limit || clock()>=extra_deadline)break;
                if(alternative.moves==raw_moves || alternative.score>raw_score+slack)continue;
                actual.restore_route_state(alternative.state);
                auto shortened=nov5_relaxed_trace::optimize(actual,a,trace_start,alternative.moves,extra_deadline);
                double score=shortened.size()+actual.route_state_value();
                tried++;portfolio_trials++;
                const bool admissible=portfolio_accept_mode==0 ||
                    (shortened.size()<moves.size() &&
                     (portfolio_accept_mode<2 || actual.active_supports.size()<=best_state.supports.size()));
                if(score<best_score-1.e-9 && admissible) {
                    portfolio_wins++;portfolio_saved+=int(moves.size())-int(shortened.size());
                    best_score=score;best_state=actual.save_route_state();moves=move(shortened);
                }
            }
            // Even a timeout or a non-improving last trial leaves the chosen
            // incumbent's exact deck, support set, and cuts installed.
            actual.restore_route_state(best_state);
            portfolio_cpu+=double(clock()-began)/CLOCKS_PER_SEC;
        }else if(actual.portfolio_enabled)portfolio_late_skips++;

        int trace_before=moves.size();
        moves=trace_shortener::optimize(trace_start,std::move(moves),actual.opaque,absolute_deadline(search_budget::real("NOV8_SHORT_END",2.96)));
        trace_saved+=trace_before-int(moves.size());
        trace_queries+=trace_before>int(moves.size());
        feedback.observe(actual,a,previous_heads,q);
        cout<<moves.size()<<'\n';
        for(auto [k,d]:moves)cout<<k<<' '<<d<<'\n';
        cout.flush();
    }

    cerr<<"PORTFOLIO configured="<<portfolio_configured<<" accept_mode="<<portfolio_accept_mode<<" trials="<<portfolio_trials<<" wins="<<portfolio_wins
        <<" saved="<<portfolio_saved<<" cpu="<<portfolio_cpu<<" primary_cpu="<<double(primary_ticks)/CLOCKS_PER_SEC
        <<" collected="<<portfolio_collected<<" warmup_skips="<<portfolio_warmup_skips
        <<" reserve_skips="<<portfolio_reserve_skips<<" cpu_skips="<<portfolio_cpu_skips
        <<" late_skips="<<portfolio_late_skips<<" reserve_factor="<<portfolio_reserve<<"\n";
    const auto &bc=routing_budget::counters();
    cerr<<"BUDGET design_ready="<<design_ready<<" online_started="<<online_started<<" synthetic_queries="<<synthetic_queries<<" cpu="<<double(clock()-design_started)/CLOCKS_PER_SEC<<" budget_units="<<design_elapsed()
        <<" future_aborts="<<bc.future_build_aborts<<" utility_aborts="<<bc.support_utility_aborts<<" policy_stops="<<bc.policy_early_stops
        <<" extract_aborts="<<bc.extract_dp_aborts<<" trace_skips="<<bc.trace_skipped_calls<<" trace_aborts="<<bc.trace_dp_aborts<<"\n";
    cerr<<"TRACE_SHORTEN saved="<<trace_saved<<" queries="<<trace_queries<<"\n";
    const auto&nr=nov5_relaxed_trace::stats();
    cerr<<"NOV5_RELAX calls="<<nr.calls<<" trials="<<nr.trials<<" display="<<nr.display_ok
        <<" state="<<nr.state_ok<<" accepted="<<nr.accepted<<" saved="<<nr.saved
        <<" auto_skipped="<<nr.skipped_invisible<<" extra_supports="<<nr.extra_supports
        <<" wide="<<nr.wide_accepts<<" interleaved="<<nr.interleaved_supports<<" triples="<<nr.triple_trials
        <<" early_main="<<nr.early_main<<" early_color="<<nr.early_color
        <<" cpu="<<nr.cpu<<"\n";
    if(actual.graph)cerr<<"BACKING_ROUTE_CACHE hits="<<actual.graph->route_cache_hits<<" misses="<<actual.graph->route_cache_misses<<"\n";
    cerr<<"BMDP_USE hits="<<actual.bucket_mdp_hits<<" misses="<<actual.bucket_mdp_misses
        <<" support_fallbacks="<<actual.bucket_mdp_support_fallbacks
        <<" projection_hits="<<actual.bucket_mdp_projection_hits<<"\n";
    cerr<<"POLICY_GUARD hits="<<actual.route_guard_hits<<"\n";
    cerr<<"ALTERNATE_RETURN trials="<<actual.return_trials<<" selected="<<actual.return_wins<<"\n";
    cerr<<"ROUND12 choices="<<actual.post_choice_trials<<" wins="<<actual.post_choice_wins<<" saved="<<actual.post_choice_saved<<" front_kept="<<actual.front_support_kept<<" core_returns="<<actual.front_support_core_returns<<"\n";
    cerr<<"POST_SECOND choices="<<actual.post_second_trials<<" wins="<<actual.post_second_wins<<" saved="<<actual.post_second_saved<<"\n";
    cerr<<"NOV9_INSERT_CACHE requests="<<actual.nov9_insert_requests<<" hits="<<actual.nov9_insert_hits
        <<" misses="<<actual.nov9_insert_misses<<" scanned="<<actual.nov9_insert_scanned
        <<" peak="<<actual.nov9_insert_peak<<" long_keys="<<actual.nov9_insert_long_keys
        <<" full="<<actual.nov9_insert_full<<" enabled="<<actual.nov9_insert_enabled<<" storage=fixed256\n";
    cerr<<"EXTRACT_DP calls="<<actual.joint_extract_calls<<" improved="<<actual.joint_extract_improved<<" cache_hits="<<actual.joint_extract_cache_hits<<" gain="<<actual.joint_extract_gain<<"\n";
    return 0;
}

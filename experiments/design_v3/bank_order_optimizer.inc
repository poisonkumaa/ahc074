// Permute complete prefix banks together with their anchor columns. Pairing,
// backing ink, bank contents, and within-bank initial orders remain fixed.
// Include after HeadSample/capture_heads and before main().
#include <numeric>
namespace design_v3_bank_order {
struct Value {double mismatch=0,donor=0,total=0;};
struct Model {
    const Design& d;
    int T,B,H;
    uint64_t prefix;
    std::array<int,K> owner{},old_home{};
    std::array<uint64_t,K> allowed{};
    std::vector<HeadSample> heads;
    std::vector<std::vector<double>> reward;
    double donor_weight;
    clock_t deadline;
    int evaluations=0;
    explicit Model(const Design& source,const std::vector<HeadSample>& history,
                   int samples,double donor,clock_t stop):
        d(source),T(d.T),B(d.tot_B),H(std::min(samples,int(history.size()))),
        prefix((1ULL<<T)-1),reward(T,std::vector<double>(T)),
        donor_weight(donor),deadline(stop) {
        owner.fill(-1);old_home.fill(-1);allowed.fill(prefix);
        for(int a=0;a<P;a++) {
            const int k=d.prod_to_paper[a];
            if(owner[k]<0)owner[k]=a;
            else allowed[k]&=~(d.internal_catalog[owner[k]]^d.internal_catalog[a]);
        }
        for(int k=B;k<K;k++) {
            old_home[k]=d.paper_bucket[k];
            for(int j=0;j<=old_home[k];j++)
                if(d.S_init[k][j]=='.')allowed[k]&=~(1ULL<<j);
        }
        for(int h=0;h<H;h++)heads.push_back(history[size_t(h)*history.size()/H]);
        std::array<std::array<double,2>,64> wrong{};
        for(int j=0;j<T;j++)for(const auto& h:heads)
            wrong[j][1-int(h.black>>j&1)]+=1.0/H;
        for(int a=0;a<P;a++) {
            const int k=d.prod_to_paper[a],home=old_home[k];
            for(int j=0;j<T;j++)if((allowed[k]>>j)&1)
                reward[home][j]+=wrong[j][int(d.internal_catalog[a]>>j&1)];
        }
    }
    uint64_t permute(uint64_t bits,const std::vector<int>& order) const {
        uint64_t result=bits&~prefix;
        for(int j=0;j<T;j++)result|=((bits>>order[j])&1ULL)<<j;
        return result;
    }
    double gain(const std::vector<int>& order) const {
        double value=0;
        for(int i=0;i<T;i++)for(int j=0;j<i;j++)value+=reward[order[i]][order[j]];
        return value;
    }
    double insertion_gain(const std::vector<int>& order,int from,int to) const {
        const int k=order[from];double delta=0;
        if(from<to)for(int p=from+1;p<=to;p++) {
            const int other=order[p];delta+=reward[k][other]-reward[other][k];
        }else for(int p=to;p<from;p++) {
            const int other=order[p];delta+=reward[other][k]-reward[k][other];
        }
        return delta;
    }
    bool evaluate(const std::vector<int>& order,Value& value) {
        if(clock()>=deadline)return false;
        std::array<int,64> rank{};
        for(int j=0;j<T;j++)rank[order[j]]=j;
        std::array<uint64_t,P> catalogue{};
        std::array<uint64_t,K> ink{},black{};
        std::vector<uint64_t> background(H);
        for(int a=0;a<P;a++)catalogue[a]=permute(d.internal_catalog[a],order);
        for(int k=B;k<K;k++) {
            ink[k]=permute(allowed[k],order)&((1ULL<<(rank[old_home[k]]+1))-1);
            black[k]=catalogue[owner[k]];
        }
        for(int h=0;h<H;h++)background[h]=permute(heads[h].black,order);
        double errors=0,savings=0;
        std::vector<int> best(H);
        std::vector<uint64_t> mismatch(H);
        for(int a=0;a<P;a++) {
            if(!(a&7) && clock()>=deadline)return false;
            const int main=d.prod_to_paper[a];
            const uint64_t target=catalogue[a],holes=prefix&~ink[main];
            std::fill(best.begin(),best.end(),0);
            for(int h=0;h<H;h++) {
                mismatch[h]=(background[h]^target)&holes;
                errors+=__builtin_popcountll(mismatch[h]);
            }
            if(donor_weight!=0)for(int k=B;k<K;k++)if(k!=main) {
                const int home=rank[old_home[k]];
                if(!(holes>>home&1))continue;
                const uint64_t bad=(black[k]^target)&ink[k]&holes;
                const int cut=bad?63-__builtin_clzll(bad):-1;
                if(cut>=home)continue;
                const uint64_t cover=ink[k]&holes&(~0ULL<<(cut+1));
                if(__builtin_popcountll(cover)<3)continue;
                for(int h=0;h<H;h++) {
                    const int saving=__builtin_popcountll(mismatch[h]&cover)-heads[h].depth[k]-2;
                    best[h]=std::max(best[h],saving);
                }
            }
            for(int v:best)savings+=v;
        }
        value={errors/(P*H),savings/(P*H),(errors-donor_weight*savings)/(P*H)};
        evaluations++;
        return clock()<deadline;
    }
};
inline void insert(std::vector<int>& order,int from,int to) {
    const int k=order[from];
    if(from<to)std::move(order.begin()+from+1,order.begin()+to+1,order.begin()+from);
    else std::move_backward(order.begin()+to,order.begin()+from,order.begin()+from+1);
    order[to]=k;
}
inline bool valid_design(const Design& d) {
    if(d.C!=0 || int(d.S_init.size())!=K || int(d.internal_catalog.size())!=P)return false;
    std::array<int,K> own{};
    std::array<std::array<int,2>,64> colors{};
    for(int a=0;a<P;a++) {
        const int k=d.prod_to_paper[a];if(k<d.tot_B || k>=K)return false;
        own[k]++;
        for(int j=0;j<N;j++)if(d.S_init[k][j]!='.' &&
            d.S_init[k][j]!=char('0'+int(d.internal_catalog[a]>>j&1)))return false;
    }
    int previous=-1;
    for(int k=d.tot_B;k<K;k++) {
        const int w=d.paper_bucket[k];
        if(w<0 || w>=d.T || w<previous || own[k]<1 || own[k]>2)return false;
        if(bool(own[k]==2)!=bool(d.paper_is_paired[k]))return false;
        const char anchor=d.S_init[k][w];if(anchor!='0' && anchor!='1')return false;
        colors[w][anchor-'0']++;
        for(int j=w+1;j<d.T;j++)if(d.S_init[k][j]!='.')return false;
        previous=w;
    }
    for(int w=0;w<d.T;w++)if(!colors[w][0] || !colors[w][1])return false;
    for(int i=0;i<int(d.pairs.size());i++) {
        const auto& p=d.pairs[i];
        if(d.prod_to_paper[p.u]!=p.paper_id || d.prod_to_paper[p.v]!=p.paper_id ||
           d.prod_to_pair_idx[p.u]!=i || d.prod_to_pair_idx[p.v]!=i ||
           d.paper_bucket[p.paper_id]!=p.bucket_w)return false;
    }
    return has_complete_backing_graph(d);
}
inline bool optimize(Design& d,std::vector<HeadSample>& history,clock_t absolute_deadline) {
    if(!search_budget::integer("V3_BANK_ORDER",0))return false;
    if(d.C!=0 || d.T<2 || d.T>40 || history.empty() || !valid_design(d)) {
        std::cerr<<"V3_BANK_ORDER skipped=unsupported\n";return false;
    }
    const clock_t started=clock();
    const double budget=search_budget::seconds(std::max(0.0,search_budget::real("V3_ORDER_CPU",.30)));
    clock_t deadline=started+clock_t(budget*CLOCKS_PER_SEC);
    if(absolute_deadline>0)deadline=std::min(deadline,absolute_deadline);
    if(clock()>=deadline){std::cerr<<"V3_BANK_ORDER skipped=deadline\n";return false;}
    const int H=std::clamp(search_budget::integer("V3_ORDER_HEADS",12),1,64);
    const int iterations=std::max(0,search_budget::integer("V3_ORDER_ANNEAL",16000));
    const int pool_limit=std::clamp(search_budget::integer("V3_ORDER_POOL",12),2,64);
    const int local_limit=std::max(0,search_budget::integer("V3_ORDER_LOCAL",80));
    const double donor=std::max(0.0,search_budget::real("V3_ORDER_DONOR",1.0));
    const double margin=std::max(0.0,search_budget::real("V3_ORDER_MARGIN",.005));
    const bool force=search_budget::integer("V3_ORDER_FORCE",0)!=0;
    Model model(d,history,H,donor,deadline);
    const int T=d.T;
    std::vector<int> identity(T);std::iota(identity.begin(),identity.end(),0);
    Value before;
    if(!model.evaluate(identity,before)) {
        std::cerr<<"V3_BANK_ORDER skipped=initial_score_timeout\n";return false;
    }
    std::vector<std::vector<int>> proposals;
    auto offer=[&](const std::vector<int>& order) {
        if(order==identity || std::find(proposals.begin(),proposals.end(),order)!=proposals.end())return;
        proposals.push_back(order);
    };
    uint64_t random=d.internal_catalog[0]^0x9ff25c57ac83d16bULL;
    if(!random)random=1;
    auto next=[&](){random^=random<<7;random^=random>>9;return uint32_t(random);};
    auto global=identity;
    double global_gain=model.gain(global);
    const int restarts=4;
    for(int restart=0;restart<restarts && clock()<deadline;restart++) {
        auto order=restart%2?global:identity;
        double value=model.gain(order),best=value;
        auto local_best=order;
        const int steps=iterations/restarts;
        for(int it=0;it<steps;it++) {
            if(!(it&255) && clock()>=deadline)break;
            int x=next()%T,y=next()%T;if(x==y)continue;
            const double delta=model.insertion_gain(order,x,y);
            const double temperature=(restart<2?2.0:4.0)*std::pow(.01,double(it)/std::max(1,steps));
            if(delta>=0 || double(next())/4294967296.0<std::exp(delta/temperature)) {
                insert(order,x,y);value+=delta;
                if(value>best+1e-9){best=value;local_best=order;}
                if(value>global_gain+1e-9){global_gain=value;global=order;}
            }
        }
        offer(local_best);
    }
    offer(global);
    // Keep moderate changes as well as the coverage optimum; strong donor
    // opportunities can make the best full order lie between those extremes.
    auto partial=identity;
    int changed=0;
    for(int at=T-1;at>=0;at--) {
        int old=std::find(partial.begin(),partial.end(),global[at])-partial.begin();
        if(old==at)continue;
        insert(partial,old,at);changed++;
        if(changed%std::max(1,T/6)==0)offer(partial);
    }
    if(int(proposals.size())>pool_limit)proposals.resize(pool_limit);
    auto best_order=identity,forced_order=identity;
    Value best_value=before,forced_value;forced_value.total=1e100;
    int tested=0;
    for(const auto& order:proposals) {
        Value value;if(!model.evaluate(order,value))break;tested++;
        if(value.total<forced_value.total){forced_value=value;forced_order=order;}
        if(value.total<best_value.total-1e-9){best_value=value;best_order=order;}
    }
    struct Trial {int from,to;double gain;};
    int local_evaluated=0,rounds=0;
    while(local_evaluated<local_limit && clock()<deadline) {
        std::vector<Trial> trials;
        for(int x=0;x<T;x++)for(int y=0;y<T;y++)if(x!=y)
            trials.push_back({x,y,model.insertion_gain(best_order,x,y)});
        std::sort(trials.begin(),trials.end(),[](const Trial&a,const Trial&b){
            if(a.gain!=b.gain)return a.gain>b.gain;
            if(a.from!=b.from)return a.from<b.from;
            return a.to<b.to;
        });
        auto round_order=best_order;Value round_value=best_value;
        std::vector<std::pair<int,int>> used;
        for(int pick=0;pick<16 && local_evaluated<local_limit;pick++) {
            const auto& trial=trials[pick<8?pick:next()%trials.size()];
            const std::pair<int,int> key{trial.from,trial.to};
            if(std::find(used.begin(),used.end(),key)!=used.end())continue;
            used.push_back(key);
            auto order=best_order;insert(order,trial.from,trial.to);
            Value value;if(!model.evaluate(order,value))break;
            local_evaluated++;
            if(order!=identity && value.total<forced_value.total){forced_value=value;forced_order=order;}
            if(value.total<round_value.total-1e-9){round_value=value;round_order=std::move(order);}
        }
        rounds++;
        if(round_order==best_order)break;
        best_order=std::move(round_order);best_value=round_value;
    }
    bool selected=best_order!=identity && best_value.total<before.total-margin;
    if(!selected && force && forced_order!=identity) {
        selected=true;best_order=forced_order;best_value=forced_value;
    }
    int moved=0,inversions=0;
    for(int i=0;i<T;i++) {
        moved+=best_order[i]!=i;
        for(int j=0;j<i;j++)inversions+=best_order[j]>best_order[i];
    }
    bool committed=false;
    if(selected) {
        Design candidate=d;
        std::array<int,64> rank{};
        for(int j=0;j<T;j++)rank[best_order[j]]=j;
        std::vector<int> papers;
        for(int k=d.tot_B;k<K;k++)papers.push_back(k);
        std::stable_sort(papers.begin(),papers.end(),[&](int a,int b){return rank[d.paper_bucket[a]]<rank[d.paper_bucket[b]];});
        std::array<int,K> paper_map{};
        for(int k=0;k<d.tot_B;k++)paper_map[k]=k;
        for(int i=0;i<int(papers.size());i++)paper_map[papers[i]]=d.tot_B+i;
        for(int j=0;j<T;j++)candidate.column_order[j]=d.column_order[best_order[j]];
        for(int a=0;a<P;a++) {
            candidate.internal_catalog[a]=model.permute(d.internal_catalog[a],best_order);
            candidate.prod_to_paper[a]=paper_map[d.prod_to_paper[a]];
        }
        for(int old=0;old<K;old++) {
            const int k=paper_map[old];candidate.S_init[k]=d.S_init[old];
            candidate.paper_is_paired[k]=d.paper_is_paired[old];
            if(old<d.tot_B) {
                for(int j=0;j<T;j++)candidate.S_init[k][j]=d.S_init[old][best_order[j]];
                continue;
            }
            const int home=rank[d.paper_bucket[old]];
            candidate.paper_bucket[k]=home;
            const uint64_t ink=model.permute(model.allowed[old],best_order)&((1ULL<<(home+1))-1);
            const uint64_t bits=candidate.internal_catalog[model.owner[old]];
            for(int j=0;j<T;j++)candidate.S_init[k][j]=(ink>>j&1)?char('0'+int(bits>>j&1)):'.';
        }
        for(auto& p:candidate.pairs){p.paper_id=paper_map[p.paper_id];p.bucket_w=rank[p.bucket_w];}
        if(valid_design(candidate)) {
            auto new_history=history;
            for(size_t h=0;h<history.size();h++) {
                new_history[h].black=model.permute(history[h].black,best_order);
                for(int k=0;k<K;k++)new_history[h].depth[paper_map[k]]=history[h].depth[k];
            }
            d=std::move(candidate);history=std::move(new_history);committed=true;
        }
    }
    std::cerr<<"V3_BANK_ORDER evaluations="<<model.evaluations<<" proposals="<<tested
        <<" local="<<local_evaluated<<" rounds="<<rounds<<" selected="<<selected
        <<" committed="<<committed<<" moved="<<moved<<" inversions="<<inversions
        <<" before="<<before.total<<" after="<<best_value.total
        <<" mismatch_before="<<before.mismatch<<" mismatch_after="<<best_value.mismatch
        <<" donor_before="<<before.donor<<" donor_after="<<best_value.donor
        <<" timeout="<<(clock()>=deadline)<<" cpu="<<double(clock()-started)/CLOCKS_PER_SEC<<"\n";
    return committed;
}
} // namespace design_v3_bank_order

#pragma once
#include <bits/stdc++.h>

// Exact whole-pattern coordinate optimization against real shortest move
// distances, on internally generated reachable states (one per public target).
inline void train_real_prefix_moves(const std::vector<uint64_t>& products,
                                   std::vector<PrefixPaper>& papers,
                                   int n,int passes=1,int coordinate_passes=2,
                                   int minimum_length=1,int minimum_gain=0,int state_count=4,bool unique_patterns=true) {
    const int V=products.size(),P=V*state_count,K=papers.size(),INF=1000000;
    struct Example {
        std::vector<int> order,pos;
        std::vector<std::array<int,193>> cnt;
    };
    std::vector<Example> examples(P);
    std::vector<int> length,order(K),indices(P);
    for(auto a:papers)length.push_back(a.len);
    std::iota(order.begin(),order.end(),0);std::iota(indices.begin(),indices.end(),0);
    std::mt19937 random(0x2513c);
    PrefixDP online(n,papers);
    auto advance=[&](int target){
        int distance;auto goal=online.solve(order,products[target],distance);
        std::vector<char> selected(K);for(int k:goal)selected[k]=true;
        for(int step=0;step<distance;step++){
            auto [from,to]=GlobalPrefix::best_move(order,length,selected);
            if(from<0)throw std::runtime_error("real training simulation stalled");
            int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);
        }
    };
    for(int i=0;i<128;i++)advance(random()%V);
    std::shuffle(indices.begin(),indices.end(),random);
    for(int v:indices){
        auto&e=examples[v];e.order=order;e.pos.resize(K);e.cnt.resize(n+2);
        for(int i=0;i<K;i++)e.pos[order[i]]=i;
        for(int l=0;l<=n+1;l++){e.cnt[l][0]=0;for(int i=0;i<K;i++)e.cnt[l][i+1]=e.cnt[l][i]+(length[order[i]]>=l);}
        advance(v%V);
    }
    std::vector<std::vector<int>> level(n+1);
    for(int k=0;k<K;k++)level[length[k]].push_back(k);
    std::vector<std::array<int,193>>forward(n+1),backward(n+1);
    std::vector<int>first(K),suffix(K+2);
    long long total_gain=0;int changed=0;
    for(int pass=0;pass<passes;pass++)for(int l=n;l>=minimum_length;l--){
        const int Q=level[l].size();if(!Q)continue;
        std::vector<int>cost(P*Q*(l+1),INF),bypass(P,INF),initial(P);
        auto at=[&](int v,int q,int d)->int&{return cost[(v*Q+q)*(l+1)+d];};
        for(int v=0;v<P;v++){
            auto&e=examples[v];auto&cnt=e.cnt;
            for(int k=0;k<K;k++){uint64_t d=(products[v%V]^papers[k].color)&PrefixDP::low(length[k]);first[k]=d?64-__builtin_clzll(d):0;}
            for(auto&r:forward)r.fill(INF);forward[0][0]=0;
            for(int j=0;j<n;j++){
                int active[193],count=0;for(int h=0;h<=K;h++)if(forward[j][h]<INF)active[count++]=h;
                if(!count)continue;
                for(int k=0;k<K;k++)if(length[k]>j&&first[k]<=j){
                    int upper=length[k],position=e.pos[k];
                    for(int ai=0;ai<count;ai++){
                        int h=active[ai],add=0;
                        if(position>=h){int mandatory=cnt[upper][position]-cnt[upper][h];int opaque=cnt[j+1][position]-cnt[j+1][h];add=mandatory+(opaque>mandatory);}
                        int nh=std::max(position+1,h);
                        forward[upper][nh]=std::min(forward[upper][nh],forward[j][h]+add);
                    }
                }
            }
            for(auto&r:backward)r.fill(INF);backward[n].fill(0);
            for(int j=n-1;j>=l;j--)for(int k=0;k<K;k++)if(length[k]>j&&first[k]<=j){
                int upper=length[k],position=e.pos[k];
                for(int h=0;h<=K;h++){
                    int add=0;
                    if(position>=h){int mandatory=cnt[upper][position]-cnt[upper][h];int opaque=cnt[j+1][position]-cnt[j+1][h];add=mandatory+(opaque>mandatory);}
                    int nh=std::max(position+1,h);
                    backward[j][h]=std::min(backward[j][h],add+backward[upper][nh]);
                }
            }
            initial[v]=*std::min_element(forward[n].begin(),forward[n].begin()+K+1);
            // Every path avoiding length l has one edge that jumps across l.
            for(int k=0;k<K;k++)if(length[k]>l){
                int upper=length[k],position=e.pos[k];
                for(int j=first[k];j<l;j++)for(int h=0;h<=K;h++)if(forward[j][h]<INF){
                    int add=0;if(position>=h){int mandatory=cnt[upper][position]-cnt[upper][h];int opaque=cnt[j+1][position]-cnt[j+1][h];add=mandatory+(opaque>mandatory);}
                    bypass[v]=std::min(bypass[v],forward[j][h]+add+backward[upper][std::max(position+1,h)]);
                }
            }
            // Forced-use costs for every paper at l, before its color matters.
            for(int j=0;j<l;j++){
                suffix[K+1]=INF;
                for(int h=K;h>=0;h--)suffix[h]=std::min(suffix[h+1],forward[j][h]+backward[l][h]);
                int all=INF,recent=INF,earlier=INF;
                int prefix_best[192];
                for(int position=0;position<K;position++){
                    if(position&&length[e.order[position-1]]>j&&length[e.order[position-1]]<l){earlier=all;recent=INF;}
                    int value=forward[j][position]-cnt[l][position];
                    all=std::min(all,value);recent=std::min(recent,value);
                    prefix_best[position]=std::min(recent,earlier+1);
                }
                for(int q=0;q<Q;q++){
                    int position=e.pos[level[l][q]];
                    at(v,q,l-j)=std::min(suffix[position+1],prefix_best[position]+cnt[l][position]+backward[l][position+1]);
                }
            }
            for(int q=0;q<Q;q++)for(int d=1;d<=l;d++)at(v,q,d)=std::min(at(v,q,d),at(v,q,d-1));
            int check=bypass[v];for(int q=0;q<Q;q++){int k=level[l][q];check=std::min(check,at(v,q,l-first[k]));}
            if(check!=initial[v])throw std::runtime_error("real movement model mismatch");
        }
        struct Node {int child[2]{-1,-1},sample=0;long long weight=0,best=0;int choice=-1;};
        std::vector<Node>trie(1);std::vector<std::array<int,65>>paths(P);
        for(int v=0;v<P;v++){
            int x=0;paths[v][0]=0;trie[0].sample=v;
            for(int d=1;d<=l;d++){
                int bit=products[v%V]>>(l-d)&1;
                if(trie[x].child[bit]<0){int nx=trie.size();trie[x].child[bit]=nx;trie.emplace_back();}
                x=trie[x].child[bit];trie[x].sample=v;paths[v][d]=x;
            }
        }
        auto depth=[&](int v,int q){uint64_t d=(products[v%V]^papers[level[l][q]].color)&PrefixDP::low(l);return d?l-64+__builtin_clzll(d):l;};
        std::vector<std::vector<int>> match(P,std::vector<int>(Q));
        for(int v=0;v<P;v++)for(int q=0;q<Q;q++)match[v][q]=depth(v,q);
        if(true){
            // Average the true position-dependent improvement over all paper
            // positions at this length. This removes spurious paper-ID fitting.
            for(auto&node:trie)node.weight=0;
            for(int v=0;v<P;v++){
                long long previous=0;
                for(int d=1;d<=l;d++){
                    long long gain=0;for(int q=0;q<Q;q++)gain+=std::max(0,bypass[v]-at(v,q,d));
                    trie[paths[v][d]].weight+=8*(gain-previous)+Q;previous=gain;
                }
            }
            std::vector<std::array<long long,17>> tree_dp(trie.size());
            std::vector<std::array<int,17>> tree_split(trie.size());
            for(int x=int(trie.size())-1;x>=0;x--){
                auto&d=tree_dp[x];d.fill(-(1LL<<50));d[0]=0;
                int a=trie[x].child[0],b=trie[x].child[1];
                if(a<0&&b<0){d[1]=trie[x].weight;continue;}
                for(int k=1;k<=Q;k++)for(int left=0;left<=k;left++){
                    int right=k-left;if((a<0&&left)||(b<0&&right))continue;
                    long long value=(a<0?0:tree_dp[a][left])+(b<0?0:tree_dp[b][right])+trie[x].weight;
                    if(value>d[k])d[k]=value,tree_split[x][k]=left;
                }
            }
            std::vector<uint64_t>patterns;
            std::function<void(int,int)>collect=[&](int x,int k){
                if(!k)return;int a=trie[x].child[0],b=trie[x].child[1];
                if(a<0&&b<0){patterns.push_back(products[trie[x].sample%V]&PrefixDP::low(l));return;}
                int left=tree_split[x][k];if(left)collect(a,left);if(k-left)collect(b,k-left);
            };collect(0,Q);
            if(int(patterns.size())==Q){
                std::vector<int>assignment(Q),best_assignment;std::iota(assignment.begin(),assignment.end(),0);
                long long best_alignment=LLONG_MAX;
                do{
                    long long score=0;for(int q=0;q<Q;q++){
                        uint64_t difference=patterns[assignment[q]]^papers[level[l][q]].color;
                        int suffix=difference?l-64+__builtin_clzll(difference):l;
                        score+=(l-suffix)*100+__builtin_popcountll(difference);
                    }
                    if(score<best_alignment)best_alignment=score,best_assignment=assignment;
                }while(std::next_permutation(assignment.begin(),assignment.end()));
                long long old_sum=0,new_sum=0;bool reachable=true;
                for(int v=0;v<P;v++){
                    int old_cost=bypass[v],new_cost=bypass[v];
                    for(int q=0;q<Q;q++){
                        old_cost=std::min(old_cost,at(v,q,match[v][q]));
                        uint64_t d=(patterns[best_assignment[q]]^products[v%V])&PrefixDP::low(l);
                        int depth=d?l-64+__builtin_clzll(d):l;new_cost=std::min(new_cost,at(v,q,depth));
                    }
                    if(new_cost>=INF)reachable=false;old_sum+=old_cost;new_sum+=new_cost;
                }
                if(reachable){for(int q=0;q<Q;q++){int k=level[l][q];changed+=papers[k].color!=patterns[best_assignment[q]];papers[k].color=patterns[best_assignment[q]];}total_gain+=old_sum-new_sum;}
            }
        }else for(int iteration=0;iteration<coordinate_passes;iteration++){
            bool improved=false;
            for(int q=0;q<Q;q++){
                for(auto&node:trie)node.weight=0;
                long long old_gain=0;
                for(int v=0;v<P;v++){
                    int base=bypass[v];for(int other=0;other<Q;other++)if(other!=q)base=std::min(base,at(v,other,match[v][other]));
                    old_gain+=std::max(0,base-at(v,q,match[v][q]));
                    int previous=0;
                    for(int d=1;d<=l;d++){
                        int gain=std::max(0,base-at(v,q,d));
                        trie[paths[v][d]].weight+=gain-previous;previous=gain;
                    }
                }
                for(int x=int(trie.size())-1;x>=0;x--){
                    auto&node=trie[x];node.best=node.weight;node.choice=-1;
                    if(unique_patterns&&node.child[0]<0&&node.child[1]<0){
                        uint64_t color=products[node.sample%V]&PrefixDP::low(l);
                        bool duplicate=false;for(int other=0;other<Q;other++)if(other!=q&&papers[level[l][other]].color==color)duplicate=true;
                        if(duplicate){node.best=-(1LL<<50);continue;}
                    }
                    for(int child:node.child)if(child>=0&&(node.choice<0||trie[child].best>trie[node.choice].best))node.choice=child;
                    if(node.choice>=0)node.best+=trie[node.choice].best;
                }
                if(trie[0].best<=old_gain+minimum_gain)continue;
                int leaf=0;while(trie[leaf].choice>=0)leaf=trie[leaf].choice;
                papers[level[l][q]].color=products[trie[leaf].sample%V]&PrefixDP::low(l);
                for(int v=0;v<P;v++)match[v][q]=depth(v,q);
                total_gain+=trie[0].best-old_gain;changed++;improved=true;
            }
            if(!improved)break;
        }
        if(l%8==0)std::cerr<<"real train length="<<l<<" changed="<<changed<<" empirical_gain="<<double(total_gain)/P<<" cpu="<<double(clock())/CLOCKS_PER_SEC<<"\n";
    }
}

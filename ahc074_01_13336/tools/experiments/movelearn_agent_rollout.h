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
    std::vector<int>warmup;
    for(int i=0;i<128;i++){int v=random()%V;warmup.push_back(v);advance(v);}
    std::shuffle(indices.begin(),indices.end(),random);
    for(int v:indices){
        auto&e=examples[v];e.order=order;e.pos.resize(K);e.cnt.resize(n+2);
        for(int i=0;i<K;i++)e.pos[order[i]]=i;
        for(int l=0;l<=n+1;l++){e.cnt[l][0]=0;for(int i=0;i<K;i++)e.cnt[l][i+1]=e.cnt[l][i]+(length[order[i]]>=l);}
        advance(v%V);
    }
    auto rollout=[&](std::vector<std::vector<int>>*record){
        PrefixDP machine(n,papers);std::vector<int>p(K);std::iota(p.begin(),p.end(),0);
        auto step=[&](int v){int cost;auto chosen=machine.solve(p,products[v],cost);std::vector<char>goal(K);for(int k:chosen)goal[k]=true;for(int it=0;it<cost;it++){auto [a,b]=GlobalPrefix::best_move(p,length,goal);if(a<0)throw std::runtime_error("rollout stalled");int k=p[a];p.erase(p.begin()+a);p.insert(p.begin()+b,k);}return cost;};
        for(int v:warmup)step(v);int sum=0;if(record)record->resize(P);
        for(int v:indices){if(record)(*record)[v]=p;sum+=step(v%V);}return sum;
    };
    int real_cost=rollout(nullptr),original_real_cost=real_cost,accepted=0,rejected=0;
    std::vector<std::vector<int>> level(n+1);
    for(int k=0;k<K;k++)level[length[k]].push_back(k);
    std::vector<std::array<int,193>>forward(n+1),backward(n+1);
    std::vector<int>first(K),suffix(K+2);
    long long total_gain=0;int changed=0;
    for(int pass=0;pass<passes;pass++)for(int l=n;l>=minimum_length;l--){
        const int Q=level[l].size();if(!Q)continue;
        std::vector<uint64_t>old_colors;for(int k:level[l])old_colors.push_back(papers[k].color);
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
        for(int iteration=0;iteration<coordinate_passes;iteration++){
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
        bool different=false;for(int q=0;q<Q;q++)different|=papers[level[l][q]].color!=old_colors[q];
        if(different){
            std::vector<std::vector<int>>new_orders;int next_cost=rollout(&new_orders);
            if(next_cost<real_cost){
                real_cost=next_cost;accepted++;
                for(int v=0;v<P;v++){
                    auto&e=examples[v];e.order=std::move(new_orders[v]);
                    for(int i=0;i<K;i++)e.pos[e.order[i]]=i;
                    for(int upper=0;upper<=n+1;upper++){e.cnt[upper][0]=0;for(int i=0;i<K;i++)e.cnt[upper][i+1]=e.cnt[upper][i]+(length[e.order[i]]>=upper);}
                }
            }else{for(int q=0;q<Q;q++)papers[level[l][q]].color=old_colors[q];rejected++;}
        }
        if(l%8==0)std::cerr<<"real train length="<<l<<" changed="<<changed<<" rollout="<<original_real_cost<<"->"<<real_cost<<" accept="<<accepted<<" reject="<<rejected<<" cpu="<<double(clock())/CLOCKS_PER_SEC<<"\n";
    }
}

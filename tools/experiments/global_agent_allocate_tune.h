#pragma once
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
        std::array<int,17> dp,split;
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

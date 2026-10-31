#include <bits/stdc++.h>
using namespace std;

// A six-paper design represents all sixteen patterns on four windows.
// Its 720 permutations permit exact planning inside each independent block.
struct BlockMachine {
    static constexpr int STATES = 720;
    static constexpr int MASK[6] = {11, 15, 2, 15, 15, 8};
    static constexpr int BLACK[6] = {8, 13, 2, 9, 8, 0};
    struct Edge { short from; unsigned char action; };
    array<array<unsigned char, 6>, STATES> order{};
    array<array<short, 36>, STATES> next{};
    array<vector<Edge>, STATES> reverse;
    array<int, STATES> display{};
    static int rank(const array<unsigned char, 6>& p) {
        int r = 0;
        for (int i = 0; i < 6; ++i) {
            int less = 0;
            for (int j = i + 1; j < 6; ++j) less += p[j] < p[i];
            r = r * (6 - i) + less;
        }
        return r;
    }
    BlockMachine() {
        array<unsigned char, 6> p = {0, 1, 2, 3, 4, 5};
        int s = 0;
        do { order[s++] = p; } while (next_permutation(p.begin(), p.end()));
        for (s = 0; s < STATES; ++s) {
            int covered = 0, bits = 0;
            next[s].fill(-1);
            for (int i = 0; i < 6; ++i) {
                int k = order[s][i];
                bits |= BLACK[k] & ~covered;
                if (MASK[k] & ~covered) {
                    for (int j = 0; j < 6; ++j) if (i != j) {
                        auto q = order[s];
                        if (i < j) rotate(q.begin() + i, q.begin() + i + 1, q.begin() + j + 1);
                        else rotate(q.begin() + j, q.begin() + i, q.begin() + i + 1);
                        int t = rank(q), action = i * 6 + j;
                        next[s][action] = t;
                        reverse[t].push_back({short(s), (unsigned char)action});
                    }
                }
                covered |= MASK[k];
            }
            display[s] = bits;
        }
    }
    using Policy = array<array<signed char, STATES>, 16>;
    Policy optimize(const array<int, 16>& count, int products) const {
        array<double, STATES> value{}, updated{};
        Policy policy;
        // Relative value iteration, accounting for exclusion of a repeated
        // product. Terminal values choose favorable orders for later requests.
        for (int iteration = 0; iteration < 24; ++iteration) {
            updated.fill(0);
            for (int target = 0; target < 16; ++target) {
                array<double, STATES> distance;
                distance.fill(1e100);
                policy[target].fill(-1);
                priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> queue;
                for (int s = 0; s < STATES; ++s) if (display[s] == target) {
                    distance[s] = value[s];
                    queue.push({value[s], s});
                }
                while (!queue.empty()) {
                    auto [cost, s] = queue.top(); queue.pop();
                    if (cost > distance[s] + 1e-10) continue;
                    for (auto e : reverse[s]) if (distance[e.from] > cost + 1 + 1e-10) {
                        distance[e.from] = cost + 1;
                        policy[target][e.from] = e.action;
                        queue.push({cost + 1, e.from});
                    }
                }
                for (int s = 0; s < STATES; ++s) {
                    double probability = double(count[target] - (display[s] == target)) / (products - 1);
                    updated[s] += probability * distance[s];
                }
            }
            double offset = updated[0];
            for (int s = 0; s < STATES; ++s) value[s] = updated[s] - offset;
        }
        return policy;
    }
};

struct CostModel {
    int n, products;
    vector<array<int,16>> next, length;
    vector<int> display;
    vector<double> initial;
    mutable vector<double> workspace;
    CostModel(const BlockMachine& machine, int p): products(p) {
        array<int,16> uniform;
        uniform.fill(16);
        auto policy=machine.optimize(uniform,256);
        vector<int> states{0};
        array<int,720> ids;ids.fill(-1);ids[0]=0;
        for(int i=0;i<(int)states.size();i++){
            array<int,16> ns{},ls{};
            for(int t=0;t<16;t++){
                int s=states[i],steps=0;
                while(policy[t][s]>=0){s=machine.next[s][policy[t][s]];steps++;}
                if(ids[s]<0){ids[s]=states.size();states.push_back(s);}
                ns[t]=ids[s];ls[t]=steps;
            }
            next.push_back(ns);length.push_back(ls);display.push_back(machine.display[states[i]]);
        }
        n=states.size();initial.assign(n,1./n);workspace.resize(3*n);
        for(int it=0;it<40;it++){
            vector<double> updated(n);
            for(int s=0;s<n;s++)for(int t=0;t<16;t++)updated[next[s][t]]+=initial[s]/16;
            initial.swap(updated);
        }
    }
    double eval(const array<int,16>& count,int iterations=6) const {
        double probability[16];
        for(int t=0;t<16;t++)probability[t]=double(count[t])/(products-1);
        double* now=workspace.data();
        double* updated=now+n;
        double* reward=updated+n;
        for(int s=0;s<n;s++){
            now[s]=initial[s];reward[s]=0;
            for(int t=0;t<16;t++)reward[s]+=probability[t]*length[s][t];
            if(count[display[s]]) reward[s]-=double(length[s][display[s]])/(products-1);
            else reward[s]*=double(products-1)/products;
        }
        for(int it=0;it<iterations;it++){
            fill(updated,updated+n,0.);
            for(int s=0;s<n;s++){
                double mass=now[s];
                if(count[display[s]]){
                    for(int t=0;t<16;t++)updated[next[s][t]]+=mass*probability[t];
                    updated[next[s][display[s]]]-=mass/(products-1);
                } else {
                    for(int t=0;t<16;t++)updated[next[s][t]]+=mass*count[t]/products;
                }
            }
            swap(now,updated);
        }
        double result=0;
        for(int s=0;s<n;s++)result+=now[s]*reward[s];
        return result;
    }
};

struct Group {
    array<int,4> column;
    int flip=0;
    array<unsigned char,256> code;
    array<int,16> count{};
    double cost=0;
};

int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    int N,K,P,R;if(!(cin>>N>>K>>P>>R))return 0;
    vector<string>A(P);for(auto&s:A)cin>>s;
    if(N!=64||K!=192||P!=256)return 0;
    constexpr int iterations=100000;
    constexpr double search_cpu_budget=0.95;
    mt19937 rng(712367);
    BlockMachine machine;CostModel model(machine,P);
    array<Group,16>groups,bestgroups;
    double current=0;
    for(int g=0;g<16;g++){
        for(int b=0;b<4;b++)groups[g].column[b]=4*g+b;
        for(int a=0;a<P;a++){
            int c=0;for(int b=0;b<4;b++)c|=(A[a][4*g+b]-'0')<<b;
            groups[g].code[a]=c;groups[g].count[c]++;
        }
        groups[g].cost=model.eval(groups[g].count);current+=groups[g].cost;
    }
    bestgroups=groups;double best=current;
    // CPU time excludes reactive I/O waits and limits the expensive search.
    const clock_t search_start=clock();
    double timed_progress=0;
    for(int it=0;it<iterations;it++){
        if((it&127)==0){
            double used=double(clock()-search_start)/CLOCKS_PER_SEC;
            if(used>=search_cpu_budget) break;
            timed_progress=used/search_cpu_budget;
        }
        int x=rng()%64,y=rng()%64,g=x/4,h=y/4,b=x%4,c=y%4;
        auto ng=groups[g],nh=groups[h];
        bool flip=(rng()%6==0);
        if(flip){
            ng.flip^=1<<b;ng.count.fill(0);
            for(int a=0;a<P;a++)ng.code[a]^=1<<b,ng.count[ng.code[a]]++;
        } else if(g==h){
            if(b==c)continue;
            swap(ng.column[b],ng.column[c]);int diff=((ng.flip>>b)^(ng.flip>>c))&1;
            ng.flip^=(diff<<b)|(diff<<c);ng.count.fill(0);
            for(int a=0;a<P;a++){
                int diff=((ng.code[a]>>b)^(ng.code[a]>>c))&1;
                ng.code[a]^=(diff<<b)|(diff<<c);ng.count[ng.code[a]]++;
            }
        } else {
            swap(ng.column[b],nh.column[c]);int diff=((ng.flip>>b)^(nh.flip>>c))&1;
            ng.flip^=diff<<b;nh.flip^=diff<<c;ng.count.fill(0);nh.count.fill(0);
            for(int a=0;a<P;a++){
                int diff=((ng.code[a]>>b)^(nh.code[a]>>c))&1;
                ng.code[a]^=diff<<b;nh.code[a]^=diff<<c;
                ng.count[ng.code[a]]++;nh.count[nh.code[a]]++;
            }
        }
        ng.cost=model.eval(ng.count);double delta=ng.cost-groups[g].cost;
        if(!flip&&g!=h){nh.cost=model.eval(nh.count);delta+=nh.cost-groups[h].cost;}
        double temp=0.018*pow(0.015,max(double(it)/iterations,timed_progress));
        if(delta<=0||double(rng())/(double(rng.max())+1)<exp(-delta/temp)){
            groups[g]=ng;if(!flip&&g!=h)groups[h]=nh;current+=delta;
            if(current<best){best=current;bestgroups=groups;}
        }
    }
    groups=bestgroups;
    vector<BlockMachine::Policy>policy;
    vector<string>paper(K,string(N,'.'));
    for(int g=0;g<16;g++){
        policy.push_back(machine.optimize(groups[g].count,P));
        for(int k=0;k<6;k++)for(int b=0;b<4;b++)if(BlockMachine::MASK[k]>>b&1)
            paper[6*g+k][groups[g].column[b]]='0'+(((BlockMachine::BLACK[k]^groups[g].flip)>>b)&1);
    }
    for(auto&s:paper)cout<<s<<'\n';cout.flush();
    array<int,16>state{};
    for(int r=0;r<R;r++){
        int a;if(!(cin>>a)||a<0||a>=P)return 0;
        vector<pair<int,int>>moves;
        for(int g=0;g<16;g++){
            int target=groups[g].code[a];
            while(policy[g][target][state[g]]>=0){
                int action=policy[g][target][state[g]];
                moves.emplace_back(6*g+machine.order[state[g]][action/6],6*g+action%6);
                state[g]=machine.next[state[g]][action];
            }
        }
        cout<<moves.size()<<'\n';for(auto[k,d]:moves)cout<<k<<' '<<d<<'\n';cout.flush();
    }
}

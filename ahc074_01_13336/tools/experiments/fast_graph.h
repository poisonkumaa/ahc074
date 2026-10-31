#pragma once
#include <bits/stdc++.h>
using namespace std;

struct FastGraph {
    int n,bits,states,values,choices,words,max_distance=0;
    vector<string> pattern;
    vector<array<uint8_t,8>> permutations;
    vector<uint16_t> edge;
    vector<uint8_t> display,visible,distance;
    vector<double> transition;
    array<uint8_t,256> pop{};
    FastGraph(vector<string> p):n(p.size()),bits(p[0].size()),values(1<<bits),choices(n*(n-1)),words((values+63)/64),pattern(move(p)) {
        for(int i=1;i<256;i++)pop[i]=pop[i>>1]+(i&1);
        array<uint8_t,8> a{};iota(a.begin(),a.begin()+n,0);
        do{permutations.push_back(a);}while(next_permutation(a.begin(),a.begin()+n));
        states=permutations.size();edge.resize(states*choices);display.resize(states);visible.resize(states);
        int mask[8]{},color[8]{};
        for(int k=0;k<n;k++)for(int j=0;j<bits;j++)if(pattern[k][j]!='.'){
            mask[k]|=1<<j;if(pattern[k][j]=='1')color[k]|=1<<j;
        }
        for(int s=0;s<states;s++){
            int covered=0,e=0;
            for(int i=0;i<n;i++){
                int k=permutations[s][i],v=mask[k]&~covered;
                if(v)visible[s]|=1<<i;
                display[s]|=color[k]&v;covered|=mask[k];
                for(int d=0;d<n;d++)if(i!=d){
                    auto q=permutations[s];
                    if(i<d)rotate(q.begin()+i,q.begin()+i+1,q.begin()+d+1);
                    else rotate(q.begin()+d,q.begin()+i,q.begin()+i+1);
                    edge[s*choices+e++]=rank(q);
                }
            }
        }
        vector<uint64_t> reach(states*words),next(states*words);
        distance.assign(states*values,255);
        for(int s=0;s<states;s++){reach[s*words+display[s]/64]|=1ULL<<(display[s]%64);distance[s*values+display[s]]=0;}
        int left=states*(values-1);
        for(int depth=1;left;depth++){
            if(depth>32)throw runtime_error("unreachable template");
            next=reach;
            int changes=0;
            for(int s=0;s<states;s++){
                auto out=&next[s*words];
                for(int i=0;i<n;i++)if(visible[s]>>i&1){
                    int start=s*choices+i*(n-1);
                    for(int j=0;j<n-1;j++){
                        auto in=&reach[edge[start+j]*words];
                        for(int w=0;w<words;w++)out[w]|=in[w];
                    }
                }
                for(int w=0;w<words;w++){
                    uint64_t add=out[w]&~reach[s*words+w];
                    while(add){int b=__builtin_ctzll(add);add&=add-1;distance[s*values+w*64+b]=depth;changes++;}
                }
            }
            if(!changes)throw runtime_error("unreachable template");
            left-=changes;max_distance=depth;reach.swap(next);
        }
    }
    int rank(const array<uint8_t,8>&a)const{
        int r=0,used=0;
        for(int i=0;i<n;i++){int x=a[i];r=r*(n-i)+x-pop[used&((1<<x)-1)];used|=1<<x;}
        return r;
    }
    int dest(int e)const{int i=e/(n-1),j=e%(n-1);return j+(j>=i);}
    vector<int> quality(const vector<int>&frequency)const{
        vector<int> q(states);
        for(int s=0;s<states;s++)for(int t=0;t<values;t++)q[s]+=frequency[t]*distance[s*values+t];
        return q;
    }
    int greedy(int s,int t,const vector<int>&q)const{
        int best=INT_MAX,action=-1,d=distance[s*values+t];
        for(int i=0;i<n;i++)if(visible[s]>>i&1)for(int j=0;j<n-1;j++){
            int e=i*(n-1)+j,v=edge[s*choices+e];
            if(distance[v*values+t]+1==d && q[v]<best)best=q[v],action=e;
        }
        return action;
    }
    struct Path{int state;array<uint8_t,16> action{};};
    vector<int> path(int s,int t,const vector<int>&q,int width=8)const{
        int d=distance[s*values+t];if(!d)return {};
        vector<Path> beam(1);beam[0].state=s;
        struct Candidate {int state,parent,action,score;};
        static vector<int>seen;static int stamp=0;
        if((int)seen.size()<states)seen.resize(states);
        for(int step=0;step<d;step++){
            ++stamp;vector<Candidate>next;
            for(int p=0;p<(int)beam.size();p++){
                int cur=beam[p].state;
                for(int i=0;i<n;i++)if(visible[cur]>>i&1)for(int j=0;j<n-1;j++){
                    int e=i*(n-1)+j,v=edge[cur*choices+e];
                    if(distance[v*values+t]!=d-step-1 || seen[v]==stamp)continue;
                    seen[v]=stamp;next.push_back({v,p,e,q[v]});
                }
            }
            int count=min(width,(int)next.size());
            partial_sort(next.begin(),next.begin()+count,next.end(),[](auto a,auto b){return a.score<b.score || (a.score==b.score&&a.state<b.state);});
            vector<Path> nb;nb.reserve(count);
            for(int i=0;i<count;i++){
                auto c=next[i];Path p=beam[c.parent];p.state=c.state;p.action[step]=c.action;nb.push_back(p);
            }
            beam.swap(nb);
        }
        vector<int> result(d);for(int i=0;i<d;i++)result[i]=beam[0].action[i];return result;
    }
    void build_transition(){
        auto q=quality(vector<int>(values,1));
        vector<int>visit(states),total(values);
        uint64_t rnd=712367;int s=0;
        for(int it=0;it<40000;it++){
            rnd^=rnd<<7;rnd^=rnd>>9;int t=rnd%values;
            while(display[s]!=t){int e=greedy(s,t,q);s=edge[s*choices+e];}
            if(it>=1000)visit[s]++,total[t]++;
        }
        transition.assign(values*values,0);
        for(int s=0;s<states;s++)if(visit[s]){
            double weight=double(visit[s])/total[display[s]];
            for(int t=0;t<values;t++)transition[display[s]*values+t]+=weight*distance[s*values+t];
        }
    }
};

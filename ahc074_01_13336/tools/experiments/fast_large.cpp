#include "fast_graph.h"
struct Random {uint64_t x=712367;uint64_t next(){x^=x<<7;x^=x>>9;return x;}int pick(int n){return next()%n;}double real(){return (next()>>11)*0x1p-53;}};
struct Group {FastGraph*graph;int begin,base,state=0;vector<int>freq,quality;};
int main(int argc,char**argv){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    int N,K,P,R;if(!(cin>>N>>K>>P>>R))return 0;
    clock_t started=clock();vector<uint64_t>A(P);string s;
    for(auto&a:A){cin>>s;for(int i=0;i<N;i++)if(s[i]=='1')a|=1ULL<<i;}
    vector<string>pat={"0....00","10...00",".10.000","1110000",".111000","11..100",".....10","......1"};
    if(argc>1){ifstream f(argv[1]);int n,b;f>>n>>b;pat.resize(n);for(auto&s:pat)f>>s;}
    FastGraph large(pat),small({"0","1"});large.build_transition();small.build_transition();
    vector<Group>groups;int used=0;
    for(int i=0;i<9;i++){groups.push_back({&large,i*7,used});used+=large.n;}
    groups.push_back({&small,63,used});
    array<int,64>column,flip{};iota(column.begin(),column.end(),0);int belongs[64];
    for(int g=0;g<(int)groups.size();g++)for(int b=0;b<groups[g].graph->bits;b++)belongs[groups[g].begin+b]=g;
    auto frequency=[&](int g){auto&gr=groups[g];vector<int>f(gr.graph->values);for(auto a:A){int t=0;for(int b=0;b<gr.graph->bits;b++){int i=gr.begin+b;t|=(((a>>column[i])&1)^flip[i])<<b;}f[t]++;}return f;};
    auto cost=[&](int g,const vector<int>&f){auto&gr=*groups[g].graph;double value=0;for(int a=0;a<gr.values;a++)if(f[a])for(int b=0;b<gr.values;b++)value+=double(f[a])*f[b]*gr.transition[a*gr.values+b];return value/(P*double(P));};
    vector<vector<int>>frequency_now;vector<double>score;double current=0;
    for(int g=0;g<(int)groups.size();g++){frequency_now.push_back(frequency(g));score.push_back(cost(g,frequency_now.back()));current+=score.back();}
    auto bestcolumn=column,bestflip=flip;double best=current;Random rnd;
    constexpr int iterations=16000;
    double search_start=double(clock()-started)/CLOCKS_PER_SEC;
    for(int it=0;it<iterations;it++){
        double elapsed=double(clock()-started)/CLOCKS_PER_SEC;
        if((it&127)==0&&elapsed>1.05)break;
        int x=rnd.pick(64),y=rnd.pick(64);bool invert=rnd.pick(5)==0;int gx=belongs[x],gy=invert?gx:belongs[y];
        if(invert)flip[x]^=1;else{swap(column[x],column[y]);swap(flip[x],flip[y]);}
        auto fx=frequency(gx);double sx=cost(gx,fx),delta=sx-score[gx];vector<int>fy;double sy=0;
        if(gx!=gy){fy=frequency(gy);sy=cost(gy,fy);delta+=sy-score[gy];}
        double progress=max(double(it)/iterations,(elapsed-search_start)/max(0.01,1.05-search_start));
        double temperature=.035*pow(.012,min(1.,progress));
        if(delta<=0||rnd.real()<exp(-delta/temperature)){
            current+=delta;frequency_now[gx]=move(fx);score[gx]=sx;
            if(gx!=gy){frequency_now[gy]=move(fy);score[gy]=sy;}
            if(current<best){best=current;bestcolumn=column;bestflip=flip;}
        }else if(invert)flip[x]^=1;else{swap(column[x],column[y]);swap(flip[x],flip[y]);}
    }
    column=bestcolumn;flip=bestflip;vector<string>design(K,string(N,'.'));
    for(int g=0;g<(int)groups.size();g++){
        auto&gr=groups[g];gr.freq=frequency(g);gr.quality=gr.graph->quality(gr.freq);
        for(int k=0;k<gr.graph->n;k++)for(int b=0;b<gr.graph->bits;b++){
            char c=gr.graph->pattern[k][b];if(c=='.')continue;int i=gr.begin+b;
            design[gr.base+k][column[i]]='0'+((c-'0')^flip[i]);
        }
    }
    cerr<<"states="<<large.states<<" maxdist="<<large.max_distance<<" cost="<<best<<" prep="<<double(clock()-started)/CLOCKS_PER_SEC<<endl;
    for(auto&s:design)cout<<s<<'\n';cout.flush();
    for(int r=0;r<R;r++){
        int a;if(!(cin>>a)||a<0||a>=P)return 0;vector<pair<int,int>>moves;
        for(auto&g:groups){
            int target=0;for(int b=0;b<g.graph->bits;b++){int i=g.begin+b;target|=(((A[a]>>column[i])&1)^flip[i])<<b;}
            auto path=g.graph->path(g.state,target,g.quality,8);
            for(int e:path){int i=e/(g.graph->n-1);moves.emplace_back(g.base+g.graph->permutations[g.state][i],g.base+g.graph->dest(e));g.state=g.graph->edge[g.state*g.graph->choices+e];}
        }
        cout<<moves.size()<<'\n';for(auto[k,d]:moves)cout<<k<<' '<<d<<'\n';cout.flush();
    }
    cerr<<"cpu="<<double(clock()-started)/CLOCKS_PER_SEC<<endl;
}

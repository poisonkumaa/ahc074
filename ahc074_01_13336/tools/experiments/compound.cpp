#define main previous_main
#include "block_main.cpp"
#undef main
using U=uint64_t;
struct Paper { U mask=0,black=0; vector<pair<int,int>> part; };
struct Solver {
    BlockMachine machine;
    array<array<int,720>,16> distance;
    array<array<array<int,6>,6>,720> transition;
    array<array<int,6>,720> rank;
    array<int,192> order;
    vector<Paper>papers;
    vector<string>A;
    int group_size;
    mt19937 rng{712367};
    Solver(vector<string>a,int span,int init): A(a),group_size(span) {
        for(int s=0;s<720;s++)for(int i=0;i<6;i++){
            rank[s][machine.order[s][i]]=i;
            for(int j=0;j<6;j++){
                auto p=machine.order[s];
                if(i<j)rotate(p.begin()+i,p.begin()+i+1,p.begin()+j+1);
                else if(i>j)rotate(p.begin()+j,p.begin()+i,p.begin()+i+1);
                transition[s][i][j]=machine.rank(p);
            }
        }
        for(int t=0;t<16;t++){
            distance[t].fill(1000);queue<int>q;
            for(int s=0;s<720;s++)if(machine.display[s]==t)distance[t][s]=0,q.push(s);
            while(!q.empty()){int s=q.front();q.pop();for(auto e:machine.reverse[s])if(distance[t][e.from]>distance[t][s]+1)distance[t][e.from]=distance[t][s]+1,q.push(e.from);}
        }
        auto add=[&](vector<pair<int,int>>part){Paper p;p.part=part;for(auto[g,k]:part){p.mask|=U(machine.MASK[k])<<(4*g);p.black|=U(machine.BLACK[k])<<(4*g);}papers.push_back(p);};
        for(int g=0;g<16;g++)for(int k=0;k<6;k++)add({{g,k}});
        for(int i=0;i<96;i++){
            vector<int>gs(16);iota(gs.begin(),gs.end(),0);shuffle(gs.begin(),gs.end(),rng);
            vector<pair<int,int>>part;
            for(int j=0;j<span;j++)part.push_back({gs[j],int(rng()%6)});
            add(part);
        }
        iota(order.begin(),order.end(),0);
        if(init)shuffle(order.begin(),order.end(),rng);
        else for(int i=0;i<96;i++){order[2*i]=i;order[2*i+1]=96+i;}
    }
    struct Choice {int from=-1,to=-1,score=INT_MAX;};
    Choice best_move(U target,int &oldscore,U &value) {
        int first[16][6],second[16][6],state[16]{},t[16];
        for(int g=0;g<16;g++){fill(first[g],first[g]+6,192);fill(second[g],second[g]+6,192);t[g]=(target>>(g*4))&15;}
        for(int i=0;i<192;i++)for(auto[g,k]:papers[order[i]].part){if(first[g][k]==192)first[g][k]=i;else if(second[g][k]==192)second[g][k]=i;}
        int h=0;
        for(int g=0;g<16;g++){array<unsigned char,6>p={0,1,2,3,4,5};sort(p.begin(),p.end(),[&](int a,int b){return first[g][a]<first[g][b];});state[g]=machine.rank(p);h+=distance[t[g]][state[g]];}
        U premask[193]{},precorrect[193]{};value=0;int runs=0;
        for(int i=0;i<192;i++){auto&p=papers[order[i]];U wrong=(p.black^target)&p.mask;value|=p.black&~premask[i];premask[i+1]=premask[i]|p.mask;precorrect[i+1]=precorrect[i]|(p.mask&~wrong);runs+=__builtin_popcountll(wrong&~precorrect[i]);}
        oldscore=h*4+runs;
        Choice best;
        for(int i=0;i<192;i++){
            auto &p=papers[order[i]];if(!(p.mask&~premask[i]))continue;
            int diff[193]{},base=0;
            for(auto[g,k]:p.part){
                int s=state[g],r=rank[s][k],cap=first[g][k]==i?second[g][k]:first[g][k];
                cap-=cap>i;
                int before=distance[t[g]][s],last=distance[t[g]][transition[s][r][0]],at=0;
                base+=last-before;
                for(int j=0;j<6;j++){
                    int kk=machine.order[s][j];if(kk==k)continue;
                    int pos=first[g][kk]-(first[g][kk]>i);if(pos>=cap)break;
                    int next=distance[t[g]][transition[s][r][++at]];
                    diff[pos+1]+=next-last;last=next;
                }
            }
            int f[192],cur=base;
            for(int j=0;j<192;j++){cur+=diff[j];f[j]=(h+cur)*4;}
            U pw=(p.black^target)&p.mask,pc=p.mask&~pw;
            int hh=runs;U correct=precorrect[i];
            for(int j=i+1;j<192;j++){
                auto&q=papers[order[j]];U qw=(q.black^target)&q.mask,qc=q.mask&~qw;
                hh+=__builtin_popcountll(pc&qw&~correct)-__builtin_popcountll(pw&qc&~correct);correct|=qc;
                int sc=f[j]+hh;if(sc<best.score||(sc==best.score&&j>best.to))best={i,j,sc};
            }
            hh=runs;
            for(int j=i-1;j>=0;j--){
                auto&q=papers[order[j]];U qw=(q.black^target)&q.mask,qc=q.mask&~qw;
                hh+=__builtin_popcountll(pw&qc&~precorrect[j])-__builtin_popcountll(pc&qw&~precorrect[j]);
                int sc=f[j]+hh;if(sc<best.score)best={i,j,sc};
            }
        }
        return best;
    }
    void move(int from,int to){if(from<to)rotate(order.begin()+from,order.begin()+from+1,order.begin()+to+1);else rotate(order.begin()+to,order.begin()+from,order.begin()+from+1);}
    int solve(U target){
        int steps=0;
        for(;steps<384;steps++){
            int old;U value;auto c=best_move(target,old,value);
            if(value==target)break;
            if(c.score>=old){
                U covered=0;
                for(int i=0;i<192;i++){
                    auto &p=papers[order[i]];
                    if((p.black^target)&p.mask&~covered){c={i,191,0};break;}
                    covered|=p.mask;
                }
            }
            move(c.from,c.to);
        }
        return steps;
    }
};
int main(int argc,char**argv){
    int N,K,P,R;cin>>N>>K>>P>>R;vector<string>A(P);for(auto&s:A)cin>>s;
    int span=argc>1?atoi(argv[1]):4,init=argc>2?atoi(argv[2]):1;
    auto start=chrono::steady_clock::now();Solver solver(A,span,init);
    int sum=0,errors=0,maxlen=0;
    for(int r=0;r<128;r++){int a;cin>>a;U target=0;for(int j=0;j<64;j++)if(A[a][j]=='1')target|=1ULL<<j;int steps=solver.solve(target);sum+=steps;maxlen=max(maxlen,steps);errors+=steps==384;}
    cerr<<"span="<<span<<" avg="<<sum/128.<<" errors="<<errors<<" max="<<maxlen<<" sec="<<chrono::duration<double>(chrono::steady_clock::now()-start).count()<<endl;
}

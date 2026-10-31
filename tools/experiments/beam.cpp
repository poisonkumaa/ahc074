#include <bits/stdc++.h>
using namespace std;using U=uint64_t;
int pc(U x){return __builtin_popcountll(x);}struct Card{U m,b;};struct Move{int k,d;};
struct State{array<uint8_t,192>o;vector<Move>path;U val;int h;};
struct Cand{int src,i,j,score;U val;double tie;int h;};
mt19937 rng(712367);
int main(int argc,char**argv){
 int N,K,P,R;cin>>N>>K>>P>>R;vector<U>A(P);string s;for(U&a:A){cin>>s;for(int j=0;j<N;j++)if(s[j]=='1')a|=1ULL<<j;}
 int sz=argc>1?atoi(argv[1]):6, BW=argc>2?atoi(argv[2]):32,mode=argc>3?atoi(argv[3]):0;
 vector<Card>C;
 if(mode==0)for(int j=0;j<N;j++){C.push_back({1ULL<<j,0});C.push_back({1ULL<<j,1ULL<<j});}
 if(mode==5)for(int j=0;j<N;j++)C.push_back({1ULL<<j,1ULL<<j});
 if(mode==3){ifstream f("tools/experiments/cards8.txt");while(f>>s){U m=0,b=0;for(int j=0;j<N;j++)if(s[j]!='.')m|=1ULL<<j,b|=U(s[j]=='1')<<j;C.push_back({m,b});}}
 if(mode==4){vector<int>slots;for(int i=0;i<K*sz;i++)slots.push_back(i%(N*2));shuffle(slots.begin(),slots.end(),rng);for(int i=0;i<K;i++){U m=0,b=0;for(int j=0;j<sz;j++){int p=i*sz+j;for(int t=0;t<100 && (m>>(slots[p]/2)&1);t++){int q=p+rng()%(slots.size()-p);swap(slots[p],slots[q]);}int x=slots[p];m|=1ULL<<(x/2);if(x&1)b|=1ULL<<(x/2);}C.push_back({m,b});}}
 while(C.size()<K){U m=0,b=A[rng()%P];while(pc(m)<sz)m|=1ULL<<(rng()%N);if(mode==5)b=0;C.push_back({m,b&m});}
 State current;for(int i=0;i<K;i++)current.o[i]=i;
 auto eval=[&](State &s){U m=0,b=0;for(int i=0;i<K;i++){auto c=C[s.o[i]];b|=c.b&~m;m|=c.m;}return s.val=b;};eval(current);
 long long total=0;int errors=0,maxlen=0;auto start=chrono::steady_clock::now();
 for(int r=0;r<min(R,128);r++){
  int a;cin>>a;U target=A[a];current.path.clear();current.h=0;U cor=0;for(int i=0;i<K;i++){auto c=C[current.o[i]];U wrong=(c.b^target)&c.m;current.h+=pc(wrong&~cor);cor|=c.m&~wrong;}vector<State>beam{current};State result;bool solved=false;
  unordered_set<U>seen;
  for(int depth=0;depth<64;depth++){
   vector<Cand>cs;
   for(int si=0;si<(int)beam.size();si++){
    auto &st=beam[si];if(st.val==target){result=st;solved=true;break;}
    U pre[193]={},preCor[193]={};for(int i=0;i<K;i++){auto c=C[st.o[i]];pre[i+1]=pre[i]|c.m;preCor[i+1]=preCor[i]|(c.m&~(c.b^target));}
    for(int i=0;i<K;i++){
     auto c=C[st.o[i]];U vis=c.m&~pre[i];if(!vis)continue;
     U beneath=0,covered=0,prev=st.val;U cw=(c.b^target)&c.m,cc=c.m&~cw,cor=preCor[i];int hh=st.h,prevh=hh;
     for(int j=i+1;j<K;j++){
      auto d=C[st.o[j]];beneath|=d.b&~covered;covered|=d.m;U dw=(d.b^target)&d.m,dc=d.m&~dw;hh+=pc(cc&dw&~cor)-pc(cw&dc&~cor);cor|=dc;
      U val=(st.val&~vis)|(vis&(beneath|(c.b&~covered)));
      if(val!=prev||hh!=prevh){cs.push_back({si,i,j,pc(val^target)*2+hh,val,double(rng()%1000)*1e-5,hh});prev=val;prevh=hh;}
      if((c.m&~cor)==0)break;
     }
     prev=st.val;hh=prevh=st.h;
     for(int j=i-1;j>=0;j--){auto d=C[st.o[j]];U dw=(d.b^target)&d.m,dc=d.m&~dw;hh+=pc(cw&dc&~preCor[j])-pc(cc&dw&~preCor[j]);U changed=c.m&~pre[j];U val=(st.val&~changed)|(c.b&changed);if(val!=prev||hh!=prevh){cs.push_back({si,i,j,pc(val^target)*2+hh,val,double(rng()%1000)*1e-5,hh});prev=val;prevh=hh;}}
    }
   }
   if(solved)break;
   sort(cs.begin(),cs.end(),[](auto &a,auto &b){if(a.score!=b.score)return a.score<b.score;return a.tie<b.tie;});
   vector<State>next;unordered_map<U,int>nvals;
   for(auto c:cs){
    if(nvals[c.val]>=2)continue;
    State ns=beam[c.src];int k=ns.o[c.i];if(c.i<c.j)rotate(ns.o.begin()+c.i,ns.o.begin()+c.i+1,ns.o.begin()+c.j+1);else rotate(ns.o.begin()+c.j,ns.o.begin()+c.i,ns.o.begin()+c.i+1);
    U h=0;for(int i=0;i<K;i++)h=h*1000000007+ns.o[i]+1;if(!seen.insert(h).second)continue;
    nvals[c.val]++;ns.val=c.val;ns.h=c.h;if(eval(ns)!=c.val){cerr<<"BAD SIMULATION"<<endl;return 1;}U co=0;int h2=0;for(int i=0;i<K;i++){auto x=C[ns.o[i]];U w=(x.b^target)&x.m;h2+=pc(w&~co);co|=x.m&~w;}if(h2!=ns.h){cerr<<"BAD H "<<h2<<" "<<ns.h<<endl;return 2;}ns.path.push_back({k,c.j});next.push_back(move(ns));if(next.size()>=BW)break;
   }
   if(next.empty())break;beam=move(next);
  }
  if(!solved){errors++;result=beam[0];if(r<5)cerr<<"residual="<<pc(result.val^target)<<endl;}current=move(result);total+=current.path.size();maxlen=max(maxlen,(int)current.path.size());
 }
 cerr<<"size="<<sz<<" width="<<BW<<" mode="<<mode<<" avg="<<total/128.<<" errors="<<errors<<" max="<<maxlen<<" sec="<<chrono::duration<double>(chrono::steady_clock::now()-start).count()<<endl;
}

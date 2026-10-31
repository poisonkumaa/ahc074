#pragma once
struct AdjacentDesigner {
 int P,K,N=64,INF=10000,importance;
 vector<U64>&A;vector<PrefixPaper>&papers;
 vector<vector<int>>level,start;
 vector<array<int,65>>F,B;
 AdjacentDesigner(vector<U64>&aa,vector<PrefixPaper>&pp,int im):P(aa.size()),K(pp.size()),importance(im),A(aa),papers(pp),level(65),start(P,vector<int>(K)),F(P),B(P){for(int k=0;k<K;k++)level[papers[k].len].push_back(k);update();}
 void update(){for(int v=0;v<P;v++)for(int k=0;k<K;k++){U64 d=(A[v]^papers[k].color)&PrefixDP::low(papers[k].len);start[v][k]=d?64-__builtin_clzll(d):0;}}
 int distances(){int total=0;for(int v=0;v<P;v++){auto&f=F[v];auto&b=B[v];f.fill(INF);b.fill(INF);f[0]=0;b[N]=0;for(int l=1;l<=N;l++)for(int k:level[l])for(int j=start[v][k];j<l;j++)f[l]=min(f[l],f[j]+1);for(int l=N;l>0;l--)for(int k:level[l])for(int j=start[v][k];j<l;j++)b[j]=min(b[j],b[l]+1);total+=f[N];}return total;}
 bool fit(int l){
  int amount=level[l].size();if(!amount)return true;
  distances();
  struct Node{int child[2]{-1,-1},weight=0,sample=0;array<int,9>dp,split;Node(){dp.fill(-1000000000);dp[0]=0;split.fill(0);}};
  vector<Node>trie(1);
  for(int v=0;v<P;v++){
   auto f=F[v];f[l]=INF;for(int upper=l+1;upper<=N;upper++){f[upper]=INF;for(int k:level[upper])for(int j=start[v][k];j<upper;j++)f[upper]=min(f[upper],f[j]+1);}
   int bypass=f[N],bestprefix=INF,previous=0,x=0;
   for(int d=1;d<=l;d++){
    int bit=A[v]>>(l-d)&1;if(trie[x].child[bit]<0){int ni=trie.size();trie[x].child[bit]=ni;trie.emplace_back();}x=trie[x].child[bit];trie[x].sample=v;
    bestprefix=min(bestprefix,F[v][l-d]);int route=bestprefix+1+B[v][l];int benefit=0;
    if(route<INF){if(bypass>=INF)benefit=1000000+importance*(N+1-route);else benefit=importance*max(0,bypass-route);}
    trie[x].weight+=benefit-previous+1;previous=benefit;
   }
  }
  for(int x=int(trie.size())-1;x>=0;x--){auto&p=trie[x];int a=p.child[0],b=p.child[1];if(a<0&&b<0){p.dp[1]=p.weight;continue;}
   for(int q=1;q<=amount;q++)for(int left=0;left<=q;left++){int right=q-left;if((a<0&&left)||(b<0&&right))continue;int value=(a<0?0:trie[a].dp[left])+(b<0?0:trie[b].dp[right])+p.weight;if(value>p.dp[q])p.dp[q]=value,p.split[q]=left;}
  }
  if(trie[0].dp[amount]<0)return false;
  vector<U64>pattern;function<void(int,int)>collect=[&](int x,int q){if(!q)return;auto&p=trie[x];if(p.child[0]<0&&p.child[1]<0){pattern.push_back(A[p.sample]&PrefixDP::low(l));return;}int left=p.split[q];if(left)collect(p.child[0],left);if(q-left)collect(p.child[1],q-left);};collect(0,amount);
  for(int q=0;q<amount;q++)papers[level[l][q]].color=pattern[q];update();
  distances();for(int v=0;v<P;v++)if(F[v][N]>=INF)return false;return true;
 }
 static U64 swapped(U64 v,int x){U64 b=((v>>x)^(v>>(x+1)))&1;return v^(b<<x)^(b<<(x+1));}
 int optimize(vector<int>&cols,vector<int>&flip,int iterations,int low,int neutral,double seconds,int radius=0){
  int current=distances(),initial=current,accepted=0,invalid=0,improved=0;mt19937_64 rng(93313);
  vector<int>todo;for(int x=low;x<63;x++)todo.push_back(x);shuffle(todo.begin(),todo.end(),rng);
  for(int it=0;it<iterations;it++){
   if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>seconds)break;
   if(it%todo.size()==0)shuffle(todo.begin(),todo.end(),rng);int x=todo[it%todo.size()],l=x+1;
   auto oldA=A;auto oldP=papers;
   for(U64&v:A)v=swapped(v,x);for(auto&p:papers)if(p.len>l)p.color=swapped(p.color,x);
   update();bool valid=fit(l);
   if(radius){for(int u=min(N,l+radius);u>=max(1,l-radius);u--)fit(u);valid=true;distances();for(int v=0;v<P;v++)valid&=F[v][N]<INF;}
   int next=valid?distances():INF*P;
   if(valid&&(next<current||(next==current&&int(rng()%100)<neutral))){
    improved+=next<current;current=next;accepted++;swap(cols[x],cols[x+1]);swap(flip[x],flip[x+1]);
   }else{invalid+=!valid;A=move(oldA);papers=move(oldP);update();}
  }
  cerr<<"Adjacent "<<double(initial)/P<<" -> "<<double(current)/P<<" accepted="<<accepted<<" improved="<<improved<<" invalid="<<invalid<<" CPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";return current;
 }
};

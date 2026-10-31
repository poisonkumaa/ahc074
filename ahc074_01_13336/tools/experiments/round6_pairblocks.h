#pragma once
struct PairBlocksDP {
 struct State{int group,subset;U64 covered;};
 struct Edge{uint8_t paper,next;};
 int K,N=64,S,B,epoch=0;vector<PrefixPaper>papers;
 vector<int>base,size,group,category,reward,position;
 vector<array<int,4>>state_index;vector<State>states;
 vector<U64>mask;
 vector<array<int,193>>allcnt,catcnt,work,parent,which,stamp;
 vector<vector<vector<Edge>>>candidate;
 PairBlocksDP(const vector<PrefixPaper>&p,int first_pair):K(p.size()),papers(p),reward(K,16),position(K){
  for(int j=0;j<N;){int n=j<first_pair?1:2;base.push_back(j);size.push_back(n);j+=n;}
  B=base.size();state_index.resize(B);states.push_back({-1,0,0});
  for(int g=0;g<B;g++)for(int s=1;s<(1<<size[g]);s++){state_index[g][s]=states.size();states.push_back({g,s,PrefixDP::low(base[g])|(U64(s)<<base[g])});}
  S=states.size();mask.resize(K);group.resize(K);category.resize(K);
  for(int k=0;k<K;k++){int endpoint=p[k].len-1,g=upper_bound(base.begin(),base.end(),endpoint)-base.begin()-1;group[k]=g;category[k]=endpoint-base[g];mask[k]=PrefixDP::low(base[g])|(1ULL<<endpoint);}
  allcnt.resize(B+1);catcnt.resize(B*2);work.resize(S);parent.resize(S);which.resize(S);stamp.resize(S);
 }
 bool public_data(const vector<U64>&A){
  candidate.resize(A.size(),vector<vector<Edge>>(S));
  for(int v=0;v<int(A.size());v++){
   vector<char>reach(S);reach[0]=true;
   for(int s=0;s<S;s++)for(int k=0;k<K;k++){
    int g=group[k],c=category[k];const auto&t=states[s];if(g<t.group||(g==t.group&&(t.subset>>c&1)))continue;
    if((A[v]^papers[k].color)&mask[k]&~t.covered)continue;
    int ns=state_index[g][g==t.group?(t.subset|(1<<c)):(1<<c)];candidate[v][s].push_back({uint8_t(k),uint8_t(ns)});if(reach[s])reach[ns]=true;
   }
   if(!reach.back()){cerr<<"PAIR UNREACHABLE "<<v<<"\n";return false;}
  }
  return true;
 }
 vector<int>solve(const vector<int>&order,int target,int&distance,vector<int>&virtual_length){
  for(int i=0;i<K;i++)position[order[i]]=i;
  for(int g=0;g<=B;g++){allcnt[g][0]=0;for(int i=0;i<K;i++)allcnt[g][i+1]=allcnt[g][i]+(group[order[i]]>=g);}
  for(int g=0;g<B;g++)for(int c=0;c<size[g];c++){auto&out=catcnt[g*2+c];out[0]=0;for(int i=0;i<K;i++){int k=order[i];out[i+1]=out[i]+(group[k]>g||(group[k]==g&&category[k]==c));}}
  epoch++;int active[97][193],count[97]{};work[0][0]=0;stamp[0][0]=epoch;active[0][count[0]++]=0;
  for(int s=0;s<S;s++){
   if(!count[s])continue;sort(active[s],active[s]+count[s]);const auto&t=states[s];
   const array<int,193>*upper=&allcnt[t.group+1];if(t.group>=0&&size[t.group]==2&&t.subset!=3)upper=&catcnt[t.group*2+(t.subset==1?1:0)];
   for(auto edge:candidate[target][s]){
    int k=edge.paper,ns=edge.next,g=group[k],q=position[k];
    const auto&moved=g==t.group?catcnt[g*2+category[k]]:allcnt[g];
    for(int ix=0;ix<count[s];ix++){
     int h=active[s][ix],add=0;if(q>=h){int m=moved[q]-moved[h],u=(*upper)[q]-(*upper)[h];assert(u>=m);add=m+(u>m);}
     int nh=max(h,q+1),value=work[s][h]+add*65536-reward[k];
     if(stamp[ns][nh]!=epoch){stamp[ns][nh]=epoch;work[ns][nh]=1000000000;active[ns][count[ns]++]=nh;}
     if(value<work[ns][nh]){work[ns][nh]=value;parent[ns][nh]=s*(K+1)+h;which[ns][nh]=k;}
    }
   }
  }
  int s=S-1;if(!count[s]){distance=1000000;return{};}int h=active[s][0];for(int ix=1;ix<count[s];ix++){int x=active[s][ix];if(work[s][x]<work[s][h]||(work[s][x]==work[s][h]&&x<h))h=x;}
  distance=(work[s][h]+65535)/65536;vector<int>chosen;
  while(s){int k=which[s][h],prev=parent[s][h];chosen.push_back(k);s=prev/(K+1);h=prev%(K+1);}reverse(chosen.begin(),chosen.end());
  vector<int>orientation(B,-1);for(int k:chosen)if(orientation[group[k]]<0)orientation[group[k]]=category[k];
  virtual_length.resize(K);for(int k=0;k<K;k++){int g=group[k],first=max(0,orientation[g]);virtual_length[k]=base[g]+1+(size[g]==2&&category[k]!=first);}
  return chosen;
 }
};

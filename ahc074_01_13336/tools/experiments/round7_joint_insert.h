#pragma once
static int joint_insert_limit=10;
static long long joint_insert_changed=0,joint_insert_dp=0,joint_insert_fallback=0,joint_insert_gain=0,joint_insert_strict=0;
inline vector<int> joint_insert_final(const vector<int>&order,const vector<int>&length,const vector<char>&selected,const vector<U64>*future){
 auto baseline=direct_final(order,length,selected,future);if(!future)return baseline;
 int K=order.size(),N=*max_element(length.begin(),length.end());vector<int>owner(N+1,-1);int root=-1;
 for(int k=0;k<K;k++)if(selected[k])owner[length[k]]=k;
 for(int l=N;l>=0;l--){if(owner[l]>=0)root=owner[l];owner[l]=root;}
 vector<char>mandatory(K);int suffix=N+1;for(int i=K-1;i>=0;i--){int k=order[i];mandatory[k]=length[k]>=suffix;if(selected[k])suffix=min(suffix,length[k]);}
 vector<vector<int>>fixed(K),moving(K),old(K);
 for(int k:order)if(!selected[k]){if(mandatory[k])moving[owner[length[k]]].push_back(k);else fixed[owner[length[k]]].push_back(k);}
 root=-1;for(int k:baseline){if(selected[k])root=k;else old[root].push_back(k);}
 vector<int>result;result.reserve(K);
 for(int l=1;l<=N;l++){
  int k=owner[l];if(k<0||!selected[k]||length[k]!=l)continue;result.push_back(k);auto&v=fixed[k];auto&add=moving[k];int M=v.size(),m=add.size();
  if(m<=1||m>joint_insert_limit||m>=18){result.insert(result.end(),old[k].begin(),old[k].end());continue;}
  vector<int>types;for(int x:v)types.push_back(length[x]);for(int x:add)types.push_back(length[x]);sort(types.begin(),types.end());types.erase(unique(types.begin(),types.end()),types.end());int T=types.size(),S=1<<m;int type[192];
  for(int x:v)type[x]=lower_bound(types.begin(),types.end(),length[x])-types.begin();for(int x:add)type[x]=lower_bound(types.begin(),types.end(),length[x])-types.begin();
  vector<U64>suffixmask((M+1)*T),unions(S);vector<uint8_t>suffixless((M+1)*T);vector<int>le(T),lt(T);
  for(int t=0;t<T;t++)for(int a=0;a<m;a++){if(length[add[a]]<=types[t])le[t]|=1<<a;if(length[add[a]]<types[t])lt[t]|=1<<a;}
  for(int s=1;s<S;s++){int a=__builtin_ctz(unsigned(s));unions[s]=unions[s^(1<<a)]|(*future)[add[a]];}
  for(int i=M-1;i>=0;i--)for(int t=0;t<T;t++){suffixmask[i*T+t]=suffixmask[(i+1)*T+t]|(length[v[i]]<=types[t]?(*future)[v[i]]:0ULL);suffixless[i*T+t]=suffixless[(i+1)*T+t]+(length[v[i]]<types[t]);}
  vector<int>dp((M+1)*S,1000000000);vector<uint8_t>parent((M+1)*S);dp[M*S]=0;
  auto cost=[&](int i,int s,int t){return __builtin_popcountll(suffixmask[i*T+t]|unions[s&le[t]])*65536+suffixless[i*T+t]+__builtin_popcount(unsigned(s&lt[t]));};
  for(int i=M;i>=0;i--)for(int s=0;s<S;s++){
   int value=dp[i*S+s];if(value==1000000000)continue;
   if(i){int next=(i-1)*S+s,nv=value+cost(i,s,type[v[i-1]]);if(nv<dp[next]){dp[next]=nv;parent[next]=255;}}
   for(int rem=(S-1)^s;rem;rem&=rem-1){int a=__builtin_ctz(unsigned(rem)),next=i*S+(s|(1<<a)),nv=value+cost(i,s,type[add[a]]);if(nv<dp[next]){dp[next]=nv;parent[next]=a;}}
  }
  int oldcost=0;for(int i=0;i<int(old[k].size());i++){int x=old[k][i],inv=0;U64 mask=0;for(int j=i+1;j<int(old[k].size());j++){int y=old[k][j];if(length[y]<=length[x])mask|=(*future)[y];inv+=length[y]<length[x];}oldcost+=__builtin_popcountll(mask)*65536+inv;}
  assert(dp[S-1]<=oldcost);joint_insert_gain+=(oldcost/65536)-(dp[S-1]/65536);joint_insert_strict+=dp[S-1]/65536<oldcost/65536;
  vector<int>choice;int i=0,s=S-1;while(i<M||s){int a=parent[i*S+s];if(a==255)choice.push_back(v[i++]);else{choice.push_back(add[a]);s^=1<<a;}}
  if(choice!=old[k])joint_insert_changed++;joint_insert_dp++;result.insert(result.end(),choice.begin(),choice.end());
 }
 assert(result.size()==order.size());return result;
}

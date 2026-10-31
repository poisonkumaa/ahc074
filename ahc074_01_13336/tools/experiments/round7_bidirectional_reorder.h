#pragma once
inline vector<pair<int,int>>bidirectional_reorder(vector<int>&order,const vector<U64>&masks,const vector<BidirectionalPaper>&papers,const vector<int>&goal,U64 target){
 int K=order.size(),S=goal.size();assert(S<=20);vector<int>index(K,-1);for(int i=0;i<S;i++)index[goal[i]]=i;
 vector<bitset<192>>required(S);U64 covered=0;
 for(int i=0;i<S;i++){U64 own=masks[goal[i]]&~covered;covered|=masks[goal[i]];for(int k=0;k<K;k++)if(k!=goal[i]&&(masks[k]&own))required[i][k]=true;}
 for(int i=S-1;i>=0;i--)for(int j=i+1;j<S;j++)if(required[i][goal[j]])required[i]|=required[j];
 auto before=[&](int a,int b){return index[a]>=0&&required[index[a]][b];};
 auto display=[&](){U64 m=0,v=0;for(int k:order){v|=papers[k].color&masks[k]&~m;m|=masks[k];}return v;};
 auto hash=[&](){U64 h=0;for(int k:order)h=h*1000000007+(k+1);return h;};
 unordered_set<U64>seen;seen.insert(hash());vector<pair<int,int>>ops;
 while(display()!=target){
  assert(ops.size()<384);int pos[192],bad[192]{},mandatory_selected=0,mandatory_other=0;bool visible[192]{},mandatory[192]{};U64 cover=0;
  for(int i=0;i<K;i++){int k=order[i];pos[k]=i;visible[k]=bool(masks[k]&~cover);cover|=masks[k];}
  int adjacent[20]{};
  for(int i=0;i<S;i++)for(int k=0;k<K;k++)if(k!=goal[i]&&pos[k]<pos[goal[i]]){
   if((masks[k]&masks[goal[i]])==masks[goal[i]])mandatory[k]=true;
   if(required[i][k]){bad[k]|=1<<i;if(index[k]>=0){adjacent[i]|=1<<index[k];adjacent[index[k]]|=1<<i;}}
  }
  int count=1<<S,all=count-1;vector<int>sum(count);int ordinary=0;
  for(int k=0;k<K;k++)if(index[k]>=0){if(mandatory[k])mandatory_selected|=1<<index[k];}else if(mandatory[k])mandatory_other++;else sum[bad[k]]++,ordinary++;
  for(int bit=0;bit<S;bit++)for(int mask=0;mask<count;mask++)if(mask>>bit&1)sum[mask]+=sum[mask^(1<<bit)];
  vector<uint8_t>independent(count);independent[0]=true;
  for(int mask=1;mask<count;mask++){int bit=__builtin_ctz(unsigned(mask)),rest=mask&(mask-1);independent[mask]=independent[rest]&&!(adjacent[bit]&rest);}
  int best=1000000,bestmask=-1,tiebest=1000000;
  for(int moved=0;moved<count;moved++)if((moved&mandatory_selected)==mandatory_selected&&independent[all^moved]){
   int value=__builtin_popcount(unsigned(moved))+mandatory_other+ordinary-sum[moved],tie=0;for(int i=0;i<S;i++)if(moved>>i&1)tie+=!visible[goal[i]];
   if(value<best||(value==best&&tie<tiebest))best=value,bestmask=moved,tiebest=tie;
  }
  assert(bestmask>=0);bool moving[192]{};for(int k=0;k<K;k++)moving[k]=index[k]>=0?(bestmask>>index[k]&1):(mandatory[k]||(bad[k]&~bestmask));
  vector<vector<int>>edge(K);int degree[192]{};
  for(int i=0;i<S;i++)for(int k=0;k<K;k++)if(required[i][k])edge[goal[i]].push_back(k),degree[k]++;
  int previous=-1;for(int k:order)if(!moving[k]){if(previous>=0)edge[previous].push_back(k),degree[k]++;previous=k;}
  priority_queue<pair<int,int>,vector<pair<int,int>>,greater<pair<int,int>>>ready;for(int k=0;k<K;k++)if(!degree[k])ready.push({pos[k],k});vector<int>final;
  while(!ready.empty()){int k=ready.top().second;ready.pop();final.push_back(k);for(int x:edge[k])if(--degree[x]==0)ready.push({pos[x],x});}
  assert(int(final.size())==K);int rank[192];for(int i=0;i<K;i++)rank[final[i]]=i;
  int chosen_from=-1,chosen_to=-1,best_delta=1000000;
  auto consider=[&](int from,int to){if(from==to)return;int k=order[from],delta=0,dir=to>from?1:-1;for(int j=from+dir;dir*(to-j)>=0;j+=dir){int x=order[j];delta+=dir*(int(before(k,x))-int(before(x,k)));}auto backup=order;order.erase(order.begin()+from);order.insert(order.begin()+to,k);bool fresh=!seen.count(hash());order=move(backup);if(fresh&&delta<best_delta)chosen_from=from,chosen_to=to,best_delta=delta;};
  for(int from=0;from<K;from++){int k=order[from];if(!visible[k]||!moving[k])continue;int next=-1;for(int r=rank[k]+1;r<K;r++)if(!moving[final[r]]){next=final[r];break;}int to=K-1;if(next>=0){to=pos[next];if(from<to)to--;}consider(from,to);}
  if(chosen_from<0||best_delta>=0){
   chosen_from=chosen_to=-1;best_delta=1000000;
   for(int from=0;from<K;from++)if(visible[order[from]])for(int to=0;to<K;to++)consider(from,to);
  }
  assert(chosen_from>=0);int k=order[chosen_from];order.erase(order.begin()+chosen_from);order.insert(order.begin()+chosen_to,k);seen.insert(hash());ops.push_back({k,chosen_to});
 }
 return ops;
}
inline vector<pair<int,int>>bidirectional_retrieve(vector<int>&order,const vector<U64>&masks,const vector<BidirectionalPaper>&papers,const vector<int>&goal,U64 target,bool bottom){
 int K=order.size(),S=goal.size();vector<U64>regions;U64 covered=0;for(int k:goal){regions.push_back(masks[k]&~covered);covered|=masks[k];}
 vector<vector<int>>ancestors(S);for(int i=0;i<S;i++)for(int j=0;j<i;j++)if(masks[goal[i]]&regions[j])ancestors[i].push_back(j);
 vector<char>done(S);vector<pair<int,int>>ops;
 auto display=[&](){U64 m=0,v=0;for(int k:order){v|=papers[k].color&masks[k]&~m;m|=masks[k];}return v;};
 for(int step=0;step<S&&display()!=target;step++){
  int pos[192],depth[64]{};for(int i=0;i<K;i++)pos[order[i]]=i;
  int best=1000000,index=-1,window=-1;
  for(int i=0;i<S;i++)if(!done[i]){
   bool ready=true;for(int a:ancestors[i])ready&=done[a];if(!ready)continue;
   fill(depth,depth+64,0);for(int j=0;j<pos[goal[i]];j++){U64 mask=masks[order[j]]&regions[i];while(mask){int bit=__builtin_ctzll(mask);depth[bit]++;mask&=mask-1;}}
   for(int w=0;w<64;w++)if(regions[i]>>w&1)if(depth[w]<best)best=depth[w],index=i,window=w;
  }
  assert(index>=0);int owner=goal[index];
  while(true){int at=-1,ownerpos=-1;for(int i=0;i<K;i++){int k=order[i];if(k==owner){ownerpos=i;break;}if(at<0&&(masks[k]>>window&1))at=i;}if(at<0)break;int k=order[at];U64 before=0;for(int i=0;i<at;i++)before|=masks[order[i]];assert(masks[k]&~before);order.erase(order.begin()+at);int to=bottom?K-1:ownerpos;order.insert(order.begin()+to,k);ops.push_back({k,to});assert(ops.size()<=384);}
  int at=find(order.begin(),order.end(),owner)-order.begin(),to=0;for(int a:ancestors[index])to=max(to,int(find(order.begin(),order.end(),goal[a])-order.begin())+1);
  if(to!=at){U64 before=0;for(int i=0;i<at;i++)before|=masks[order[i]];assert(masks[owner]&~before);order.erase(order.begin()+at);if(at<to)to--;order.insert(order.begin()+to,owner);ops.push_back({owner,to});}done[index]=true;
 }
 assert(display()==target&&ops.size()<=384);return ops;
}

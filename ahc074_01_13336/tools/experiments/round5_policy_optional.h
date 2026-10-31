// Sum of exact fixed-goal distances to all 64 recorded goal sets.
inline pair<int,int> history_distance(const vector<int>&order,const vector<int>&length,const vector<U64>&future){
 U64 below[192],tree[65]{};int mandatory=0,optional=0;
 for(int i=int(order.size())-1;i>=0;i--){int k=order[i];U64 mask=0;for(int x=length[k];x;x-=x&-x)mask|=tree[x];below[k]=mask;mandatory+=__builtin_popcountll(mask);for(int x=length[k];x<=64;x+=x&-x)tree[x]|=future[k];}
 U64 bylen[65]{},seen[65]{};for(int k:order)bylen[length[k]]|=future[k];
 for(int k:order){U64 chosen=future[k]&~below[k],bad=0;
  for(int l=length[k];l>=1&&chosen;l--){bad|=seen[l]&chosen;chosen&=~bylen[l-1];}
  optional+=__builtin_popcountll(bad);seen[length[k]]|=~below[k]&~future[k];
 }
 return {mandatory,optional};
}
inline int relocation_refine(const vector<int>&initial,vector<int>&target,const vector<int>&length,const vector<char>&selected,const vector<U64>&future,double weight,int passes){
 vector<int>moving;int suffix=65;for(int i=int(initial.size())-1;i>=0;i--){int k=initial[i];if(!selected[k]&&length[k]>=suffix)moving.push_back(k);if(selected[k])suffix=min(suffix,length[k]);}
 reverse(moving.begin(),moving.end());
 auto quality=[&](const vector<int>&p){auto [mandatory,optional]=history_distance(p,length,future);return mandatory+weight*optional;};
 double score=quality(target);int changed=0;
 for(int pass=0;pass<passes;pass++){
  for(int k:moving){int pos=find(target.begin(),target.end(),k)-target.begin();auto order=target;order.erase(order.begin()+pos);
   int left=pos-1;while(left>=0&&!selected[order[left]])left--;int right=left+1;while(right<int(order.size())&&!selected[order[right]])right++;
   int bestpos=pos;double best=score;
   for(int to=left+1;to<=right;to++)if(to!=pos){order.insert(order.begin()+to,k);double q=quality(order);order.erase(order.begin()+to);if(q<best-1e-8){best=q;bestpos=to;}}
   if(bestpos!=pos){order.insert(order.begin()+bestpos,k);target=move(order);score=best;changed++;}
  }
 }
 return changed;
}
inline int global_refine(const vector<int>&initial,vector<int>&target,const vector<int>&length,const vector<char>&selected,const vector<U64>&future,int cutoff,int step){
 vector<int>moving;int suffix=65;for(int i=int(initial.size())-1;i>=0;i--){int k=initial[i];if(!selected[k]&&length[k]>=suffix&&__builtin_popcountll(future[k])<=cutoff)moving.push_back(k);if(selected[k])suffix=min(suffix,length[k]);}
 reverse(moving.begin(),moving.end());
 auto quality=[&](const vector<int>&p){auto [mandatory,optional]=history_distance(p,length,future);return mandatory+optional;};
 int score=quality(target),changed=0;
 for(int k:moving){int pos=find(target.begin(),target.end(),k)-target.begin();auto order=target;order.erase(order.begin()+pos);int bestpos=pos,best=score;
  for(int to=int(order.size());to>pos;to-=step){order.insert(order.begin()+to,k);int q=quality(order);order.erase(order.begin()+to);if(q<best){best=q;bestpos=to;}}
  if(bestpos!=pos){order.insert(order.begin()+bestpos,k);target=move(order);score=best;changed++;}
 }
 return changed;
}

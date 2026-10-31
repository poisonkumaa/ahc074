// Closest permutation displaying a fixed prefix goal if invisible moves were
// allowed. Ties prefer deleting currently visible papers to help legal replay.
inline vector<int> free_goal_target(const vector<int>&order,const vector<int>&length,const vector<char>&selected){
 int K=order.size();array<int,65>cost,next;cost.fill(1000000);cost[0]=0;
 vector<int>roots;for(int k=0;k<K;k++)if(selected[k])roots.push_back(k);sort(roots.begin(),roots.end(),[&](int x,int y){return length[x]<length[y];});
 int owner[65];for(int l=0;l<=64;l++){owner[l]=0;while(length[roots[owner[l]]]<l)owner[l]++;}
 uint8_t previous[193][65]{};bool deleted[193][65]{},visible[192]{};int high=0;
 for(int k:order)if(length[k]>high){visible[k]=true;high=length[k];}
 for(int i=0;i<K;i++){int k=order[i];next.fill(1000000);for(int h=0;h<=64;h++)if(cost[h]<1000000){
  int value=cost[h]+256-visible[k];if(value<next[h]){next[h]=value;previous[i+1][h]=h;deleted[i+1][h]=true;}
  if(!selected[k]||length[k]>h){int nh=max(h,length[roots[owner[length[k]]]]);if(cost[h]<next[nh]){next[nh]=cost[h];previous[i+1][nh]=h;deleted[i+1][nh]=false;}}
 }cost=next;}
 int h=min_element(cost.begin(),cost.end())-cost.begin(),distance=(cost[h]+255)/256;vector<char>moving(K);
 for(int i=K;i>0;i--){moving[order[i-1]]=deleted[i][h];h=previous[i][h];}
 vector<int>target;int at=0;
 for(int k:order)if(!moving[k]){int through=owner[length[k]];while(at<=through){int root=roots[at++];assert(moving[root]||root==k);target.push_back(root);}if(!selected[k])target.push_back(k);}
 while(at<int(roots.size())){int k=roots[at++];assert(moving[k]);target.push_back(k);}
 for(int k:order)if(moving[k]&&!selected[k])target.push_back(k);
 assert(int(target.size())==K);(void)distance;return target;
}

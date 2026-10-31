struct R7InversionDP:PrefixDP {
 vector<array<uint16_t,193>> inversions;
 vector<array<long long,193>> value;
 int level_prefix[65]{},last_inversions=0;
 R7InversionDP(int nn,const vector<PrefixPaper>&aa):PrefixDP(nn,aa),inversions(65*66),value(65){
  for(auto p:aa)level_prefix[p.len]++;for(int l=1;l<=n;l++)level_prefix[l]+=level_prefix[l-1];
 }
 void build_row(int j,const vector<int>&order){
  int delta[192];for(int i=0;i<k;i++){int l=a[order[i]].len;delta[i]=(cnt[j+1][k]-cnt[j+1][i+1])-(cnt[l][k]-cnt[l][i+1]);}
  for(int l=j+1;l<=n+1;l++){
   auto&d=inversions[j*66+l];d[k]=0;
   for(int i=k-1;i>=0;i--){int length=a[order[i]].len;d[i]=d[i+1]+(j<length&&length<l?delta[i]:0);}
  }
 }
 int edge_inversions(int j,int l,int h,int nh)const {
  return level_prefix[l-1]-level_prefix[j]+inversions[j*66+l][h]+inversions[j*66+l+1][nh]-inversions[j*66+l][nh];
 }
 vector<int>solve_geometry(const vector<int>&order,U64 target,int&moves,int weight){
  prepare(order);epoch++;int active[65][193],count[65]{};value[0][0]=0;stamp[0][0]=epoch;active[0][count[0]++]=0;const auto&all=candidate[product_index.at(target)];
  for(int j=0;j<n;j++){
   int ac=count[j];if(!ac)continue;sort(active[j],active[j]+ac);build_row(j,order);
   for(int z:all[j]){int l=a[z].len,q=pos[z];
    for(int ai=0;ai<ac;ai++){
     int h=active[j][ai],add=0;if(q>=h){int moved=cnt[l][q]-cnt[l][h],upper=cnt[j+1][q]-cnt[j+1][h];add=moved+(upper>moved);}
     int nh=max(q+1,h),inv=edge_inversions(j,l,h,nh);
     long long v=value[j][h]+add*(65536LL*1024)+inv*(65536LL*weight)-reward[z];
     if(stamp[l][nh]!=epoch){stamp[l][nh]=epoch;value[l][nh]=1LL<<60;active[l][count[l]++]=nh;}
     if(v<value[l][nh]){value[l][nh]=v;parent[l][nh]=j*(k+1)+h;which[l][nh]=z;}
    }
   }
  }
  int h=active[n][0],j=n;for(int i=1;i<count[n];i++){int x=active[n][i];if(value[n][x]<value[n][h]||(value[n][x]==value[n][h]&&x<h))h=x;}
  vector<int>chosen;last_inversions=0;
  while(j){int z=which[j][h],previous=parent[j][h],pj=previous/(k+1),ph=previous%(k+1);last_inversions+=edge_inversions(pj,j,ph,h);chosen.push_back(z);j=pj;h=ph;}
  reverse(chosen.begin(),chosen.end());vector<char>selected(k);for(int z:chosen)selected[z]=true;vector<int>length;for(auto p:a)length.push_back(p.len);
  moves=GlobalPrefix::fixed_distance(order,length,selected);return chosen;
 }
};

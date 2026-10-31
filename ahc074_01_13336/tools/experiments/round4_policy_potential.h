struct PotentialPlanner {
 PrefixDP&base;const vector<PrefixPaper>&paper;int N,K,epoch=0,cache_epoch=0,last_phi=0;
 vector<array<long long,193>>work;vector<array<int,193>>stamp,parent,which,moves_sum,phi_sum;
 vector<vector<vector<int>>>interval;int cache_stamp[193*193]{},cache_phi[193*193]{};
 Access below[192];int inv[192];
 PotentialPlanner(PrefixDP&b):base(b),paper(b.a),N(b.n),K(b.k),work(N+1),stamp(N+1),parent(N+1),which(N+1),moves_sum(N+1),phi_sum(N+1),interval(N+1,vector<vector<int>>(N+1)){
  for(int j=0;j<N;j++)for(int l=j+1;l<=N;l++)for(int k=0;k<K;k++)if(j<paper[k].len&&paper[k].len<=l)interval[j][l].push_back(k);
 }
 int group_cost(const vector<int>&members,int length,int h,int q,const vector<Access>&future){
  int size=members.size(),v[192],missing[192],vc=0,mc=0;Access all;
  for(int i=0;i<size;i++){int k=members[i];below[i]=0;inv[i]=0;all|=future[k];bool moved=base.pos[k]<(paper[k].len==length?max(h,q+1):h);if(moved)missing[mc++]=i;else v[vc++]=i;}
  int total=maskpop(all);
  for(int i=vc-1;i>=0;i--){int x=v[i],k=members[x];for(int j=i+1;j<vc;j++){int y=v[j],z=members[y];if(paper[z].len<=paper[k].len)below[x]|=future[z];if(paper[z].len<paper[k].len)inv[x]++;}total+=maskpop(below[x]);}
  stable_sort(missing,missing+mc,[&](int a,int b){return paper[members[a]].len<paper[members[b]].len;});
  for(int mi=0;mi<mc;mi++){
   int x=missing[mi],k=members[x],kl=paper[k].len;Access remaining[193];int lower[193];remaining[vc]=0;lower[vc]=0;
   for(int i=vc-1;i>=0;i--){int z=members[v[i]];remaining[i]=remaining[i+1]|(paper[z].len<=kl?future[z]:Access(0));lower[i]=lower[i+1]+(paper[z].len<kl);}
   int gain=0,larger=0,best=INT_MAX,bestinv=INT_MAX,at=0;
   for(int i=0;i<=vc;i++){int cost=gain+maskpop(remaining[i]),inversions=larger+lower[i];if(cost<best||(cost==best&&inversions<bestinv)){best=cost;bestinv=inversions;at=i;}
    if(i<vc){int y=v[i],z=members[y];if(paper[z].len>kl)larger++;if(paper[z].len>=kl)gain+=maskpop(future[k]&~below[y]);}
   }
   total+=best;below[x]=remaining[at];inv[x]=lower[at];for(int i=0;i<at;i++){int y=v[i],z=members[y];if(paper[z].len>=kl)below[y]|=future[k];if(paper[z].len>kl)inv[y]++;}
   for(int i=vc;i>at;i--)v[i]=v[i-1];v[at]=x;vc++;
  }
  return total;
 }
 vector<int>solve(const vector<int>&order,U64 target,const vector<Access>&future,int weight,int&moves){
  base.prepare(order);epoch++;int active[65][193],count[65]{};work[0][0]=0;stamp[0][0]=epoch;moves_sum[0][0]=phi_sum[0][0]=0;active[0][count[0]++]=0;
  const auto&all=base.candidate[base.product_index.at(target)];
  for(int j=0;j<N;j++){
   int ac=count[j];if(!ac)continue;sort(active[j],active[j]+ac);
   for(int z:all[j]){
    int l=paper[z].len,q=base.pos[z],lower_pos[192],same_pos[192],lc=0,sc=0;vector<int>members;members.reserve(interval[j][l].size());
    for(int k:interval[j][l])if(k!=z)members.push_back(k);sort(members.begin(),members.end(),[&](int a,int b){return base.pos[a]<base.pos[b];});
    for(int k:members)if(paper[k].len==l)same_pos[sc++]=base.pos[k];else lower_pos[lc++]=base.pos[k];
    int tag=++cache_epoch;
    for(int ai=0;ai<ac;ai++){
     int h=active[j][ai],add=0;if(q>=h){int removed=base.cnt[l][q]-base.cnt[l][h],upper=base.cnt[j+1][q]-base.cnt[j+1][h];add=removed+(upper>removed);}
     int nh=max(q+1,h),lowcut=lower_bound(lower_pos,lower_pos+lc,h)-lower_pos,samecut=lower_bound(same_pos,same_pos+sc,nh)-same_pos,key=lowcut*193+samecut;
     int phi;if(cache_stamp[key]==tag)phi=cache_phi[key];else{phi=group_cost(members,l,h,q,future);cache_stamp[key]=tag;cache_phi[key]=phi;}
     long long value=work[j][h]+add*1000000000000LL+1LL*weight*phi-base.reward[z];
     if(stamp[l][nh]!=epoch){stamp[l][nh]=epoch;work[l][nh]=LLONG_MAX;active[l][count[l]++]=nh;}
     if(value<work[l][nh]){work[l][nh]=value;moves_sum[l][nh]=moves_sum[j][h]+add;phi_sum[l][nh]=phi_sum[j][h]+phi;parent[l][nh]=j*193+h;which[l][nh]=z;}
    }
   }
  }
  int h=active[N][0],j=N;for(int i=1;i<count[N];i++){int x=active[N][i];if(work[N][x]<work[N][h]||(work[N][x]==work[N][h]&&x<h))h=x;}
  moves=moves_sum[N][h];last_phi=phi_sum[N][h];vector<int>goal;while(j){goal.push_back(which[j][h]);int prev=parent[j][h];j=prev/193;h=prev%193;}reverse(goal.begin(),goal.end());return goal;
 }
};
int total_potential(const vector<int>&order,const vector<int>&length,const vector<Access>&future){Access tree[65];int total=0;for(int i=int(order.size())-1;i>=0;i--){int k=order[i];Access u;for(int x=length[k];x>0;x-=x&-x)u|=tree[x];total+=maskpop(u);for(int x=length[k];x<=64;x+=x&-x)tree[x]|=future[k];}return total;}

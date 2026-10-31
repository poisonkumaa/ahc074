from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'goal_reward_fast.cpp').read_text().replace('int main(){','int main(int argc,char**argv){')
a=s.index('struct PrefixDP {');b=s.index('\n};',a)+3
s=s[:a]+'''struct PrefixDP {
 int n,k;vector<PrefixPaper>a;vector<array<int,193>>cnt,work,parent,which;int pos[192];vector<int>reward;
 PrefixDP(int nn,vector<PrefixPaper>aa):n(nn),k(aa.size()),a(move(aa)),cnt(n+2),work(n+1),parent(n+1),which(n+1),reward(k,16){}
 static U64 low(int n){return n==64?~0ULL:(1ULL<<n)-1;}
 void prepare(const vector<int>&p){for(int i=0;i<k;i++)pos[p[i]]=i;for(int l=0;l<=n+1;l++){cnt[l][0]=0;for(int i=0;i<k;i++)cnt[l][i+1]=cnt[l][i]+(a[p[i]].len>=l);}}
 vector<int> solve(const vector<int>&p,U64 target,int&best,bool reconstruct=true){prepare(p);return solve_prepared(target,best,reconstruct);}
 vector<int> solve_prepared(U64 target,int&best,bool reconstruct=true){
  auto&dp=work;for(auto&r:dp)r.fill(1000000000);dp[0][0]=0;
  for(int j=0;j<n;j++){
   int active[193],ac=0;for(int h=0;h<=k;h++)if(dp[j][h]<1000000000)active[ac++]=h;if(!ac)continue;
   int cand[192],cc=0;for(int z=0;z<k;z++)if(a[z].len>j&&!((a[z].color^target)&(low(a[z].len)^low(j))))cand[cc++]=z;
   for(int ci=0;ci<cc;ci++){int z=cand[ci],l=a[z].len,q=pos[z];
    for(int ai=0;ai<ac;ai++){int h=active[ai],add=0;if(q>=h){int moved=cnt[l][q]-cnt[l][h],upper=cnt[j+1][q]-cnt[j+1][h];add=moved+(upper>moved);}int nh=max(q+1,h),value=dp[j][h]+add*65536-reward[z];
     if(value<dp[l][nh]){dp[l][nh]=value;if(reconstruct){parent[l][nh]=j*(k+1)+h;which[l][nh]=z;}}
    }
   }
  }
  int h=min_element(dp[n].begin(),dp[n].begin()+k+1)-dp[n].begin(),j=n;best=(dp[n][h]+65535)/65536;vector<int>ans;if(!reconstruct)return ans;
  while(j){int z=which[j][h],prev=parent[j][h];ans.push_back(z);j=prev/(k+1);h=prev%(k+1);}reverse(ans.begin(),ans.end());return ans;
 }
};''' +s[b:]
start=s.index('  for(int k=0;k<int(papers.size());k++)dp.reward[k]=16+2*')
end=s.index('  {int period=1;',start)
s=s[:start]+'''  int variants=argc>1?atoi(argv[1]):3,samplecount=argc>2?atoi(argv[2]):8,mode=argc>3?atoi(argv[3]):2;
  vector<int>targets(samplecount);for(int&i:targets){do{i=freq_rng()%P;}while(i==product);}
  auto mandatory=[&](const vector<int>&p){uint64_t tree[65]{};int q=0;for(int i=int(p.size())-1;i>=0;i--){int k=p[i];uint64_t u=0;for(int x=length[k];x>0;x-=x&-x)u|=tree[x];q+=__builtin_popcountll(u);for(int x=length[k];x<=64;x+=x&-x)tree[x]|=future[k];}return q;};
  vector<pair<int,int>>operations;vector<int>bestorder,bestchosen;vector<vector<int>>seen;double bestquality=1e30;
  for(int v=0;v<variants;v++){
   int alpha=v==0?2:v==1?0:8;
   for(int k=0;k<int(papers.size());k++)dp.reward[k]=16+alpha*__builtin_popcountll(future[k]);
   if(v>=3)for(int k=0;k<int(papers.size());k++)dp.reward[k]=16+2*__builtin_popcountll(future[k])+(freq_rng()%33);
   int moves;auto chosen=dp.solve(order,A[product],moves);if(moves<0||moves>2*K)return 0;
   if(find(seen.begin(),seen.end(),chosen)!=seen.end())continue;seen.push_back(chosen);
   vector<char>selected(papers.size());for(int k:chosen)selected[k]=true;
   auto p=order;vector<pair<int,int>>ops;ops.reserve(moves);
   for(int i=0;i<moves;i++){auto[from,to]=GlobalPrefix::best_move(p,length,selected,.5,&future);if(from<0)return 0;int k=p[from];p.erase(p.begin()+from);p.insert(p.begin()+to,k);ops.push_back({k,to});}
   double quality=mandatory(p);
   if(mode==1){int pos[192];for(int i=0;i<int(p.size());i++)pos[p[i]]=i;int total=0;
    for(int f=0;f<64;f++){uint64_t bit=1ULL<<f;int owner[65],root=-1;fill(owner,owner+65,-1);for(int k=0;k<int(p.size());k++)if(future[k]&bit)owner[length[k]]=k;for(int l=64;l>=0;l--){if(owner[l]>=0)root=owner[l];owner[l]=root;}
     bool moved[192]{},bad[192]{};int minimum=65;for(int i=int(p.size())-1;i>=0;i--){int k=p[i];moved[k]=length[k]>=minimum;total+=moved[k];if(future[k]&bit)minimum=min(minimum,length[k]);}
     for(int k=0;k<int(p.size());k++)if(!(future[k]&bit)&&!moved[k]&&pos[k]<pos[owner[length[k]]])bad[owner[length[k]]]=true;
     for(int k=0;k<int(p.size());k++)if((future[k]&bit)&&!moved[k]&&bad[k])total++;
    }quality=total;
   }else if(mode>=2){dp.prepare(p);int total=0;for(int target:targets){int distance;dp.solve_prepared(A[target],distance,false);total+=distance;}quality=total+mandatory(p)*0.00001;if(mode==3)quality=double(total)/samplecount+mandatory(p)/128.;}
   if(quality<bestquality-1e-10){bestquality=quality;bestorder=p;bestchosen=chosen;operations=ops;}
  }
  order=bestorder;vector<char>selected(papers.size());for(int k:bestchosen)selected[k]=true;
''' +s[end:]
(p/'policy2_agent_goal_ensemble.cpp').write_text(s)

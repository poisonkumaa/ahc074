from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy2_agent_goal_ensemble.cpp').read_text()
a=s.index('struct PrefixDP {');b=s.index('\n};',a)+3
s=s[:a]+'''struct PrefixDP {
 int n,k,epoch=0;vector<PrefixPaper>a;vector<array<int,193>>cnt,work,parent,which,stamp;int pos[192];vector<int>reward;
 vector<array<vector<uint8_t>,64>>candidate;unordered_map<U64,int>product_index;
 PrefixDP(int nn,vector<PrefixPaper>aa):n(nn),k(aa.size()),a(move(aa)),cnt(n+2),work(n+1),parent(n+1),which(n+1),stamp(n+1),reward(k,16){}
 static U64 low(int n){return n==64?~0ULL:(1ULL<<n)-1;}
 void public_data(const vector<U64>&A){candidate.resize(A.size());for(int v=0;v<int(A.size());v++){product_index[A[v]]=v;for(int z=0;z<k;z++){U64 diff=(A[v]^a[z].color)&low(a[z].len);int start=diff?64-__builtin_clzll(diff):0;for(int j=start;j<a[z].len;j++)candidate[v][j].push_back(z);}}}
 void prepare(const vector<int>&p){for(int i=0;i<k;i++)pos[p[i]]=i;for(int l=0;l<=n+1;l++){cnt[l][0]=0;for(int i=0;i<k;i++)cnt[l][i+1]=cnt[l][i]+(a[p[i]].len>=l);}}
 vector<int>solve(const vector<int>&p,U64 target,int&best,bool reconstruct=true){prepare(p);return solve_prepared(target,best,reconstruct);}
 vector<int>solve_prepared(U64 target,int&best,bool reconstruct=true){
  auto&dp=work;epoch++;int active[65][193],count[65]{};dp[0][0]=0;stamp[0][0]=epoch;active[0][count[0]++]=0;const auto&all=candidate[product_index.at(target)];
  for(int j=0;j<n;j++){
   int ac=count[j];if(!ac)continue;if(reconstruct)sort(active[j],active[j]+ac);
   for(int z:all[j]){int l=a[z].len,q=pos[z];
    for(int ai=0;ai<ac;ai++){int h=active[j][ai],add=0;if(q>=h){int moved=cnt[l][q]-cnt[l][h],upper=cnt[j+1][q]-cnt[j+1][h];add=moved+(upper>moved);}int nh=max(q+1,h),value=dp[j][h]+add*65536-reward[z];
     if(stamp[l][nh]!=epoch){stamp[l][nh]=epoch;dp[l][nh]=1000000000;active[l][count[l]++]=nh;}
     if(value<dp[l][nh]){dp[l][nh]=value;if(reconstruct){parent[l][nh]=j*(k+1)+h;which[l][nh]=z;}}
    }
   }
  }
  int h=active[n][0],j=n;for(int i=1;i<count[n];i++){int x=active[n][i];if(dp[n][x]<dp[n][h]||(dp[n][x]==dp[n][h]&&x<h))h=x;}best=(dp[n][h]+65535)/65536;vector<int>ans;if(!reconstruct)return ans;
  while(j){int z=which[j][h],prev=parent[j][h];ans.push_back(z);j=prev/(k+1);h=prev%(k+1);}reverse(ans.begin(),ans.end());return ans;
 }
};''' +s[b:]
s=s.replace(' PrefixDP dp(n,papers);vector<int>order',' PrefixDP dp(n,papers);dp.public_data(A);vector<int>order')
(p/'policy2_agent_goal_sparse.cpp').write_text(s)

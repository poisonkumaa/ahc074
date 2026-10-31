from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy2_agent_union.cpp').read_text()
# Retain original planner, create a secondary planner with randomized tie-breaking.
s=s.replace('vector<int> solve(const vector<int>&p,U64 target,int &best){','vector<int> solve(const vector<int>&p,U64 target,int &best,int salt=0){')
s=s.replace('r.fill(1000000)','r.fill(1000000000)').replace('dp[j][h]<1000000','dp[j][h]<1000000000')
s=s.replace('int nh=max(q+1,h),value=dp[j][h]+add*256-1;', '''uint64_t hash=uint64_t(z+1)*0x9e3779b97f4a7c15ULL+uint64_t(salt)*0xbf58476d1ce4e5b9ULL;hash^=hash>>30;hash*=0xbf58476d1ce4e5b9ULL;hash^=hash>>27;
     int bonus=salt?int(hash%1024):0;
     int nh=max(q+1,h),value=dp[j][h]+add*1000000-10000+bonus;''')
s=s.replace('best=(dp[n][h]+255)/256','best=(dp[n][h]+999999)/1000000')
# save sample selected and nearest selected owner for future fixed goal estimator
s=s.replace('for(int r=0;r<128+samples;r++){','vector<vector<char>> fselected(samples,vector<char>(papers.size()));vector<vector<int>> fnext(samples,vector<int>(papers.size()));\n for(int r=0;r<128+samples;r++){')
s=s.replace('for(int t=0;t<moves;t++){', '''if(r>=128){fselected[r-128]=selected;int owner[65],root=-1;fill(owner,owner+65,-1);for(int k:chosen)owner[length[k]]=k;for(int l=64;l>=0;l--){if(owner[l]>=0)root=owner[l];owner[l]=root;}for(int k=0;k<int(papers.size());k++)fnext[r-128][k]=owner[length[k]];}
  for(int t=0;t<moves;t++){''',1)
start=s.index('  int moves;auto chosen=dp.solve(order,A[product],moves);')
end=s.index('  cout<<operations.size()',start)
s=s[:start]+'''  auto future_cost=[&](const vector<int>&p){int total=0,pos[192];for(int i=0;i<int(p.size());i++)pos[p[i]]=i;
   for(int f=0;f<samples;f++){bool moved[192]{},bad[192]{};int minimum=65;
    for(int i=int(p.size())-1;i>=0;i--){int k=p[i];moved[k]=length[k]>=minimum;total+=moved[k];if(fselected[f][k])minimum=min(minimum,length[k]);}
    for(int k=0;k<int(p.size());k++)if(!fselected[f][k]&&!moved[k]&&pos[k]<pos[fnext[f][k]])bad[fnext[f][k]]=true;
    for(int k=0;k<int(p.size());k++)if(fselected[f][k]&&!moved[k]&&bad[k])total++;
   }return total;
  };
  vector<pair<int,int>>operations;vector<int>nextorder;int bestquality=INT_MAX;
  int variants=argc>2?atoi(argv[2]):4;bool union_policy=argc>3?atoi(argv[3]):false;
  for(int v=0;v<variants;v++){
   int moves;auto chosen=dp.solve(order,A[product],moves,v?int(freq_rng()%100000)+1:0);if(moves<0||moves>2*K)return 0;
   vector<char>selected(papers.size());for(int k:chosen)selected[k]=true;
   auto candorder=order;vector<pair<int,int>>candops;
   for(int i=0;i<moves;i++){
    auto[from,to]=GlobalPrefix::best_move(candorder,length,selected,.5,union_policy?&future:nullptr,words);if(from<0)return 0;
    int k=candorder[from];candorder.erase(candorder.begin()+from);candorder.insert(candorder.begin()+to,k);candops.push_back({k,to});
   }
   int quality=future_cost(candorder);if(quality<bestquality){bestquality=quality;nextorder=candorder;operations=candops;}
  }
  order=nextorder;
''' + s[end:]
(p/'policy2_agent_goals.cpp').write_text(s)

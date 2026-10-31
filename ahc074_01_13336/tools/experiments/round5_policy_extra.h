// Exact minimum and minimum-plus-one paths in the prefix state DAG.
// A path costing at least two above its state's minimum cannot reach a
// terminal goal within one move of the global minimum.
struct ExtraGoalDP {
 struct Node { int epoch=0,dist=1000,reward[2],from[2],paper[2],delta[2]; };
 PrefixDP&base; vector<array<Node,193>>nodes; int epoch=0;
 ExtraGoalDP(PrefixDP&b):base(b),nodes(b.n+1){}
 array<vector<int>,2> solve(const vector<int>&order,U64 target,int&minimum){
  base.prepare(order); ++epoch; int active[65][193],count[65]{};
  auto&initial=nodes[0][0]; initial.epoch=epoch;initial.dist=0;initial.reward[0]=0;initial.reward[1]=INT_MIN;
  active[0][count[0]++]=0;
  const auto&all=base.candidate[base.product_index.at(target)];
  for(int j=0;j<base.n;j++){
   int ac=count[j];if(!ac)continue;sort(active[j],active[j]+ac);
   for(int z:all[j]){int l=base.a[z].len,q=base.pos[z];
    for(int ai=0;ai<ac;ai++){
     int h=active[j][ai],add=0;
     if(q>=h){int moved=base.cnt[l][q]-base.cnt[l][h],upper=base.cnt[j+1][q]-base.cnt[j+1][h];add=moved+(upper>moved);}
     int nh=max(q+1,h);auto&src=nodes[j][h];auto&dest=nodes[l][nh];
     if(dest.epoch!=epoch){dest.epoch=epoch;dest.dist=1000;dest.reward[0]=dest.reward[1]=INT_MIN;active[l][count[l]++]=nh;}
     for(int delta=0;delta<2;delta++)if(src.reward[delta]!=INT_MIN){
      int distance=src.dist+delta+add,reward=src.reward[delta]+base.reward[z];
      if(distance<dest.dist){
       if(distance+1==dest.dist){dest.reward[1]=dest.reward[0];dest.from[1]=dest.from[0];dest.paper[1]=dest.paper[0];dest.delta[1]=dest.delta[0];}
       else dest.reward[1]=INT_MIN;
       dest.dist=distance;dest.reward[0]=INT_MIN;
      }
      int dd=distance-dest.dist;
      if(dd<2&&reward>dest.reward[dd]){dest.reward[dd]=reward;dest.from[dd]=j*(base.k+1)+h;dest.paper[dd]=z;dest.delta[dd]=delta;}
     }
    }
   }
  }
  minimum=1000;for(int i=0;i<count[base.n];i++)minimum=min(minimum,nodes[base.n][active[base.n][i]].dist);
  array<vector<int>,2>answer;
  for(int extra=0;extra<2;extra++){
   int best=INT_MIN,bh=-1,bd=-1;
   for(int i=0;i<count[base.n];i++){int h=active[base.n][i];auto&nd=nodes[base.n][h];int d=minimum+extra-nd.dist;
    if(d>=0&&d<2&&(nd.reward[d]>best||(nd.reward[d]==best&&h<bh))){best=nd.reward[d];bh=h;bd=d;}
   }
   if(bh<0)continue;
   int j=base.n,h=bh,d=bd;
   while(j){auto&nd=nodes[j][h];answer[extra].push_back(nd.paper[d]);int prev=nd.from[d],pd=nd.delta[d];j=prev/(base.k+1);h=prev%(base.k+1);d=pd;}
   reverse(answer[extra].begin(),answer[extra].end());
  }
  return answer;
 }
};

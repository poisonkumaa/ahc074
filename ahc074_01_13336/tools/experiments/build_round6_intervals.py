from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'round5_sampling_adaptive.cpp').read_text()
s=s.replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100').replace('#define AHC_REFINEMENT_BUDGET 1.65','#define AHC_REFINEMENT_BUDGET 100')
s=s[:s.index('int main()')]
s+=r'''
struct IntervalPlan {
 int K;vector<U64>mask,color;
 IntervalPlan(const vector<PrefixPaper>&p,const vector<int>&begin):K(p.size()),mask(K),color(K){for(int k=0;k<K;k++){mask[k]=PrefixDP::low(p[k].len)^PrefixDP::low(begin[k]);color[k]=p[k].color&mask[k];}}
 U64 display(const vector<int>&o)const{U64 seen=0,out=0;for(int k:o){out|=color[k]&~seen;seen|=mask[k];}return out;}
 bool plan(vector<int>&o,const vector<int>&goal,U64 target,int bound,vector<pair<int,int>>&ops,int mode)const{
  int rank[192],score[192],prev[192];for(int i=0;i<K;i++)rank[goal[i]]=i;
  int best=-1;for(int i=0;i<K;i++){score[i]=1;prev[i]=-1;for(int j=0;j<i;j++)if(rank[o[j]]<rank[o[i]]&&(score[j]+1>score[i]||(mode&1&&score[j]+1==score[i]))){score[i]=score[j]+1;prev[i]=j;}if(best<0||score[i]>score[best]||(mode&1&&score[i]==score[best]))best=i;}
  vector<char>moving(K,1);for(int i=best;i>=0;i=prev[i])moving[o[i]]=0;
  ops.clear();
  for(int it=0;it<bound+K;it++){
   U64 covered=0,shown=0;int from=-1,firstbad=-1;int highrank=-1;bool visible[192]{};
   for(int i=0;i<K;i++){int k=o[i];U64 v=mask[k]&~covered;visible[k]=v!=0;shown|=color[k]&v;covered|=mask[k];if(v&(color[k]^target))if(firstbad<0)firstbad=i;
    if(v&&moving[k]){if(from<0||((mode&2)&&rank[k]>highrank)){from=i;highrank=rank[k];}}}
   if(shown==target)return true;
   if(int(ops.size())>=bound)return false;
   if(from<0){if(firstbad<0)return false;from=firstbad;int k=o[from];o.erase(o.begin()+from);o.push_back(k);moving[k]=1;ops.push_back({k,K-1});continue;}
   int k=o[from],next=-1;for(int r=rank[k]+1;r<K;r++)if(!moving[goal[r]]){next=goal[r];break;}
   int to=K-1;if(next>=0){to=find(o.begin(),o.end(),next)-o.begin();if(from<to)to--;}
   moving[k]=0;
   if(to!=from){o.erase(o.begin()+from);o.insert(o.begin()+to,k);ops.push_back({k,to});}
  }return false;
 }
};
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);int n,K,P,R;if(!(cin>>n>>K>>P>>R))return 0;
 vector<U64>original(P);for(int v=0;v<P;v++){string x;cin>>x;for(int j=0;j<n;j++)if(x[j]=='1')original[v]|=1ULL<<j;}
 structure2_training_origin=clock();auto model=train_multi_model(original,12345);auto A=move(model.A);auto cols=move(model.cols);auto flip=move(model.flip);auto papers=move(model.papers);
 exact_state_refit(A,papers,1,43,32,8,0);auto future=selection_teacher(A,papers);
 vector<int>length,begin;for(auto p:papers){length.push_back(p.len);begin.push_back(p.len-1);}int active=papers.size();
 int repetitions=argc>1?atoi(argv[1]):4,tries=argc>2?atoi(argv[2]):4;
 if(repetitions<0){
  int width=-repetitions;for(int k=0;k<active;k++)begin[k]=max(0,length[k]-width);
  for(U64 target:A){int value[65],parent[65],who[65];fill(value,value+65,100000000);value[0]=0;
   for(int j=0;j<64;j++)if(value[j]<100000000)for(int k=0;k<active;k++){int l=length[k];if(l<=j||((papers[k].color^target)&(PrefixDP::low(l)^PrefixDP::low(j))))continue;int cost=value[j]+1024*max(0,begin[k]-j)+1;if(cost<value[l]){value[l]=cost;parent[l]=j;who[l]=k;}}
   for(int l=64;l;){int k=who[l],j=parent[l];begin[k]=min(begin[k],j);l=j;}
  }
 }else
 {
  PrefixDP teacher(n,papers);teacher.public_data(A);vector<int>order(active);iota(order.begin(),order.end(),0);mt19937 rng(718927);vector<int>targets(P);iota(targets.begin(),targets.end(),0);
  for(int r=0;r<128+P*repetitions;r++){
   if(r>=128&&(r-128)%P==0)shuffle(targets.begin(),targets.end(),rng);int t=r<128?rng()%P:targets[(r-128)%P],moves;
   for(int k=0;k<active;k++)teacher.reward[k]=16+2*__builtin_popcountll(future[k]);
   auto chosen=teacher.solve(order,A[t],moves);vector<char>selected(active);int last=0;for(int k:chosen){selected[k]=1;if(r>=128)begin[k]=min(begin[k],last);last=length[k];}
   order=direct_final(order,length,selected,&future);
  }
 }
 if(repetitions==0)fill(begin.begin(),begin.end(),0);
 PrefixDP dp(n,papers);dp.public_data(A);for(auto&row:dp.candidate)for(int j=0;j<n;j++)row[j].erase(remove_if(row[j].begin(),row[j].end(),[&](int k){return j<begin[k];}),row[j].end());
 IntervalPlan planner(papers,begin);vector<int>order(active);iota(order.begin(),order.end(),0);
 int totalwidth=0;for(int k=0;k<active;k++)totalwidth+=length[k]-begin[k];cerr<<"IntervalWidth="<<double(totalwidth)/active<<"\n";
 for(int k=0;k<K;k++){string pattern(n,'.');if(k<active)for(int j=begin[k];j<length[k];j++)pattern[cols[j]]=char('0'+(((papers[k].color>>j)&1)^flip[j]));cout<<pattern<<'\n';}cout.flush();
 int savings=0,improved=0,fallbacks=0;
 for(int r=0;r<R;r++){
  int product;if(!(cin>>product)||product<0)return 0;for(int k=0;k<active;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);int moves;
  auto chosen=dp.solve(order,A[product],moves);vector<char>selected(active);for(int k:chosen)selected[k]=1;auto target=direct_final(order,length,selected,&future);
  auto original_order=order;vector<pair<int,int>>operations;
  if(!direct_moves(order,length,selected,target,operations)){fallbacks++;order=original_order;for(int i=0;i<moves;i++){auto[from,to]=GlobalPrefix::best_move(order,length,selected,.5,&future);if(from<0)abort();int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);operations.push_back({k,to});}}
  for(int mode=0;mode<tries;mode++){auto test=original_order;vector<pair<int,int>>ops;if(planner.plan(test,target,A[product],int(operations.size())-1,ops,mode)){operations=move(ops);order=move(test);}}
  if(int(operations.size())<moves){savings+=moves-operations.size();improved++;}assert(planner.display(order)==A[product]);
  U64 bit=1ULL<<(r%64);for(int k=0;k<active;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
  cout<<operations.size()<<'\n';for(auto[k,d]:operations)cout<<k<<' '<<d<<'\n';cout.flush();
 }
 cerr<<"IntervalSavings="<<savings<<" Improved="<<improved<<" PathFallbacks="<<fallbacks<<" FinishCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
}
'''
(d/'round6_intervals.cpp').write_text(s)

from pathlib import Path
root=Path(__file__).resolve().parents[2]
text=(root/'main.cpp').read_text()
text=text[:text.index('int main(){')]
text=text.replace('int n,k,epoch=0;', 'vector<int> category;vector<array<int,193>> catcnt;int n,k,epoch=0;',1)
text=text.replace('void prepare(const vector<int>&p){', 'void prepare(const vector<int>&p){if(!category.empty())for(int g=0;g<int(catcnt.size());g++){catcnt[g][0]=0;for(int i=0;i<k;i++)catcnt[g][i+1]=catcnt[g][i]+(category[p[i]]==g);}',1)
text=text.replace('add=moved+(upper>moved);', 'if(!category.empty()&&l==n&&category[z]>=0)moved=catcnt[category[z]][q]-catcnt[category[z]][h];add=moved+(upper>moved);',1)
text += r'''
#include "structure2_agent_general_move.h"
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int n,K,P,R;if(!(cin>>n>>K>>P>>R)||n!=64||K!=192||P!=256)return 0;
 vector<U64>original(P);for(int v=0;v<P;v++){string x;cin>>x;if(int(x.size())!=n)return 0;for(int j=0;j<n;j++)if(x[j]=='1')original[v]|=1ULL<<j;}
 int block=argc>1?atoi(argv[1]):2,core=n-block, repetitions=argc>2?atoi(argv[2]):8;
 structure2_training_origin=clock();multi_training_budget=AHC_LEARNING_BUDGET;
 auto model=train_multi_model(original,12345);auto A=move(model.A);auto cols=move(model.cols);auto flip=move(model.flip);auto papers=move(model.papers);
 exact_state_refit(A,papers,1,43,32,repetitions,0);
 vector<int>length,original_length,group(K,-1);vector<U64>masks(K);vector<vector<int>>groups(block);
 for(int k=0;k<K;k++){
  auto&p=papers[k];original_length.push_back(p.len);
  if(p.len>core){int g=p.len-core-1;group[k]=g;groups[g].push_back(k);masks[k]=PrefixDP::low(core)|(1ULL<<(core+g));p.len=core;}
  else masks[k]=PrefixDP::low(p.len);
  length.push_back(p.len);
 }
 for(int g=0;g<block;g++){
  int colors=0;for(int k:groups[g])colors|=1<<((papers[k].color>>(core+g))&1);
  if(colors!=3){assert(groups[g].size()>=2);int k=groups[g].back();papers[k].color^=1ULL<<(core+g);}
 }
 PrefixDP dp(core,papers);dp.public_data(A);if(argc>3&&atoi(argv[3])){dp.category=group;dp.catcnt.resize(block);}
 for(int v=0;v<P;v++)for(int j=0;j<core;j++){
  auto&c=dp.candidate[v][j];c.erase(remove_if(c.begin(),c.end(),[&](int k){return group[k]>=0&&((A[v]^papers[k].color)>>(core+group[k])&1);}),c.end());
 }
 vector<int>order(K);iota(order.begin(),order.end(),0);
 // Explicit reachability check before any output or query access.
 for(int v=0;v<P;v++){
  bool reach[65]{};reach[0]=true;for(int j=0;j<core;j++)if(reach[j])for(int k:dp.candidate[v][j])reach[length[k]]=true;
  if(!reach[core]){cerr<<"UNREACHABLE "<<v<<"\n";return 3;}
 }
 auto visible=[&](){vector<char>result(K);U64 covered=0;for(int k:order){result[k]=bool(masks[k]&~covered);covered|=masks[k];}return result;};
 auto display=[&](){U64 covered=0,value=0;for(int k:order){value|=papers[k].color&masks[k]&~covered;covered|=masks[k];}return value;};
 cerr<<"PrepCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<" block="<<block<<"\n";
 for(int k=0;k<K;k++){string pattern(n,'.');for(int j=0;j<n;j++)if(masks[k]>>j&1)pattern[cols[j]]=char('0'+(((papers[k].color>>j)&1)^flip[j]));cout<<pattern<<'\n';}cout.flush();
 vector<U64>future(K);long long core_moves=0,repair_moves=0,potential_moves=0;
 for(int r=0;r<R;r++){
  int product;if(!(cin>>product)||product<0||product>=P)return 0;
  for(int k=0;k<K;k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);
  int moves;auto chosen=dp.solve(order,A[product],moves);vector<char>selected(K);for(int k:chosen)selected[k]=true;
  vector<pair<int,int>>operations;potential_moves+=moves;
  while(GlobalPrefix::fixed_distance(order,length,selected)){
   auto vis=visible();auto [from,to]=GeneralPrefix::best_move(order,length,selected,vis,length,.5);
   if(from<0||!vis[order[from]]){cerr<<"STUCK\n";return 4;}
   int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);operations.push_back({k,to});core_moves++;
  }
  for(int g=0;g<block;g++)while(true){
   int first=-1,next=-1;
   for(int i=0;i<K;i++)if(group[order[i]]==g){if(first<0)first=i;if(!((papers[order[i]].color^A[product])>>(core+g)&1)){next=i;break;}}
   assert(first>=0&&next>=0);if(first==next)break;
   int k=order[first];assert(visible()[k]);order.erase(order.begin()+first);order.insert(order.begin()+next,k);operations.push_back({k,next});repair_moves++;
  }
  if(display()!=A[product]||operations.size()>384){cerr<<"INVALID DISPLAY r="<<r<<" difference="<<__builtin_popcountll(display()^A[product])<<"\n";return 5;}
  U64 bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
  cout<<operations.size()<<'\n';for(auto[k,d]:operations)cout<<k<<' '<<d<<'\n';cout.flush();
 }
 cerr<<"Core="<<core_moves<<" repairs="<<repair_moves<<" potential="<<potential_moves<<" FinishCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
}
'''
(root/'tools/experiments/round6_tailblock.cpp').write_text(text)

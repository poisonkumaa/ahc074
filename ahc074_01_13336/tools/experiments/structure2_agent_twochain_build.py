from pathlib import Path
root=Path(__file__).resolve().parents[2]
original=(root/'main.cpp').read_text()
text=original[:original.index('int main(){')]
text=text.replace('P=A.size(),K=192,INF=1000','P=A.size(),K=96,INF=1000')
text=text.replace('P=A.size(),N=64,K=papers.size(),INF=1000','P=A.size(),N=32,K=papers.size(),INF=1000')
start=original[original.index('int main(){'):original.index(' vector<PrefixPaper>papers;',original.index('int main(){'))]
start=start.replace('int main(){','int main(int argc,char**argv){\n const int perlevel=argc>1?atoi(argv[1]):6;')
text+=start
text+='''
 struct Group{
  vector<U64>data,future;vector<PrefixPaper>papers;vector<int>order,length;
  unique_ptr<PrefixDP>dp;
 };
 vector<Group>groups(2);
 for(int g=0;g<2;g++){
  auto &x=groups[g];for(U64 a:A)x.data.push_back((a>>(32*g))&PrefixDP::low(32));
  train_backbone_dictionary(x.data,x.papers,32,perlevel,1,8);
  refit_all_prefix_papers(x.data,x.papers,4,32);
  if(x.papers.size()>96)return 0;
  x.dp=make_unique<PrefixDP>(32,x.papers);x.order.resize(x.papers.size());iota(x.order.begin(),x.order.end(),0);for(auto p:x.papers)x.length.push_back(p.len);x.future.resize(x.papers.size());
 }
 cerr<<"PrepCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\\n";
 for(auto &x:groups){
  mt19937 freq_rng(912367);vector<int>train_targets(P);iota(train_targets.begin(),train_targets.end(),0);shuffle(train_targets.begin(),train_targets.end(),freq_rng);
  for(int r=0;r<192;r++){
   int target=r<128?freq_rng()%P:train_targets[r-128],moves;auto chosen=x.dp->solve(x.order,x.data[target],moves);vector<char>selected(x.papers.size());for(int k:chosen){selected[k]=true;if(r>=128)x.future[k]|=1ULL<<(r-128);}
   for(int t=0;t<moves;t++){auto[from,to]=GlobalPrefix::best_move(x.order,x.length,selected);if(from<0)abort();int k=x.order[from];x.order.erase(x.order.begin()+from);x.order.insert(x.order.begin()+to,k);}
  }
  iota(x.order.begin(),x.order.end(),0);
 }
 cerr<<"PolicyCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\\n";
 for(int g=0;g<2;g++)for(int k=0;k<96;k++){
  string pattern(n,'.');auto &x=groups[g];if(k<int(x.papers.size()))for(int j=0;j<x.papers[k].len;j++)pattern[cols[32*g+j]]=char('0'+(((x.papers[k].color>>j)&1)^flip[32*g+j]));cout<<pattern<<'\\n';
 }
 cout.flush();
 for(int r=0;r<R;r++){
  int product;if(!(cin>>product)||product<0||product>=P)return 0;
  vector<pair<int,int>>operations;
  for(int g=0;g<2;g++){
   auto &x=groups[g];for(int k=0;k<int(x.papers.size());k++)x.dp->reward[k]=16+2*__builtin_popcountll(x.future[k]);
   int moves;auto chosen=x.dp->solve(x.order,x.data[product],moves);if(moves<0||moves>192)return 0;
   vector<char>selected(x.papers.size());for(int k:chosen)selected[k]=true;
   for(int i=0;i<moves;i++){
    auto[from,to]=GlobalPrefix::best_move(x.order,x.length,selected,.5,&x.future);if(from<0)return 0;
    int k=x.order[from];x.order.erase(x.order.begin()+from);x.order.insert(x.order.begin()+to,k);operations.push_back({96*g+k,96*g+to});
   }
   uint64_t bit=1ULL<<(r%64);for(int k=0;k<int(x.papers.size());k++)x.future[k]=(x.future[k]&~bit)|(selected[k]?bit:0ULL);
  }
  if(operations.size()>384)return 0;
  cout<<operations.size()<<'\\n';for(auto[k,d]:operations)cout<<k<<' '<<d<<'\\n';cout.flush();
 }
 cerr<<"FinishCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\\n";
}
'''
(root/'tools/experiments/structure2_agent_twochain.cpp').write_text(text)

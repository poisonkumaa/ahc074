from pathlib import Path
root=Path(__file__).resolve().parents[2]
source=(root/'main.cpp').read_text()
prefix=source[:source.index('int main(){')]
text=prefix+'#include "round4_design_marginal.h"\n'
text+='''
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);int n,K,P,R;if(!(cin>>n>>K>>P>>R))return 0;
 vector<U64>original(P);for(int v=0;v<P;v++){string s;cin>>s;for(int j=0;j<n;j++)if(s[j]=='1')original[v]|=1ULL<<j;}
 structure2_training_origin=clock();multi_training_budget=10;multi_training_finished=true;auto model=train_multi_model(original,32);
 int removals=argc>1?atoi(argv[1]):1,samples=argc>2?atoi(argv[2]):2,candidates=argc>3?atoi(argv[3]):16;double use_weight=argc>4?atof(argv[4]):1.;
 if(removals)marginal_remove(model.A,model.papers,samples,candidates,removals,use_weight);
 model.future=selection_teacher(model.A,model.papers);
 cerr<<"PrepCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\\n";
 auto&A=model.A;auto&papers=model.papers;auto&future=model.future;PrefixDP dp(n,papers);dp.public_data(A);vector<int>order(papers.size()),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);
 for(int k=0;k<K;k++){string pattern(n,'.');if(k<int(papers.size()))for(int j=0;j<papers[k].len;j++)pattern[model.cols[j]]=char('0'+(((papers[k].color>>j)&1)^model.flip[j]));cout<<pattern<<'\\n';}cout.flush();
 for(int r=0;r<R;r++){int product;if(!(cin>>product)||product<0||product>=P)return 0;for(int k=0;k<int(papers.size());k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);
  int moves;auto chosen=dp.solve(order,A[product],moves);if(moves<0||moves>2*K)return 0;vector<char>selected(papers.size());for(int k:chosen)selected[k]=true;vector<pair<int,int>>operations;
  for(int i=0;i<moves;i++){auto[from,to]=GlobalPrefix::best_move(order,length,selected,.5,&future);if(from<0)return 0;int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);operations.push_back({k,to});}
  U64 bit=1ULL<<(r%64);for(int k=0;k<int(papers.size());k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
  cout<<operations.size()<<'\\n';for(auto[k,d]:operations)cout<<k<<' '<<d<<'\\n';cout.flush();
 }cerr<<"FinishCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\\n";
}
'''
(root/'tools/experiments/round4_design_marginal.cpp').write_text(text)

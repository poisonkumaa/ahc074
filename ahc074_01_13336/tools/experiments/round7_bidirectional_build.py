from pathlib import Path
root=Path(__file__).resolve().parents[2]
text=(root/'main.cpp').read_text();text=text[:text.index('int main(')]
text+=r'''
#include "round7_bidirectional.h"
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);int N,K,P,R;if(!(cin>>N>>K>>P>>R))return 0;
 vector<U64>original(P);for(U64&a:original){string s;cin>>s;for(int j=0;j<N;j++)if(s[j]=='1')a|=1ULL<<j;}
 int right_count=argc>1?atoi(argv[1]):64,passes=argc>2?atoi(argv[2]):1,candidates=argc>3?atoi(argv[3]):256;
 structure2_training_origin=clock();multi_training_budget=100;
 auto model=train_multi_model(original,12345);const auto&A=model.A;vector<BidirectionalPaper>papers;vector<int>extra;bool seen[65]{};int one64=-1;mt19937 rng(43126);
 for(auto p:model.papers){int k=papers.size();U64 color=p.color;bool base=!seen[p.len];seen[p.len]=true;if(base)color=PrefixDP::low(p.len-1);else if(p.len==64&&one64<0){color=~0ULL;one64=k;}else extra.push_back(k);papers.push_back({p.len,false,color});}
 shuffle(extra.begin(),extra.end(),rng);
 for(int i=0;i<min(right_count,int(extra.size()));i++){int k=extra[i];U64 source=papers[k].color;int chosen=rng()%P;for(int v=0;v<P;v++)if((A[v]&PrefixDP::low(papers[k].len))==source){chosen=v;break;}papers[k].right=true;papers[k].color=A[chosen];}
 cerr<<"Initial right="<<right_count<<" intrinsic="<<bidirectional_average(A,papers)<<"\n";
 train_bidirectional(A,papers,passes,candidates);
 cout<<setprecision(12)<<bidirectional_average(A,papers)<<'\n';for(auto p:papers)cout<<p.len<<' '<<p.right<<' '<<p.color<<'\n';
}
'''
(root/'tools/experiments/round7_bidirectional.cpp').write_text(text)

from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'main_13853_final.cpp').read_text()
tail=s[s.index(' PrefixDP dp(n,papers);dp.public_data(A);vector<int>order(papers.size()),length;',s.index('int main()')):]
s=s[:s.index('struct ModelSimulation')]
s+='\n#include "round4_exact_refit.h"\n'
s+='''
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);int n,K,P,R;if(!(cin>>n>>K>>P>>R))return 0;
 vector<U64>original(P);for(int v=0;v<P;v++){string x;cin>>x;for(int j=0;j<n;j++)if(x[j]=='1')original[v]|=1ULL<<j;}
 structure2_training_origin=clock();multi_training_budget=100;
 int passes=argc>1?atoi(argv[1]):1,bottom=argc>2?atoi(argv[2]):43,id=argc>3?atoi(argv[3]):12345;
 auto model=train_multi_model(original,id);auto A=move(model.A);auto cols=move(model.cols);auto flip=move(model.flip);auto papers=move(model.papers);
 int repetitions=argc>4?atoi(argv[4]):1,regularizer=argc>5?atoi(argv[5]):0;exact_state_refit(A,papers,passes,bottom,32,repetitions,regularizer);auto future=selection_teacher(A,papers);
'''+tail
(d/'round4_exact_regularized.cpp').write_text(s)

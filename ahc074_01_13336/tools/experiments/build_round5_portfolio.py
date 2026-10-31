from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'round5_sampling_adaptive.cpp').read_text()
s=s.replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100').replace('#define AHC_REFINEMENT_BUDGET 1.65','#define AHC_REFINEMENT_BUDGET 100')
tail=s[s.index(' PrefixDP dp(n,papers);dp.public_data(A);vector<int>order(papers.size()),length;',s.index('int main()')):]
s=s[:s.index('int main()')]
old=(d/'round4_exact_select.cpp').read_text();sim=old[old.index('struct ModelSimulation'):old.index('int main(')]
s+=sim+r'''
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);int n,K,P,R;if(!(cin>>n>>K>>P>>R))return 0;
 vector<U64>original(P);for(int v=0;v<P;v++){string x;cin>>x;for(int j=0;j<n;j++)if(x[j]=='1')original[v]|=1ULL<<j;}
 structure2_training_origin=clock();multi_training_budget=100;vector<MultiModel>bank;
 for(int id:{12345,32}){auto model=train_multi_model(original,id);exact_state_refit(model.A,model.papers,1,43,32,argc>2?atoi(argv[2]):8,0);model.future=selection_teacher(model.A,model.papers);bank.push_back(move(model));}
 vector<unique_ptr<ModelSimulation>>sim;for(auto&m:bank)sim.push_back(make_unique<ModelSimulation>(m));
 int state_limit=argc>1?atoi(argv[1]):32;mt19937 sample_rng(377321);vector<int>targets,batch(P);iota(batch.begin(),batch.end(),0);int count=128+state_limit*16;
 while(int(targets.size())<count){shuffle(batch.begin(),batch.end(),sample_rng);if(!targets.empty()&&batch[0]==targets.back())swap(batch[0],batch[1]);for(int v:batch)if(int(targets.size())<count)targets.push_back(v);}
 for(int r=0;r<count;r++){for(auto&s:sim)s->step(targets[r],r);if(r>=128&&(r-128)%16==0)for(auto&s:sim)s->total+=s->mean_cost(targets[r],100);}
 int winner=0;for(int i=0;i<int(sim.size());i++){cerr<<"PortfolioModel="<<i<<" Estimate="<<sim[i]->total<<"\n";if(sim[i]->total<sim[winner]->total)winner=i;}
 cerr<<"PortfolioWinner="<<winner<<"\n";
 auto A=move(bank[winner].A);auto cols=move(bank[winner].cols);auto flip=move(bank[winner].flip);auto papers=move(bank[winner].papers);auto future=move(bank[winner].future);
'''+tail
(d/'round5_portfolio.cpp').write_text(s)

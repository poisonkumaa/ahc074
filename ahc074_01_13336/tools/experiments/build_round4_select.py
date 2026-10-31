from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'round4_exact_iter.cpp').read_text()
old=(d/'main_13853_final.cpp').read_text()
sim=old[old.index('struct ModelSimulation'):old.index('int main()')]
s=s.replace('int main(int argc,char**argv){',sim+'\nint main(int argc,char**argv){')
a=s.index('for(int round=0;round<(argc>6?atoi(argv[6]):1);round++)')
b=s.index('\n PrefixDP dp(n,papers);',a)
s=s[:a]+r'''
 vector<MultiModel>bank;
 for(int round=0;round<(argc>6?atoi(argv[6]):2);round++){
  exact_state_refit(A,papers,passes,bottom,32,repetitions,regularizer);
  bank.push_back({A,{}, {},papers,selection_teacher(A,papers)});
 }
 vector<unique_ptr<ModelSimulation>>sim;for(auto&m:bank)sim.push_back(make_unique<ModelSimulation>(m));
 int state_limit=argc>7?atoi(argv[7]):32;mt19937 sample_rng(377321);vector<int>targets,batch(P);iota(batch.begin(),batch.end(),0);int count=128+state_limit*16;
 while(int(targets.size())<count){shuffle(batch.begin(),batch.end(),sample_rng);if(!targets.empty()&&batch[0]==targets.back())swap(batch[0],batch[1]);for(int v:batch)if(int(targets.size())<count)targets.push_back(v);}
 for(int r=0;r<count;r++){for(auto&s:sim)s->step(targets[r],r);if(r>=128&&(r-128)%16==0)for(auto&s:sim)s->total+=s->mean_cost(targets[r],100);}
 int winner=0;for(int i=0;i<int(sim.size());i++){cerr<<"RefitModel="<<i<<" Estimate="<<sim[i]->total<<"\n";if(sim[i]->total<sim[winner]->total)winner=i;}
 cerr<<"RefitWinner="<<winner<<"\n";
 papers=move(bank[winner].papers);auto future=move(bank[winner].future);
'''+s[b:]
(d/'round4_exact_select.cpp').write_text(s)

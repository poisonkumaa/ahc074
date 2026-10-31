from pathlib import Path
p=Path(__file__).parent
s=(p/'multi_mean_first_agent.cpp').read_text()
s=s[:s.index('int main(int argc,char**argv){')]
s=s.replace('static double multi_training_budget=100.0;','static double multi_training_budget=1.0;static bool multi_training_finished=true;')
s=s.replace('if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>STRUCTURE2_TRAINING_CPU_LIMIT)break;','if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>STRUCTURE2_TRAINING_CPU_LIMIT){multi_training_finished=false;break;}')
s=s.replace('auto future=selection_teacher(A,papers);return {','if(!multi_training_finished)return {move(A),move(cols),move(flip),move(papers),{}};auto future=selection_teacher(A,papers);return {')
extra=r'''
struct ModelSimulation {
 const MultiModel&model;PrefixDP dp;vector<int>order,length;vector<uint64_t>future;long long total=0;
 ModelSimulation(const MultiModel&m):model(m),dp(64,m.papers),order(m.papers.size()),future(m.future){dp.public_data(m.A);iota(order.begin(),order.end(),0);for(auto p:m.papers)length.push_back(p.len);}
 void step(int target,int r){
  for(int k=0;k<int(order.size());k++)dp.reward[k]=16+2*__builtin_popcountll(future[k]);
  int moves;auto chosen=dp.solve(order,model.A[target],moves);vector<char>selected(order.size());for(int k:chosen)selected[k]=true;
  for(int i=0;i<moves;i++){auto[from,to]=GlobalPrefix::best_move(order,length,selected,.5,&future);if(from<0)abort();int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);}
  uint64_t bit=1ULL<<(r%64);for(int k=0;k<int(order.size());k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
 }
 int mean_cost(int current,double deadline){
  int total=0;dp.prepare(order);for(int v=0;v<int(model.A.size());v++)if(v!=current){
   if((v&31)==0&&double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>deadline)return -1;
   int distance;dp.solve_prepared(model.A[v],distance,false);total+=distance;
  }return total;
 }
};
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);int n,K,P,R;if(!(cin>>n>>K>>P>>R)||n!=64||K!=192||P!=256)return 0;
 vector<U64>original(P);for(int v=0;v<P;v++){string text;cin>>text;if(int(text.size())!=n)return 0;for(int j=0;j<n;j++)if(text[j]=='1')original[v]|=1ULL<<j;}
 structure2_training_origin=clock();int state_limit=argc>1?atoi(argv[1]):64;double deadline=argc>2?atof(argv[2]):1.4;multi_training_budget=argc>3?atof(argv[3]):1.0;int desired_models=argc>4?atoi(argv[4]):3;
 vector<MultiModel>models;vector<int>model_ids;for(int id:{32,12345,1}){
  if(int(models.size())>=desired_models)break;
  if(!models.empty()&&double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>=multi_training_budget-.04)break;
  multi_training_finished=true;auto model=train_multi_model(original,id);
  if(!multi_training_finished&&!models.empty())break;
  if(model.future.empty())model.future=selection_teacher(model.A,model.papers);
  models.push_back(move(model));model_ids.push_back(id);
  cerr<<"Train model="<<id<<" CPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
 }
 vector<unique_ptr<ModelSimulation>>sim;for(auto&m:models)sim.push_back(make_unique<ModelSimulation>(m));
 vector<int>targets,batch(P);iota(batch.begin(),batch.end(),0);mt19937 sample_rng(377321);int count=128+state_limit*8;
 while(int(targets.size())<count){shuffle(batch.begin(),batch.end(),sample_rng);if(!targets.empty()&&batch[0]==targets.back())swap(batch[0],batch[1]);for(int v:batch)if(int(targets.size())<count)targets.push_back(v);}
 int states=0;bool expired=false;
 for(int r=0;r<count&&!expired;r++){
  for(auto&s:sim){if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>deadline){expired=true;break;}s->step(targets[r],r);}
  if(expired)break;
  if(r>=128&&(r-128)%8==0){
   vector<int>cost;for(auto&s:sim){int c=s->mean_cost(targets[r],deadline);if(c<0){expired=true;break;}cost.push_back(c);}
   if(expired)break;for(int i=0;i<int(sim.size());i++)sim[i]->total+=cost[i];states++;
  }
 }
 int winner=0;for(int i=0;i<int(sim.size());i++){cerr<<"Round model="<<model_ids[i]<<" cost="<<sim[i]->total<<" states="<<states<<"\n";if(sim[i]->total<sim[winner]->total)winner=i;}
 cerr<<"Round selected="<<model_ids[winner]<<" states="<<states<<" models="<<models.size()<<" PrepCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<"\n";
 auto A=move(models[winner].A);auto cols=move(models[winner].cols);auto flip=move(models[winner].flip);auto papers=move(models[winner].papers);auto future=move(models[winner].future);
 PrefixDP dp(n,papers);dp.public_data(A);vector<int>order(papers.size()),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);
'''
old=(p/'multi_mean_first_agent.cpp').read_text()
a=old.index(' for(int k=0;k<K;k++){\n  string pattern')
(p/'multi_round_agent.cpp').write_text(s+extra+old[a:])

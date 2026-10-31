from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'prefix_policy_agent_search.cpp').read_text()
s=s.replace('vector<int>training(512);for(int i=0;i<(int)training.size();i++){do{training[i]=rng()%P;}while(i&&training[i]==training[i-1]);}', 'vector<int>training;for(int q=0;q<2;q++){vector<int>block(P);iota(block.begin(),block.end(),0);shuffle(block.begin(),block.end(),rng);if(!training.empty()&&training.back()==block[0])swap(block[0],block[1]);training.insert(training.end(),block.begin(),block.end());}')
s=s.replace('c.count>=24','c.count>=16').replace('c.count-24','c.count-16')
s=s.replace('simulate(c,152)', 'simulate(c,80)').replace('simulate(candidates[i],384)', 'simulate(candidates[i],256)')
s=s.replace('>1.00','>.65').replace('>1.10','>.78')
(p/'prefix_policy_agent_short.cpp').write_text(s)

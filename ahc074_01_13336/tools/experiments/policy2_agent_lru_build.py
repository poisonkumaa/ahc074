from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy2_agent_union_adapt.cpp').read_text()
s=s.replace(' for(int r=0;r<R;r++){',' int slotproduct[64],slotage[64];for(int i=0;i<64;i++){slotproduct[i]=train_targets[i];slotage[i]=i-64;}int mode=argc>1?atoi(argv[1]):2;\n for(int r=0;r<R;r++){')
s=s.replace('  int moves;auto chosen=dp.solve(order,A[product],moves);', '''  vector<uint64_t>queryfuture=future;
  if(mode>=3){uint64_t blocked=0;for(int i=0;i<64;i++)if(slotproduct[i]==product)blocked|=1ULL<<i;for(auto&v:queryfuture)v&=~blocked;}
  int moves;auto chosen=dp.solve(order,A[product],moves);''')
s=s.replace('selected,.5,&future);if(from<0)return 0;','selected,.5,&queryfuture);if(from<0)return 0;')
s=s.replace('{int period=argc>1?atoi(argv[1]):1;if(r%period==0){int slot=(r/period)%64;uint64_t bit=1ULL<<slot;for(int k=0;k<int(papers.size());k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);}}', '''{int slot=r%64;if(mode==2||mode==3){slot=-1;for(int i=0;i<64;i++)if(slotproduct[i]==product){slot=i;break;}if(slot<0)slot=min_element(slotage,slotage+64)-slotage;}slotproduct[slot]=product;slotage[slot]=r;uint64_t bit=1ULL<<slot;for(int k=0;k<int(papers.size());k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);}''')
(p/'policy2_agent_union_lru.cpp').write_text(s)
s=(p/'policy2_agent_union_adapt_grid.py').read_text().replace('policy2_agent_union_adapt','policy2_agent_union_lru').replace('for size in [1,2,4,8]:','for size in [1,2,3,4]:')
(p/'policy2_agent_union_lru_grid.py').write_text(s)

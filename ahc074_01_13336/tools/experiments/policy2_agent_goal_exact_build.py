from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy2_agent_goal_stats.cpp').read_text()
s=s.replace('vector<int>targets(samplecount);for(int&i:targets){do{i=freq_rng()%P;}while(i==product);}', '''vector<int>targets;if(samplecount>=255){for(int i=0;i<P;i++)if(i!=product)targets.push_back(i);}else{targets.resize(samplecount);for(int&i:targets){do{i=freq_rng()%P;}while(i==product);}}''')
start=s.index('   double quality=mandatory(p);')
end=s.index('   if(quality<bestquality',start)
body=s[start:end]
body=body.replace('double quality=mandatory(p);','double quality=mandatory(p);')+'   return quality;\n'
insert=s.index('  vector<pair<int,int>>operations;vector<int>bestorder')
s=s[:insert]+'  auto evaluate=[&](const vector<int>&p){\n'+body+'  };\n'+s[insert:]
start=s.index('   double quality=mandatory(p);',s.index('  for(int v=0;v<variants;v++)'))
end=s.index('   if(quality<bestquality',start)
s=s[:start]+'''   if(seen.size()==1){bestorder=p;bestchosen=chosen;operations=ops;continue;}
   if(seen.size()==2)bestquality=evaluate(bestorder);
   double quality=evaluate(p);
''' + s[end:]
s=s.replace('samplecount=argc>2?atoi(argv[2]):8','samplecount=argc>2?atoi(argv[2]):255')
(p/'policy2_agent_goal_exact.cpp').write_text(s)
s=(p/'policy2_agent_goal_ensemble_grid.py').read_text().replace('policy2_agent_goal_ensemble','policy2_agent_goal_exact')
s=s.replace('[(1,8,2),(3,8,0),(3,8,1),(3,4,2),(3,8,2),(3,16,2),(3,32,2),(4,8,2),(3,8,3)]','[(3,255,2),(3,255,3),(4,255,2),(3,64,2)]')
(p/'policy2_agent_goal_exact_grid.py').write_text(s)

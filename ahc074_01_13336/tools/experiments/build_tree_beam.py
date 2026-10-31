from pathlib import Path
p=Path(__file__).parent
s=(p/'beam.cpp').read_text()
s=s.replace(' vector<Card>C;', ''' vector<Card>C;
 if(mode==6){
  K=N+1;C.resize(K);
  for(int j=0;j<N;j++){int a=(j/sz),b=j+1;U bit=1ULL<<j;C[a].m|=bit;C[b].m|=bit;C[b].b|=bit;}
 }
''')
(p/'tree_beam.cpp').write_text(s)

from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'structure2_agent_backbone.cpp').read_text()
key=' for(U64&v:A){U64 old=v;v=0;for(int j=0;j<n;j++)v|=(((old>>cols[j])&1)^flip[j])<<j;}'
extra='''
 if(argc>10){
  double bias=atof(argv[10]);double f[64][2];int prev[64][2];
  for(int j=0;j<n;j++){
   int ones=0;for(U64 v:A)ones+=(v>>cols[j])&1;
   for(int b=0;b<2;b++){
    double c=bias*(b?ones:P-ones);f[j][b]=1e100;
    if(!j){f[j][b]=c;continue;}
    int d=correlation[cols[j-1]][cols[j]];
    for(int a=0;a<2;a++){double score=f[j-1][a]+((a^b)?P-d:d)+c;if(score<f[j][b])f[j][b]=score,prev[j][b]=a;}
   }
  }
  int b=f[n-1][1]<f[n-1][0];for(int j=n-1;j>=0;j--){flip[j]=b;if(j)b=prev[j][b];}
 }
'''
assert key in s
s=s.replace(key,extra+key)
(d/'refit_orientation_submit.cpp').write_text(s)

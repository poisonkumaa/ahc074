from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy_agent_v4opt.cpp').read_text()
s=s.replace('int state[8]{};long total=0;', '''int state[8]{};long total=0;
 int hist[8][9]{},allhist[8][9]{};
 auto record=[&](int s,int h[8][9]){auto&pm=g.perm[s];int p7=find(pm.begin(),pm.end(),7)-pm.begin(),p8=find(pm.begin(),pm.end(),8)-pm.begin();p7-=p8<p7;h[p7][p8]++;};''')
s=s.replace('state[z]=e.next;}}', 'state[z]=e.next;record(state[z],allhist);}record(state[z],hist);}')
s=s.replace('if(r==R-1)cerr<<"FinishCPU="', 'if(r==R-1){for(int x=0;x<8;x++){for(int y=0;y<9;y++)cerr<<hist[x][y]<<",";cerr<<"\\n";}cerr<<"MOVES\\n";for(int x=0;x<8;x++){for(int y=0;y<9;y++)cerr<<allhist[x][y]<<",";cerr<<"\\n";}} if(r==R-1)cerr<<"FinishCPU="')
(p/'policy_agent_v4trace.cpp').write_text(s)

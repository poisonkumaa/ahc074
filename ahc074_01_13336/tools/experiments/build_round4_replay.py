from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'round4_exact_fast.h').read_text()
s=s.replace('M=P*repetitions,INF=', 'newM=P*repetitions,INF=')
s=s.replace(' vector<Scenario>scenarios(M);',' static vector<Scenario>history;int M=newM+history.size();vector<Scenario>scenarios(M);')
s=s.replace('r<128+M;', 'r<128+newM;')
s=s.replace(' vector<array<int,192>>start(P);', ' copy(history.begin(),history.end(),scenarios.begin()+newM);history.assign(scenarios.begin(),scenarios.begin()+newM);\n vector<array<int,192>>start(P);')
(d/'round4_exact_replay.h').write_text(s)
s=(d/'round4_exact_iter.cpp').read_text().replace('round4_exact_fast.h','round4_exact_replay.h')
(d/'round4_exact_replay.cpp').write_text(s)

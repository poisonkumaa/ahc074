from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy3_agent_pathbeam.cpp').read_text()
s=s.replace(' for(int r=0;r<R;r++){',' int beamqueries=0,improvedstates=0,differentstates=0;\n for(int r=0;r<R;r++){')
s=s.replace('  else{\n   struct State','  else{beamqueries++;\n   struct State')
s=s.replace('order=move(beam[at].order);operations=move(beam[at].ops);', 'for(const auto&x:beam)if(x.baseline){improvedstates+=beam[at].quality<x.quality;differentstates+=beam[at].order!=x.order;break;}order=move(beam[at].order);operations=move(beam[at].ops);')
s=s.replace(' cerr<<"FinishCPU="',' cerr<<"Beam="<<beamqueries<<" Improved="<<improvedstates<<" Different="<<differentstates<<"\\n";\n cerr<<"FinishCPU="')
(p/'policy3_agent_pathbeam_stats.cpp').write_text(s)

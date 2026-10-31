from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'prefix_policy_agent_deep.cpp').read_text()
s=s.replace('vector<Candidate>candidates;', 'vector<Candidate>candidates;double last_candidate_cost=0;bool stop_candidates=false;')
s=s.replace('if(dir)reverse(cols.begin(),cols.end());', 'if(!candidates.empty()&&double(clock()-started)/CLOCKS_PER_SEC+1.35*last_candidate_cost>1.10){stop_candidates=true;break;}\n   if(dir)reverse(cols.begin(),cols.end());\n   clock_t candidate_started=clock();')
s=s.replace('candidates.push_back(move(c));', 'candidates.push_back(move(c));last_candidate_cost=double(clock()-candidate_started)/CLOCKS_PER_SEC;')
s=s.replace('if(candidates.size()>=3||double(clock()-started)/CLOCKS_PER_SEC>.92)break;', 'if(stop_candidates||candidates.size()>=3||double(clock()-started)/CLOCKS_PER_SEC>1.10)break;')
(p/'prefix_policy_agent_guard.cpp').write_text(s)

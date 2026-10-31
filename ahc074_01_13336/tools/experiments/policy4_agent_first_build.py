from pathlib import Path
p=Path(__file__).resolve().parent
s=(p.parents[1]/'main.cpp').read_text();(p/'policy4_agent_base.cpp').write_text(s)
a=s.index('inline std::pair<int,int> best_move(');b=s.index('\n}\n}',a)+2
t=s[a:b].replace('best_move(', 'first_move(',1).replace('''        }
    }
    return answer;''','''        }
        if(answer.first>=0)break;
    }
    return answer;''')
s=s[:b]+ '\n'+t+s[b:]
s=s.replace(' for(int r=0;r<R;r++){',' int different=0;\n for(int r=0;r<R;r++){')
s=s.replace('  vector<pair<int,int>>operations;operations.reserve(moves);','''  auto probe=order;for(int i=0;i<moves;i++){auto[from,to]=GlobalPrefix::first_move(probe,length,selected,.5,&future);if(from<0)abort();int k=probe[from];probe.erase(probe.begin()+from);probe.insert(probe.begin()+to,k);}
  vector<pair<int,int>>operations;operations.reserve(moves);''')
s=s.replace('  {int period=1;', '  if(probe!=order){if(!different)cerr<<"First difference r="<<r<<" moves="<<moves<<"\\n";different++;}\n  {int period=1;')
s=s.replace(' cerr<<"FinishCPU="',' cerr<<"Different="<<different<<"\\n";\n cerr<<"FinishCPU="')
(p/'policy4_agent_first_compare.cpp').write_text(s)

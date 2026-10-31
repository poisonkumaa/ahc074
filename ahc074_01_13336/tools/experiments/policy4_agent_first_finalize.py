from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy4_agent_first_compare.cpp').read_text()
s=s.replace(' for(int r=0;r<128+samples;r++){',' int teacher_different=0;\n for(int r=0;r<128+samples;r++){')
s=s.replace('  for(int t=0;t<moves;t++){auto[from,to]=GlobalPrefix::best_move(order,length,selected);', '''  auto teacher_probe=order;for(int t=0;t<moves;t++){auto[from,to]=GlobalPrefix::first_move(teacher_probe,length,selected);if(from<0)abort();int k=teacher_probe[from];teacher_probe.erase(teacher_probe.begin()+from);teacher_probe.insert(teacher_probe.begin()+to,k);}
  for(int t=0;t<moves;t++){auto[from,to]=GlobalPrefix::best_move(order,length,selected);''')
s=s.replace(' }\n cerr<<"PolicyCPU="','  teacher_different+=teacher_probe!=order;\n }\n cerr<<"TeacherDifferent="<<teacher_different<<"\\n";\n cerr<<"PolicyCPU="')
(p/'policy4_agent_first_compare.cpp').write_text(s)
s=(p/'policy4_agent_base.cpp').read_text();a=s.index('inline std::pair<int,int> best_move(');b=s.index('\n}\n}',a)+2
t=s[a:b].replace('''        }
    }
    return answer;''','''        }
        if(answer.first>=0)break;
    }
    return answer;''')
s=s[:a]+t+s[b:]
(p/'policy4_agent_first.cpp').write_text(s)

from pathlib import Path
p=Path(__file__).parent
s=(p/'multi_round_agent.cpp').read_text()
t=(p/'policy4_agent_direct_compare.cpp').read_text()
direct=t[t.index('vector<int> direct_final('):t.index('int main(){')]
s=s.replace('inline vector<uint64_t> selection_teacher(',direct+'\ninline vector<uint64_t> selection_teacher(')
teacher='for(int t=0;t<moves;t++){auto[from,to]=GlobalPrefix::best_move(order,length,selected);if(from<0)abort();int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);}'
step='for(int i=0;i<moves;i++){auto[from,to]=GlobalPrefix::best_move(order,length,selected,.5,&future);if(from<0)abort();int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);}'
check=s.replace(teacher,'auto predicted=direct_final(order,length,selected);'+teacher+'if(order!=predicted){cerr<<"teacher mismatch\\n";abort();}')
check=check.replace(step,'auto predicted=direct_final(order,length,selected,&future);'+step+'if(order!=predicted){cerr<<"simulation mismatch\\n";abort();}')
(p/'multi_round_direct_check.cpp').write_text(check)
s=s.replace(teacher,'order=direct_final(order,length,selected);')
s=s.replace(step,'order=direct_final(order,length,selected,&future);')
(p/'multi_round_direct_agent.cpp').write_text(s)
a=s.index('inline void refit_all_prefix_papers(');b=s.index('vector<int> direct_final(',a)
f=s[a:b]
f=f.replace('double initial=distances();int accepted=0,rejected=0;','double initial=distances();int accepted=0,rejected=0;bool dirty=false;')
f=f.replace('int amount=level[l].size();if(!amount)continue;distances();','int amount=level[l].size();if(!amount)continue;if(dirty){distances();dirty=false;}')
f=f.replace('int k=level[l][q];papers[k].color=pattern[q];update(k);','int k=level[l][q];if(papers[k].color!=pattern[q]){papers[k].color=pattern[q];update(k);dirty=true;}')
(p/'multi_round_fast_agent.cpp').write_text(s[:a]+f+s[b:])

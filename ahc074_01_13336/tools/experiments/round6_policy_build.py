from pathlib import Path
p=Path(__file__).resolve().parent
s=(p.parent.parent/'main.cpp').read_text().replace('exact_state_refit(A,papers,1,43,32,-1,0);','exact_state_refit(A,papers,1,43,32,32,0);')
(p/'round6_policy_base.cpp').write_text(s)
s=s.replace('cout.flush();int fallbacks=0;', '''cout.flush();int fallbacks=0;
 long long records=0,mandatory=0,mandatory_selected=0,mandatory_hidden=0,lis_moves=0,free_goal_moves=0,free_visible_moved=0;int free_accessible_queries=0,lis_savings_queries=0,lis_max_saving=0;vector<int>usage(papers.size()),movecount(papers.size()),badmoves(papers.size());
''')
s=s.replace('  vector<pair<int,int>>operations;auto before=order;', '''  records+=chosen.size();for(int k:chosen)usage[k]++;
  vector<char>visible(papers.size());int high=0;for(int k:order)if(length[k]>high){visible[k]=1;high=length[k];}
  int suffix=65;for(int i=int(order.size())-1;i>=0;i--){int k=order[i];if(length[k]>=suffix){mandatory++;mandatory_selected+=selected[k];mandatory_hidden+=!visible[k];}if(selected[k])suffix=min(suffix,length[k]);}
  vector<pair<int,int>>operations;auto before=order;''')
s=s.replace('  uint64_t bit=1ULL<<(r%64);', '  for(auto [k,d]:operations){movecount[k]++;badmoves[k]+=!selected[k];}\n  uint64_t bit=1ULL<<(r%64);')
s=s.replace('  if(!direct_moves(order,length,selected,target,operations)', '''  {vector<int>rank(papers.size()),lis;for(int i=0;i<int(target.size());i++)rank[target[i]]=i;for(int k:order){auto at=lower_bound(lis.begin(),lis.end(),rank[k]);if(at==lis.end())lis.push_back(rank[k]);else *at=rank[k];}int minimum=order.size()-lis.size(),saving=moves-minimum;lis_moves+=minimum;lis_savings_queries+=saving>0;lis_max_saving=max(lis_max_saving,saving);}
  {int ownerlength[65];for(int l=0;l<=64;l++){ownerlength[l]=64;for(int k:chosen)if(length[k]>=l){ownerlength[l]=length[k];break;}}array<int,65>cost,next;cost.fill(1000000);cost[0]=0;for(int k:order){next.fill(1000000);for(int h=0;h<=64;h++){next[h]=min(next[h],cost[h]+256-visible[k]);int key=ownerlength[length[k]];if(!selected[k]||length[k]>h)next[max(h,key)]=min(next[max(h,key)],cost[h]);}cost=next;}int value=*min_element(cost.begin(),cost.end()),distance=(value+255)/256,access=distance*256-value;free_goal_moves+=distance;free_visible_moved+=access;free_accessible_queries+=access>0;}
  if(!direct_moves(order,length,selected,target,operations)''')
s=s.replace(' cerr<<"PathFallbacks="', ''' cerr<<"Records="<<records<<" Mandatory="<<mandatory<<" MandatorySelected="<<mandatory_selected<<" MandatoryHidden="<<mandatory_hidden<<" LISMoves="<<lis_moves<<" LISSavingsQueries="<<lis_savings_queries<<" LISMaxSaving="<<lis_max_saving<<" FreeGoalMoves="<<free_goal_moves<<" FreeVisibleMoved="<<free_visible_moved<<" FreeAccessibleQueries="<<free_accessible_queries<<"\\n";
 for(int k=0;k<int(papers.size());k++)cerr<<"Paper "<<k<<" "<<length[k]<<" "<<usage[k]<<" "<<movecount[k]<<" "<<badmoves[k]<<"\\n";
 cerr<<"PathFallbacks="''')
(p/'round6_policy_stats.cpp').write_text(s)

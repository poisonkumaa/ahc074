from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'round4_portfolio_fast.cpp').read_text()
s=s.replace('int n,k,epoch=0;', 'int n,k,epoch=0,primary=65536;')
s=s.replace('cnt,work,parent,which,stamp;', 'cnt,work,parent,which,stamp,opcount;')
s=s.replace('stamp(n+1),reward(k,16)', 'stamp(n+1),opcount(n+1),reward(k,16)')
s=s.replace('dp[0][0]=0;stamp[0][0]', 'dp[0][0]=0;opcount[0][0]=0;stamp[0][0]')
s=s.replace('add*65536-reward[z]', 'add*primary-reward[z]')
s=s.replace('dp[l][nh]=value;if(reconstruct)', 'dp[l][nh]=value;opcount[l][nh]=opcount[j][h]+add;if(reconstruct)')
s=s.replace('best=(dp[n][h]+65535)/65536;', 'best=opcount[n][h];')
s=s.replace('int main(){','int main(int argc,char**argv){')
s=s.replace('constexpr int MAXC=R4_MAXC, PASSES=R4_PASSES, PILOT=R4_PILOT;', 'constexpr int MAXC=1, PASSES=4, PILOT=544;')
s=s.replace('auto refined=refine_columns(bestcols,input,correlation,10000,8);', 'auto refined=bestcols;')
a=s.index(' vector<Candidate>bank;double last=.20;');b=s.index(' iota(c.order.begin()',a)
s=s[:a]+''' Candidate c=build(0);simulate(c,544);cerr<<"Fixed design variant=0 PrepCPU="<<elapsed()<<"\\n";
''' +s[b:]
helper='''struct GoalChoice{vector<int>papers;int moves;};
static long long evaluated_goals=0,changed_goals=0,extra_moves=0,lookahead_queries=0;
GoalChoice choose_rollout(PrefixDP&dp,const vector<int>&order,const vector<int>&length,const vector<U64>&targets,int product,const vector<Access>&future,int mode,int sample_count,int horizon,double threshold,mt19937&rng){
 int K=order.size(),P=targets.size();vector<int>frequency(K);for(int k=0;k<K;k++)frequency[k]=maskpop(future[k]);
 dp.primary=65536;for(int k=0;k<K;k++)dp.reward[k]=16+2*frequency[k];int baseline_moves;auto baseline=dp.solve(order,targets[product],baseline_moves);if(!mode)return {baseline,baseline_moves};
 vector<GoalChoice>choices{{baseline,baseline_moves}};
 int configs=mode>=2?6:4;
 for(int v=1;v<configs;v++){
  int alpha=v==1?0:v==2?1:v==3?4:2;dp.primary=v==4?1024:v==5?512:65536;
  for(int k=0;k<K;k++)dp.reward[k]=16+alpha*frequency[k];
  int moves;auto goal=dp.solve(order,targets[product],moves);if(moves>baseline_moves+1)continue;
  bool duplicate=false;for(const auto&old:choices)duplicate|=old.papers==goal;if(!duplicate)choices.push_back({move(goal),moves});
 }
 dp.primary=65536;if(choices.size()==1)return choices[0];lookahead_queries++;evaluated_goals+=choices.size();
 vector<int>next1,next2,permutation(P);iota(permutation.begin(),permutation.end(),0);shuffle(permutation.begin(),permutation.end(),rng);
 for(int a:permutation)if(a!=product&&int(next1.size())<sample_count){next1.push_back(a);int b=rng()%(P-1);if(b>=a)b++;next2.push_back(b);}
 sample_count=next1.size();long long bestcost=LLONG_MAX;int winner=0;
 for(int v=0;v<int(choices.size());v++){
  vector<char>selected(K);for(int k:choices[v].papers)selected[k]=true;auto final=GlobalPrefix::final_order(order,length,selected,&future);
  auto cache=future;Access bit=Access::bit(product);for(int k=0;k<K;k++){cache[k]=(cache[k]&~bit)|(selected[k]?bit:Access(0));dp.reward[k]=16+2*maskpop(cache[k]);}
  long long total=1LL*choices[v].moves*sample_count;
  for(int q=0;q<sample_count;q++){
   int cost1;auto goal1=dp.solve(final,targets[next1[q]],cost1,horizon>=2);total+=cost1;
   if(horizon>=2){vector<char>selected1(K);for(int k:goal1)selected1[k]=true;auto final1=GlobalPrefix::final_order(final,length,selected1,&cache);int cost2;dp.solve(final1,targets[next2[q]],cost2,false);total+=cost2;}
  }
  if(v==0||double(total)+threshold*sample_count<double(bestcost)){bestcost=total;winner=v;}
 }
 changed_goals+=winner!=0;extra_moves+=choices[winner].moves-baseline_moves;return choices[winner];
}
'''
a=s.index('int main(');s=s[:a]+helper+s[a:]
s=s.replace(' for(int r=0;r<R;r++){',' mt19937 rollout_rng(183746);int mode=argc>1?atoi(argv[1]):0,sample_count=argc>2?atoi(argv[2]):32,horizon=argc>3?atoi(argv[3]):2;double threshold=argc>4?atof(argv[4]):.05;\n for(int r=0;r<R;r++){')
old='for(int k=0;k<int(c.paper.size());k++)c.dp->reward[k]=16+2*maskpop(c.future[k]);int m;auto chosen=c.dp->solve(c.order,c.target[a],m);if(m<0||m>2*K)return 0;vector<char>selected(c.paper.size());for(int k:chosen)selected[k]=true;'
new='auto proposal=choose_rollout(*c.dp,c.order,c.length,c.target,a,c.future,mode,sample_count,horizon,threshold,rollout_rng);int m=proposal.moves;if(m<0||m>2*K)return 0;vector<char>selected(c.paper.size());for(int k:proposal.papers)selected[k]=true;'
assert old in s;s=s.replace(old,new)
s=s.replace(' if(DIAGNOSTICS)cerr<<"FinishCPU="', ' cerr<<"Lookahead="<<lookahead_queries<<" Candidates="<<evaluated_goals<<" Changed="<<changed_goals<<" Extra="<<extra_moves<<"\\n";\n if(DIAGNOSTICS)cerr<<"FinishCPU="')
(p/'round4_policy_rollout.cpp').write_text(s)

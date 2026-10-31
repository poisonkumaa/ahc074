from pathlib import Path
import sys
p=Path(__file__).parent
iterative=len(sys.argv)>1 and sys.argv[1]=='iter'
v7=(p/'round4_portfolio_fast.cpp').read_text()
access=v7[v7.index('#ifndef WORDS'):v7.index('struct PrefixPaper')]
a=v7.index('namespace GlobalPrefix {');b=v7.index('#ifndef STRUCTURE2_TRAINING_CPU_LIMIT',a)
moves=v7[a:b].replace('namespace GlobalPrefix {','namespace GlobalPrefix256 {')
direct=(p/'policy4_agent_direct.h').read_text()
direct=direct[direct.index('namespace GlobalPrefix {'):].replace('namespace GlobalPrefix {','namespace GlobalPrefix256 {')
direct=direct.replace('std::vector<uint64_t>*future','std::vector<Access>*future').replace('uint64_t below[192]','Access below[192]').replace(':0ULL',':Access(0)').replace('__builtin_popcountll','maskpop')
(p/'round4_portfolio_access256.h').write_text('#pragma once\n'+access+moves+direct)
s=(p/('round4_exact_iter.cpp' if iterative else 'round4_exact_fast.cpp')).read_text()
teacher=r'''
vector<Access> portfolio_catalogue_teacher(const vector<U64>&A,const vector<PrefixPaper>&papers,int warm){
 int P=A.size();PrefixDP dp(64,papers);dp.public_data(A);vector<int>order(papers.size()),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);vector<Access>future(papers.size());
 dp.prepare(order);for(int v=0;v<P;v++){int moves;auto goal=dp.solve_prepared(A[v],moves);for(int k:goal)future[k]|=Access::bit(v);}
 mt19937 rng(983212);vector<int>batch(P);iota(batch.begin(),batch.end(),0);int last=-1;
 for(int r=0;r<warm;r++){
  int v;if(r<128){v=rng()%P;while(v==last)v=rng()%P;}else{if((r-128)%P==0)shuffle(batch.begin(),batch.end(),rng);v=batch[(r-128)%P];}last=v;
  for(int k=0;k<int(papers.size());k++)dp.reward[k]=16+2*maskpop(future[k]);int moves;auto goal=dp.solve(order,A[v],moves);vector<char>selected(papers.size());for(int k:goal)selected[k]=true;
  order=GlobalPrefix256::final_order(order,length,selected,&future);Access bit=Access::bit(v);for(int k=0;k<int(papers.size());k++)future[k]=(future[k]&~bit)|(selected[k]?bit:Access(0));
 }return future;
}
'''
s=s.replace('int main(int argc,char**argv){','#include "round4_portfolio_access256.h"\n'+teacher+'\nint main(int argc,char**argv){')
a=s.index('int main(int argc,char**argv){');main=s[a:]
warm_index=7 if iterative else 6
main=main.replace('auto future=selection_teacher(A,papers);',f'auto future256=portfolio_catalogue_teacher(A,papers,argc>{warm_index}?atoi(argv[{warm_index}]):384);')
main=main.replace('__builtin_popcountll(future[k])','maskpop(future256[k])').replace('GlobalPrefix::best_move(order,length,selected,.5,&future)','GlobalPrefix256::best_move(order,length,selected,.5,&future256)')
main=main.replace('{int period=1;if(r%period==0){int slot=(r/period)%64;uint64_t bit=1ULL<<slot;for(int k=0;k<int(papers.size());k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);}}','{Access bit=Access::bit(product);for(int k=0;k<int(papers.size());k++)future256[k]=(future256[k]&~bit)|(selected[k]?bit:Access(0));}')
(p/('round4_portfolio_iter.cpp' if iterative else 'round4_portfolio_exact.cpp')).write_text(s[:a]+main)

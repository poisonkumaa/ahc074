from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'round7_final_fast.cpp').read_text()
s=s.replace('int main(){','#include "round7_inversion_goal.h"\nint main(int argc,char**argv){\n int r7weight=argc>1?atoi(argv[1]):0;long long r7inversions=0,r7extra=0;')
s=s.replace('PrefixDP dp(n,papers);dp.public_data(A);','R7InversionDP dp(n,papers);dp.public_data(A);')
s=s.replace('auto chosen=dp.solve(order,A[product],moves);','auto chosen=dp.solve_geometry(order,A[product],moves,r7weight);int shortest;dp.PrefixDP::solve(order,A[product],shortest,false);r7extra+=moves-shortest;')
s=s.replace('auto target=direct_final(order,length,selected,&future);','''auto target=direct_final(order,length,selected,&future);
  int inv=0;for(int i=0;i<int(target.size());i++)for(int j=i+1;j<int(target.size());j++)inv+=length[target[i]]>length[target[j]];
  if(inv!=dp.last_inversions){cerr<<"Inv mismatch r="<<r<<" predicted="<<dp.last_inversions<<" actual="<<inv<<"\\n";return 99;}r7inversions+=inv;''')
s=s.replace('cerr<<"R7FinalCalls="','cerr<<"R7Weight="<<r7weight<<" MeanInversions="<<double(r7inversions)/R<<" ExtraMoves="<<r7extra<<"\\n";cerr<<"R7FinalCalls="')
(p/'round7_inversion_goal.cpp').write_text(s)

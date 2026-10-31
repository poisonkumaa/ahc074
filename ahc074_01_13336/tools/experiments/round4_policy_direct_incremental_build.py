from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy4_agent_direct.h').read_text()
s=s.replace('final_order(', 'final_order_incremental(',1)
s=s.replace(' for(int k:insert){auto&v=children[owner[length[k]]];int M=v.size(),kl=length[k];uint64_t below[192]{},remain[193]{};int inversions[192]{},lower_after[193]{};\n  for(int i=M-1;i>=0;i--){int x=v[i];for(int j=i+1;j<M;j++){int y=v[j];if(length[y]<length[x])inversions[x]++;if(future&&length[y]<=length[x])below[x]|=(*future)[y];}remain[i]=remain[i+1]|(future&&length[x]<=kl?(*future)[x]:0ULL);lower_after[i]=lower_after[i+1]+(length[x]<kl);}', ''' uint64_t below[192]{};int inversions[192]{};
 for(auto&v:children)for(int i=int(v.size())-1;i>=0;i--){int x=v[i];for(int j=i+1;j<int(v.size());j++){int y=v[j];if(length[y]<length[x])inversions[x]++;if(future&&length[y]<=length[x])below[x]|=(*future)[y];}}
 for(int k:insert){auto&v=children[owner[length[k]]];int M=v.size(),kl=length[k];uint64_t remain[193];int lower_after[193];remain[M]=0;lower_after[M]=0;
  for(int i=M-1;i>=0;i--){int x=v[i];remain[i]=remain[i+1]|(future&&length[x]<=kl?(*future)[x]:0ULL);lower_after[i]=lower_after[i+1]+(length[x]<kl);}''')
s=s.replace('  v.insert(v.begin()+at,k);', '''  below[k]=remain[at];inversions[k]=lower_after[at];for(int i=0;i<at;i++){int x=v[i];if(future&&length[x]>=kl)below[x]|=(*future)[k];if(length[x]>kl)inversions[x]++;}
  v.insert(v.begin()+at,k);''')
(p/'round4_policy_direct_incremental.h').write_text(s)
s=(p/'round4_policy_direct_exact.cpp').read_text()
a=s.index('int main(');s=s[:a]+(p/'round4_policy_direct_incremental.h').read_text().replace('#pragma once','')+'\n'+s[a:]
s=s.replace('auto target=direct_final(order,length,selected,&future);', 'auto target=GlobalPrefix::final_order_incremental(order,length,selected,&future);')
(p/'round4_policy_direct_incremental.cpp').write_text(s)

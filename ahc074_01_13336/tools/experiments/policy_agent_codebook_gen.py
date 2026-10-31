from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'templates_agent_hub.cpp').read_text()
old='for(int i=0;i<K;i++){M[i]=i<N?((1ULL<<(i+1))-1):(rnd.next()&all);B[i]=rnd.next()&M[i];}'
new='for(int i=0;i<K;i++){int j=i/3;U64 lower=j?((1ULL<<j)-1):0;M[i]=(rnd.next()&lower)|(1ULL<<j);B[i]=rnd.next()&M[i];}'
assert old in s
s=s.replace(old,new)
(p/'policy_agent_codebook_hub.cpp').write_text(s)
s=s.replace('U64 om=M[k],ob=B[k];','if(j>k/3)continue;if(j==k/3&&v==0)v=1+rnd.pick(2);U64 om=M[k],ob=B[k];')
(p/'policy_agent_codebook_fixed.cpp').write_text(s)

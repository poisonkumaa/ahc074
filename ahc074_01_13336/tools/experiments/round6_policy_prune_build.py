from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'round6_policy_base.cpp').read_text()
s=s.replace('int main(){','#include "round6_policy_prune.h"\nint main(int argc,char**argv){')
s=s.replace('ios::sync_with_stdio(false);cin.tie(nullptr);','ios::sync_with_stdio(false);cin.tie(nullptr);\n int prune_limit=argc>1?atoi(argv[1]):16,sample_count=argc>2?atoi(argv[2]):512,when=argc>3?atoi(argv[3]):0;')
s=s.replace(' exact_state_refit(A,papers,1,43,32,32,0);',' if(when)prune_dictionary(A,papers,prune_limit,sample_count);\n exact_state_refit(A,papers,1,43,32,32,0);\n if(!when)prune_dictionary(A,papers,prune_limit,sample_count);')
(p/'round6_policy_prune.cpp').write_text(s)

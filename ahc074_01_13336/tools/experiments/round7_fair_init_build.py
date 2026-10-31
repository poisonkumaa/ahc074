from pathlib import Path
p=Path(__file__).parent
s=(p/'round7_fair.cpp').read_text()
s=s.replace('inline void train_backbone_dictionary(', 'static vector<int>round7_initial_weight;\ninline void train_backbone_dictionary(',1)
start=s.index('inline void train_backbone_dictionary(');end=s.index('// Construct the final permutation',start)
f=s[start:end]
old='int child[2]{-1,-1},weight=0,sample=0,available=1;std::array<int,17>dp,split;'
new='int child[2]{-1,-1},sample=0,available=1;long long weight=0;std::array<long long,17>dp;std::array<int,17>split;'
assert f.count(old)==2;f=f.replace(old,new)
f=f.replace('int value=(a<0?0:', 'long long value=(a<0?0:')
f=f.replace('.weight+=delta;', '.weight+=1LL*(round7_initial_weight.empty()?1:round7_initial_weight[v])*delta;')
s=s[:start]+f+s[end:]
s=s.replace(' vector<array<int,192>>start(P);', ' if(round7_fair_alpha!=0)round7_initial_weight=fair_weight;\n vector<array<int,192>>start(P);',1)
old='''exact_state_refit(A,papers,1,43,32,repetitions,0);
 if(second)'''
new='''exact_state_refit(A,papers,0,43,32,4,0);
 train_backbone_dictionary(A,papers,64,6,1,8);refit_all_prefix_papers(A,papers,4,32);round7_fair_alpha=0;
 exact_state_refit(A,papers,1,43,32,repetitions,0);
 if(second)'''
assert old in s;s=s.replace(old,new)
(p/'round7_fair_init.cpp').write_text(s)

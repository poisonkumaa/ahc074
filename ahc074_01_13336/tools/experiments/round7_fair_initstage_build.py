from pathlib import Path
p=Path(__file__).parent
s=(p/'round7_fair_init.cpp').read_text()
old='train_backbone_dictionary(A,papers,64,6,1,8);refit_all_prefix_papers(A,papers,4,32);round7_fair_alpha=0;'
new='''auto saved_weights=round7_initial_weight;int initial_mode=argc>5?atoi(argv[5]):1;if(initial_mode==2)round7_initial_weight.clear();
 train_backbone_dictionary(A,papers,64,6,1,8);if(initial_mode==1)round7_initial_weight.clear();else if(initial_mode==2)round7_initial_weight=saved_weights;
 refit_all_prefix_papers(A,papers,4,32);round7_fair_alpha=0;'''
assert old in s;s=s.replace(old,new)
(p/'round7_fair_initstage.cpp').write_text(s)

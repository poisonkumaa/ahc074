from pathlib import Path
root=Path(__file__).resolve().parents[2]
text=(root/'tools/experiments/round4_design_marginal.cpp').read_text()
text=text.replace('inline void train_backbone_dictionary(', 'static int allocation_mode=0;\ninline void train_backbone_dictionary(',1)
text=text.replace('int amount=std::min(perlevel,int(extra[l].size()+spare.size()));', '''int requested=perlevel;
  if(allocation_mode==1)requested=l>=57?4:6;
  if(allocation_mode==2)requested=l>=57?8:4;
  if(allocation_mode==3)requested=l%2?4:8;
  if(allocation_mode==4)requested=l%2?0:12;
  if(allocation_mode==5)requested=l>=57?2:8;
  if(allocation_mode==6)requested=l>=57?10:4;
  if(allocation_mode==7)requested=l>=49?4:10;
  if(allocation_mode==8)requested=l>=49?8:2;
  if(allocation_mode==9)requested=l%3?9:0;
  if(allocation_mode==10)requested=l>=57?4:l>=49?6:8;
  if(allocation_mode==11)requested=l>=61?2:l>=53?6:8;
  if(allocation_mode==12)requested=l==64?12:6;
  if(allocation_mode==13)requested=l==64?0:6;
  if(allocation_mode==14)requested=l==64?2:6;
  int amount=std::min(requested,int(extra[l].size()+spare.size()));''')
text=text.replace('std::array<int,9>dp,split','std::array<int,17>dp,split')
text=text.replace('structure2_training_origin=clock();multi_training_budget=10;', 'allocation_mode=argc>1?atoi(argv[1]):0;structure2_training_origin=clock();multi_training_budget=10;')
text=text.replace('int removals=argc>1?atoi(argv[1]):1,','int removals=0,')
text=text.replace('train_multi_model(original,32)', 'train_multi_model(original,argc>5?atoi(argv[5]):32)')
(root/'tools/experiments/round4_design_profile.cpp').write_text(text)

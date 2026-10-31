from pathlib import Path
root=Path(__file__).resolve().parents[2]
text=(root/'main.cpp').read_text()
text=text.replace('inline void train_backbone_dictionary(',
 'static int round5_profile=0,round5_rotation=0,round5_cutoff=57,round5_upper=8,round5_lower=4;\ninline void train_backbone_dictionary(',1)
text=text.replace('int amount=std::min(perlevel,int(extra[l].size()+spare.size()));',
 'int requested=round5_profile?(l>=round5_cutoff?round5_upper:round5_lower):perlevel;int amount=std::min(requested,int(extra[l].size()+spare.size()));')
text=text.replace('std::array<int,9>dp,split','std::array<int,17>dp,split')
text=text.replace('cols=bestcols;if(random_seed==32)',
 'cols=bestcols;if(round5_rotation<0)reverse(cols.begin(),cols.end());else if(round5_rotation)rotate(cols.begin(),cols.begin()+round5_rotation,cols.end());if(random_seed==32)')
text=text.replace('int main(){','''int main(int argc,char**argv){
 round5_profile=argc>1?atoi(argv[1]):0;
 round5_rotation=argc>2?atoi(argv[2]):0;
 const int round5_repetitions=argc>3?atoi(argv[3]):8;
 round5_cutoff=argc>4?atoi(argv[4]):57;
 round5_upper=argc>5?atoi(argv[5]):8;
 round5_lower=argc>6?atoi(argv[6]):4;''')
text=text.replace('exact_state_refit(A,papers,1,43,32,8,0);',
 'exact_state_refit(A,papers,1,43,32,round5_repetitions,0);')
(root/'tools/experiments/round5_design.cpp').write_text(text)

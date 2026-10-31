from pathlib import Path
d=Path(__file__).resolve().parent
s=(d.parents[1]/'main.cpp').read_text()
s=s.replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100').replace('#define AHC_REFINEMENT_BUDGET 1.65','#define AHC_REFINEMENT_BUDGET 100')
s=s.replace('inline void train_backbone_dictionary(', 'static int allocation[65];\ninline void train_backbone_dictionary(')
s=s.replace('l>=53?8:4','allocation[l]')
s=s.replace('int main(){', '''int main(int argc,char**argv){
 int lo=argc>1?atoi(argv[1]):33;double alpha=argc>2?atof(argv[2]):0;int reps=argc>3?atoi(argv[3]):8;
 double weights[65]{};int remaining=127;
 for(int l=lo;l<=64;l++)weights[l]=pow(double(l-lo+1)/(65-lo),alpha);
 for(int i=0;i<remaining;i++){int best=-1;double value=-1;for(int l=lo;l<=64;l++)if(allocation[l]<15){double q=weights[l]/(allocation[l]+.5);if(q>value){value=q;best=l;}}assert(best>=0);allocation[best]++;}
 cerr<<"Allocation=";for(int l=1;l<=64;l++)if(allocation[l])cerr<<l<<":"<<allocation[l]<<",";cerr<<"\\n";
''')
s=s.replace('exact_state_refit(A,papers,1,43,32,-1,0);','exact_state_refit(A,papers,1,min(43,lo),32,reps,0);')
(d/'round7_allocation.cpp').write_text(s)

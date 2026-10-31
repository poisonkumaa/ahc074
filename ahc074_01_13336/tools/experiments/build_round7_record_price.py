from pathlib import Path
d=Path(__file__).resolve().parent
s=(d.parents[1]/'main.cpp').read_text().replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100').replace('#define AHC_REFINEMENT_BUDGET 1.65','#define AHC_REFINEMENT_BUDGET 100')
s=s.replace('int main(){','int main(int argc,char**argv){int price=argc>1?lround(atof(argv[1])*65536):0;int reps=argc>2?atoi(argv[2]):8;long long records=0,extras=0;')
s=s.replace('exact_state_refit(A,papers,1,43,32,-1,0);','exact_state_refit(A,papers,1,43,32,reps,0);')
at=s.index('int main(int argc')
a=s[:at];b=s[at:]
b=b.replace('dp.reward[k]=16+2*__builtin_popcountll(future[k]);','dp.reward[k]=16+2*__builtin_popcountll(future[k])-price;')
b=b.replace('if(moves<0||moves>2*K)return 0;','')
b=b.replace('  vector<pair<int,int>>operations;auto before=order;','  moves=GlobalPrefix::fixed_distance(order,length,selected);records+=chosen.size();if(moves<0||moves>2*K)return 0;\n  vector<pair<int,int>>operations;auto before=order;')
b=b.replace(' cerr<<"PathFallbacks="',' cerr<<"GoalRecords="<<records<<"\\n";cerr<<"PathFallbacks="')
(d/'round7_record_price.cpp').write_text(a+b)

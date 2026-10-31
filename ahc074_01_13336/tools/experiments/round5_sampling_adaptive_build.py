from pathlib import Path
p=Path(__file__).parent
s=(p/'round5_sampling_cut53.cpp').read_text()
s=s.replace('#define AHC_REFINEMENT_BUDGET 1.45','#define AHC_REFINEMENT_BUDGET 1.65')
s=s.replace(' const int N=64,K=papers.size(),P=A.size(),M=P*repetitions,INF=255,unit=1;',
''' const bool adaptive=repetitions<0;
 if(adaptive)repetitions=32;
 const int N=64,K=papers.size(),P=A.size(),INF=255,unit=1;
 int M=P*repetitions;clock_t calibration_start=0;''')
s=s.replace('   if(r>=128){auto&s=scenarios[r-128];', '   if(r==128)calibration_start=clock();\n   if(r>=128){auto&s=scenarios[r-128];')
old='''   order=direct_final(order,length,selected,&future);U64 bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
  }
 }'''
new='''   order=direct_final(order,length,selected,&future);U64 bit=1ULL<<(r%64);for(int k=0;k<K;k++)future[k]=(future[k]&~bit)|(selected[k]?bit:0ULL);
   if(adaptive&&r==128+4*P-1){
    double per_repetition=double(clock()-calibration_start)/CLOCKS_PER_SEC/4;
    double before_capture=double(calibration_start-structure2_training_origin)/CLOCKS_PER_SEC;
    double estimate=(AHC_REFINEMENT_BUDGET-before_capture)/(4.0*max(per_repetition,1e-6));
    repetitions=8;for(int option:{8,12,16,24,32})if(option<=estimate)repetitions=option;
    M=P*repetitions;
    cerr<<"AdaptiveR="<<repetitions<<" capture_per_repetition="<<per_repetition<<" before="<<before_capture<<"\\n";
   }
  }
 }
 scenarios.resize(M);'''
assert old in s
s=s.replace(old,new)
old='for(int cycle=0;cycle<(argc>2?atoi(argv[2]):1);cycle++)exact_state_refit(A,papers,1,argc>3?atoi(argv[3]):43,32,argc>1?atoi(argv[1]):16,0);'
assert old in s
diagnostic=s.replace(old,'exact_state_refit(A,papers,1,43,32,argc>1?atoi(argv[1]):-1,0);')
(p/'round5_sampling_adaptive_diagnostic.cpp').write_text(diagnostic)
production=s.replace('int main(int argc,char**argv){','int main(){').replace(old,'exact_state_refit(A,papers,1,43,32,-1,0);')
(p/'round5_sampling_adaptive.cpp').write_text(production)

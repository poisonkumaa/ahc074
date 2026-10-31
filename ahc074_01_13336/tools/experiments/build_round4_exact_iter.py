from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'round4_exact_fast.cpp').read_text()
s=s.replace('exact_state_refit(A,papers,passes,bottom,32,repetitions,regularizer);auto future=', 'for(int round=0;round<(argc>6?atoi(argv[6]):1);round++)exact_state_refit(A,papers,passes,bottom,32,repetitions,regularizer);auto future=')
(d/'round4_exact_iter.cpp').write_text(s)

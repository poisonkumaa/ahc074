from pathlib import Path
p=Path(__file__).parent
s=(p.parent.parent/'main.cpp').read_text()
s=s.replace('int main(){', '#include "round7_controlfull.h"\nint main(int argc,char**argv){Round7ControlFull::beta=argc>1?atof(argv[1]):1;int repetitions=argc>2?atoi(argv[2]):8;')
s=s.replace('exact_state_refit(A,papers,1,43,32,-1,0);','exact_state_refit(A,papers,1,43,32,repetitions,0);')
s=s.replace('cout.flush();int fallbacks=0;', 'cout.flush();int fallbacks=0;Round7ControlFull::init(A,papers);')
s=s.replace('auto target=direct_final(order,length,selected,&future);','auto target=Round7ControlFull::final_order(order,length,selected,future);')
s=s.replace('uint64_t bit=1ULL<<(r%64);for(int k=0;k<int(papers.size());k++)future[k]', 'Round7ControlFull::observe(product,r);\n  uint64_t bit=1ULL<<(r%64);for(int k=0;k<int(papers.size());k++)future[k]')
(p/'round7_controlfull.cpp').write_text(s)

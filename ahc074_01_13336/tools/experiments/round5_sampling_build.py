from pathlib import Path
p=Path(__file__).parent
s=(p.parent.parent/'main.cpp').read_text()
s=s.replace('int main(){','int main(int argc,char**argv){')
old='exact_state_refit(A,papers,1,43,32,8,0);'
assert old in s
s=s.replace(old,'for(int cycle=0;cycle<(argc>2?atoi(argv[2]):1);cycle++)exact_state_refit(A,papers,1,argc>3?atoi(argv[3]):43,32,argc>1?atoi(argv[1]):16,0);')
(p/'round5_sampling.cpp').write_text(s)

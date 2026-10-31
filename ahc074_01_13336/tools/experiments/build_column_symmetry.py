from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'main_13906_final.cpp').read_text().replace('int main(){','int main(int argc,char**argv){')
s=s.replace('cols=bestcols;for(int j=1;', 'cols=bestcols;if(argc>1&&atoi(argv[1]))reverse(cols.begin(),cols.end());if(argc>2)rotate(cols.begin(),cols.begin()+atoi(argv[2]),cols.end());for(int j=1;')
(d/'column_symmetry.cpp').write_text(s)

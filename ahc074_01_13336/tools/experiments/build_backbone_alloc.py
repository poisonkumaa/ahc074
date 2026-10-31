from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'structure2_agent_main.cpp').read_text().replace('int main(){','int main(int argc,char**argv){').replace('std::array<int,9>','std::array<int,17>')
s=s.replace('train_backbone_dictionary(A,papers,n,6,1,8);','train_backbone_dictionary(A,papers,n,argc>1?atoi(argv[1]):6,1,8);')
s=s.replace('refit_all_prefix_papers(A,papers,1,8);','refit_all_prefix_papers(A,papers,argc>2?atoi(argv[2]):1,argc>3?atoi(argv[3]):8);')
(d/'backbone_alloc_submit.cpp').write_text(s)

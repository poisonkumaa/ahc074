from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'global_agent_allocate_tune.cpp').read_text()
s=s.replace('int main(int argc,char**argv){','#include "column_joint.h"\nint main(int argc,char**argv){')
s=s.replace('if(papers.size()>size_t(K))return 0;','joint_columns(A,papers,cols,flip,argc>7?atoi(argv[7]):3000,argc>8?atof(argv[8]):2.0);\n if(papers.size()>size_t(K))return 0;')
(d/'column_joint_submit.cpp').write_text(s)

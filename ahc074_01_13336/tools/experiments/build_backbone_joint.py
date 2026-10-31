from pathlib import Path
d=Path(__file__).resolve().parent
h=(d/'column_joint.h').read_text().replace('joint_columns(', 'joint_backbone_columns(')
h=h.replace('std::vector<uint64_t>full(K);','''std::vector<uint64_t>full(K);std::vector<char>fixed(K);
 for(int k=0;k<K;k++)fixed[k]=papers[k].color==PrefixDP::low(papers[k].len-1)||(papers[k].len==64&&papers[k].color==~0ULL);''')
h=h.replace('for(auto&v:full)if(((v>>x)^(v>>y))&1)v^=mask;','for(int k=0;k<K;k++)if(!fixed[k]&&(((full[k]>>x)^(full[k]>>y))&1))full[k]^=mask;')
h=h.replace('swapcols(x,y);int value=evaluate();','''bool invert=rnd()%4==0;
  auto inv=[&](){for(auto&v:targets)v^=1ULL<<x;for(int k=0;k<K;k++)if(!fixed[k])full[k]^=1ULL<<x;flip[x]^=1;};
  if(invert)inv();else swapcols(x,y);int value=evaluate();''')
h=h.replace('}else swapcols(x,y);','}else if(invert)inv();else swapcols(x,y);')
(d/'backbone_joint.h').write_text(h)
s=(d/'backbone_submit.cpp').read_text()
s=s.replace('int main(int argc,char**argv){','#include "backbone_joint.h"\nint main(int argc,char**argv){')
s=s.replace('if(papers.size()>size_t(K))return 0;','joint_backbone_columns(A,papers,cols,flip,argc>7?atoi(argv[7]):10000,argc>8?atof(argv[8]):2.0);\n if(papers.size()>size_t(K))return 0;')
(d/'backbone_joint_submit.cpp').write_text(s)

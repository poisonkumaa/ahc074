from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'global_agent_allocate_tune.cpp').read_text()
s=s.replace('int main(int argc,char**argv){','#include "backbone_dictionary.h"\nint main(int argc,char**argv){')
begin=s.index(' train_prefix_dictionary(A,papers,n,')
end=s.index('\n',begin)
s=s[:begin]+' train_backbone_dictionary(A,papers,n,argc>4?atoi(argv[4]):8,argc>1?atoi(argv[1]):3,argc>2?atoi(argv[2]):8);'+s[end:]
(d/'backbone_submit.cpp').write_text(s)

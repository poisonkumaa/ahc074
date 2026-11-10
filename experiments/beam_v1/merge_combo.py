from pathlib import Path
from difflib import SequenceMatcher
p=Path(__file__).resolve().parent
old=(p/'baseline.cpp').read_text(encoding='utf-8').splitlines(True)
new=(p/'trace_beam.cpp').read_text(encoding='utf-8').splitlines(True)
s=(p/'fast_post_beam.cpp').read_text(encoding='utf-8')
for tag,i,j,a,b in SequenceMatcher(None,old,new,autojunk=False).get_opcodes():
    if tag=='equal':continue
    before,after=''.join(old[i:j]),''.join(new[a:b])
    if not before:
        before=''.join(old[i-2:i+2])
        after=''.join(old[i-2:i])+after+''.join(old[i:i+2])
    assert s.count(before)==1,(tag,i,j,s.count(before))
    s=s.replace(before,after,1)
(p/'combo.cpp').write_text(s,encoding='utf-8')

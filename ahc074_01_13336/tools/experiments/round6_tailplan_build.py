from pathlib import Path
p=Path(__file__).parent
s=(p.parent.parent/'main.cpp').read_text()
s=s[:s.index('int main(){')]
s=s.replace('  int h=active[n][0],j=n;', '  if(count[n]==0){best=1000000;return {};}int h=active[n][0],j=n;')
assert 'if(count[n]==0)' in s
s+='\n#include "round6_tailplan.h"\n'
(p/'round6_tailplan.cpp').write_text(s)

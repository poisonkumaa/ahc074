from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'pair_snapshot_append.cpp').read_text(encoding='utf-8')
needle='#include "pair_snapshot_rematch.inc"'
assert s.count(needle)==1
s=s.replace(needle,(p/'pair_snapshot_rematch.inc').read_text(encoding='utf-8'))
needle='search_budget::integer("PAIR_SNAPSHOT",0)'
assert s.count(needle)==1
s=s.replace(needle,'search_budget::integer("PAIR_SNAPSHOT",1)')
s='// Retain the original layout candidates and add one alternate product matching\n// on an existing core map when the construction budget has room.\n'+s
(p/'main_candidate.cpp').write_text(s,encoding='utf-8')

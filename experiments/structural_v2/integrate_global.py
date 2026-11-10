from pathlib import Path

root = Path(__file__).resolve().parent
s = (root / 'joint.cpp').read_text(encoding='utf-8')
s = s.replace('#include "joint_core_search.inc"',
              '#include "global_pair_repair.inc"\n#include "joint_core_global.inc"', 1)
(root / 'global.cpp').write_text(s, encoding='utf-8')

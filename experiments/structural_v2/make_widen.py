from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'global.cpp').read_text(encoding='utf-8')
s=s.replace('#include "joint_core_global.inc"','#include "core_widen.inc"',1)
s=s.replace('if(search_budget::integer("CORE_JOINT",0)) {','if(search_budget::integer("CORE_WIDEN",0)) {',1)
s=s.replace('optimize_joint_core(chosen,absolute_deadline(search_budget::real("CORE_JOINT_END",2.30)));','optimize_wider_core(chosen,absolute_deadline(search_budget::real("CORE_WIDEN_END",2.30)));',1)
(p/'widen.cpp').write_text(s,encoding='utf-8')

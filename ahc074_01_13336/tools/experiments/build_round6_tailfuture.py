from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'round6_tailblock.cpp').read_text().replace('#include "structure2_agent_general_move.h"','#include "round6_general_future.h"')
s=s.replace('GeneralPrefix::best_move(order,length,selected,vis,length,.5)','GeneralPrefixFuture::best_move(order,length,selected,vis,length,.5,&future)')
(d/'round6_tailfuture.cpp').write_text(s)

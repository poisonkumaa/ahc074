from pathlib import Path
p=Path(__file__).parent
s=(p.parent.parent/'main.cpp').read_text()
import re
s=re.sub(r'#define STRUCTURE2_TRAINING_CPU_LIMIT [\d.]+','#define STRUCTURE2_TRAINING_CPU_LIMIT 100.0',s)
s=s.replace('int main(){','#include "window_order_agent.h"\nint main(int argc,char**argv){')
s=s.replace('cols=bestcols;for(int j=1;j<n;j++)','cols=bestcols;optimize_window_columns(A,cols,correlation,argc>1?atoi(argv[1]):15000,argc>2?atoi(argv[2]):0,argc>3?atof(argv[3]):0,argc>4?atoi(argv[4]):8,argc>5?atof(argv[5]):8);for(int j=1;j<n;j++)')
(p/'window_order_agent.cpp').write_text(s)

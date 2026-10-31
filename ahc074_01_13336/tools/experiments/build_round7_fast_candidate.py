from pathlib import Path
d=Path(__file__).resolve().parent
for mode in ('maskdp','maskpop'):
 s=(d/f'round7_{mode}.cpp').read_text()
 s=s.replace('inline std::vector<int> final_order_incremental(', 'inline std::vector<int> final_order_incremental_reference(',1)
 at=s.index('inline vector<int> direct_final(')
 s=s[:at]+(d/'round7_final_fast.h').read_text()+'\n'+s[at:]
 (d/f'round7_fast_{mode}.cpp').write_text(s)

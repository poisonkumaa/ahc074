from pathlib import Path
root=Path(__file__).resolve().parents[2]
text=(root/'tools/experiments/round4_exact_fast.cpp').read_text()
text=text.replace('int amount=std::min(perlevel,int(extra[l].size()+spare.size()));',
                  'int requested=l>=57?8:4;int amount=std::min(requested,int(extra[l].size()+spare.size()));')
text=text.replace('std::array<int,9>dp,split','std::array<int,17>dp,split')
(root/'tools/experiments/round4_design_exact.cpp').write_text(text)

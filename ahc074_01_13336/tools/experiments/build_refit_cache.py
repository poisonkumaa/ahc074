from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'main_13906_final.cpp').read_text()
a=s.index('inline void refit_all_prefix_papers(');b=s.index('\nint main()',a)
p=s[a:b]
p=p.replace('double initial=distances();int accepted=0,rejected=0;', 'double initial=distances();int accepted=0,rejected=0;bool dirty=false;')
p=p.replace('if(!amount)continue;distances();', 'if(!amount)continue;if(dirty){distances();dirty=false;}')
p=p.replace('papers[k].color=pattern[q];update(k);', 'if(papers[k].color!=pattern[q]){papers[k].color=pattern[q];update(k);dirty=true;}')
s=s[:a]+p+s[b:]
(d/'refit_cache.cpp').write_text(s)

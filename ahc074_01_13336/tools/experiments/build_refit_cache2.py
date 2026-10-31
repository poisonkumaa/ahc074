from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'refit_cache.cpp').read_text()
a=s.index('inline void refit_all_prefix_papers(');b=s.index('\nint main()',a)
p=s[a:b]
p=p.replace('bool dirty=false;', 'bool dirty=false;int generation=0;std::array<int,65>processed;processed.fill(-1);')
p=p.replace('int amount=level[l].size();if(!amount)continue;', 'int amount=level[l].size();if(!amount||processed[l]==generation)continue;bool changed=false;')
p=p.replace('update(k);dirty=true;', 'update(k);dirty=true;changed=true;')
p=p.replace('}accepted++;','}accepted++;if(changed)generation++;processed[l]=generation;')
assert 'if(changed)generation++' in p
s=s[:a]+p+s[b:]
(d/'refit_cache2.cpp').write_text(s)

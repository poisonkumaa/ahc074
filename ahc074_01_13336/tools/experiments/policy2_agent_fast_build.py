from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy2_agent_union_adapt.cpp').read_text()
start=s.index('uint64_t unions[65][193];')
end=s.index('    double initial_quality=0;',start)
s=s[:start]+'''uint64_t below[192]{}, mask_tree[65]{};
    if(access){for(int i=K-1;i>=0;i--){int k=order[i];for(int x=length[k];x>0;x-=x&-x)below[k]|=mask_tree[x];for(int x=length[k];x<=N;x+=x&-x)mask_tree[x]|=(*access)[k];}}
''' +s[end:]
s=s.replace('__builtin_popcountll(unions[length[k]][i+1])','__builtin_popcountll(below[k])')
s=s.replace('int direction=initial_moved[k]||!selected[k]?1:-1;', '''int direction=initial_moved[k]||!selected[k]?1:-1;
        uint64_t remaining[193];if(access&&direction==1){remaining[K]=0;for(int d=K-1;d>i;d--){int x=order[d];remaining[d]=remaining[d+1]|(length[x]<=kl?(*access)[x]:0ULL);}}''')
s=s.replace('own=unions[kl][i+1]','own=below[k]').replace('unions[kl][d+1]','remaining[d+1]').replace('unions[xl][d+1]','below[x]')
(p/'policy2_agent_union_fast.cpp').write_text(s)

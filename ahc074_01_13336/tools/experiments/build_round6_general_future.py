from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'structure2_agent_general_move.h').read_text()
s=s.replace('namespace GeneralPrefix {','namespace GeneralPrefixFuture {').replace('#include <vector>','#include <vector>\n#include <cstdint>\n#include <iostream>')
s=s.replace('double decay=.5) {','double decay=.5, const std::vector<uint64_t>* access=nullptr) {')
s=s.replace('    double initial_quality=0;', '''    uint64_t below[192]{},mask_tree[65]{};
    if(access)for(int i=K-1;i>=0;i--){int k=order[i];for(int x=quality_length[k];x>0;x-=x&-x)below[k]|=mask_tree[x];for(int x=quality_length[k];x<=64;x+=x&-x)mask_tree[x]|=(*access)[k];}
    double initial_quality=0;''')
s=s.replace('initial_quality+=1-power[inversions[k]];','initial_quality+=access?double(__builtin_popcountll(below[k])):1-power[inversions[k]];')
s=s.replace('            moved=initial_moved;before=initial_before;bad=initial_bad;', '''            uint64_t remaining[193]{},without[192]{};
            if(access&&direction==1)for(int d=K-1;d>i;d--){int x=order[d];remaining[d]=remaining[d+1]|(quality_length[x]<=quality_length[k]?(*access)[x]:0ULL);}
            if(access&&direction==-1){uint64_t tree[65]{};for(int j=K-1;j>=0;j--){int x=order[j];if(x==k)continue;for(int h=quality_length[x];h>0;h-=h&-h)without[x]|=tree[h];for(int h=quality_length[x];h<=64;h+=h&-h)tree[h]|=(*access)[x];}}
            uint64_t own=below[k];int own_count=__builtin_popcountll(own);
            moved=initial_moved;before=initial_before;bad=initial_bad;''')
s=s.replace('quality+=power[inversions[x]]-power[inversions[x]+1];invsum++;','if(!access)quality+=power[inversions[x]]-power[inversions[x]+1];invsum++;')
s=s.replace('quality-=power[inversions[x]-1]-power[inversions[x]];invsum--;','if(!access)quality-=power[inversions[x]-1]-power[inversions[x]];invsum--;')
s=s.replace('                quality+=power[old_invk]-power[invk];','''                if(access){
                    if(direction==1){int c=__builtin_popcountll(remaining[d+1]);quality+=c-own_count;own_count=c;if(quality_length[x]>=quality_length[k])quality+=__builtin_popcountll((*access)[k]&~below[x]);}
                    else{if(quality_length[x]<=quality_length[k])own|=(*access)[x];int c=__builtin_popcountll(own);quality+=c-own_count;own_count=c;if(quality_length[x]>=quality_length[k])quality-=__builtin_popcountll((*access)[k]&~without[x]);}
                }else quality+=power[old_invk]-power[invk];''')
s=s.rsplit('    return answer;',1)[0]+'''#ifdef GENERAL_FUTURE_CHECK
    if(answer.first>=0&&access){auto q=order;int k=q[answer.first];q.erase(q.begin()+answer.first);q.insert(q.begin()+answer.second,k);int value=0;for(int i=0;i<K;i++){uint64_t mask=0;for(int j=i+1;j<K;j++)if(quality_length[q[j]]<=quality_length[q[i]])mask|=(*access)[q[j]];value+=__builtin_popcountll(mask);}if(value!=best_quality){std::cerr<<"QUALITY BUG "<<value<<" "<<best_quality<<"\\n";abort();}}
#endif
    return answer;'''+s.rsplit('    return answer;',1)[1]
(d/'round6_general_future.h').write_text(s)

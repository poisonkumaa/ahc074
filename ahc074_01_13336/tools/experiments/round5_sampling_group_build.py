from pathlib import Path
p=Path(__file__).parent
for name in ['compact','profile']:
 s=(p/f'round5_sampling_{name}.cpp').read_text()
 old='''    if(l<N){B[l-bottom].fill(INF);for(int z=0;z<K;z++)if(length[z]>l&&start[v][z]<=l){int len=length[z],q=s.pos[z];for(int h=0;h<=K;h++){int val=add(l,h,z)+B[len-bottom][max(q+1,h)];if(val<B[l-bottom][h])B[l-bottom][h]=val;}}}'''
 new='''    if(l<N){auto&out=B[l-bottom];out.fill(INF);for(int len=l+1;len<=N;len++){
     int at[193];fill(at,at+K+1,INF*2);int first=K+1;bool any=false;
     for(int z:level[len])if(start[v][z]<=l){int q=s.pos[z];first=min(first,q);any=true;at[q]=s.cnt[len][q]+B[len-bottom][q+1];}
     if(!any)continue;int same=INF*2,far=INF*2,last=-1;
     for(int h=K;h>=0;h--){int d=s.cnt[l+1][h]-s.cnt[len][h];if(last!=d){far=min(far,same);same=INF*2;last=d;}same=min(same,at[h]);int val=min(same,far+1)-s.cnt[len][h];if(first<h)val=min(val,int(B[len-bottom][h]));if(val<out[h])out[h]=val;}
    }
#ifdef SAMPLING_CHECK_B
    array<int,193>reference;reference.fill(INF);for(int z=0;z<K;z++)if(length[z]>l&&start[v][z]<=l){int len=length[z],q=s.pos[z];for(int h=0;h<=K;h++)reference[h]=min(reference[h],add(l,h,z)+B[len-bottom][max(q+1,h)]);}for(int h=0;h<=K;h++)assert(reference[h]==out[h]);
#endif
    }'''
 assert old in s
 s=s.replace(old,new)
 (p/f'round5_sampling_{name}_group.cpp').write_text(s)

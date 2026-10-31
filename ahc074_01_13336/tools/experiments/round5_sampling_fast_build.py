from pathlib import Path
p=Path(__file__).parent
for name in ['compact','profile']:
 s=(p/f'round5_sampling_{name}_group.cpp').read_text()
 s=s.replace('vector<array<uint8_t,193>>B;', 'vector<array<uint8_t,193>>B;array<uint8_t,65>bypass;')
 s=s.replace('t.B[N-bottom].fill(0);','t.B[N-bottom].fill(0);t.bypass.fill(INF);')
 s=s.replace('vector<array<uint8_t,65>>route(M*Q);','vector<array<uint8_t,65>>route(M*Q),raw(M*Q);')
 old='''    for(int z=0;z<K;z++)if(length[z]>l&&start[v][z]<l){int len=length[z],q=s.pos[z];for(int j=start[v][z];j<l;j++)for(int fi=t.offset[j];fi<t.offset[j+1];fi++){int packed=t.F[fi],h=packed>>8,f=packed&255;bypass[e]=min(bypass[e],f+add(j,h,z)+B[len-bottom][max(q+1,h)]);}}'''
 assert old in s
 s=s.replace(old,'''    bypass[e]=t.bypass[l];
#ifdef SAMPLING_CHECK_B
    int reference_bypass=INF;
'''+old.replace('bypass[e]=min(bypass[e],','reference_bypass=min(reference_bypass,')+'''
    assert(reference_bypass==bypass[e]);
#endif''')
 old='''     for(int j=l-1;j>=0;j--){int value=rr[j+1];for(int fi=t.offset[j];fi<t.offset[j+1];fi++){int packed=t.F[fi],h=packed>>8,f=packed&255;value=min(value,f+add(j,h,z)+B[l-bottom][max(pos+1,h)]);}rr[j]=value;}'''
 new='''     for(int j=l-1;j>=0;j--){int value=INF;for(int fi=t.offset[j];fi<t.offset[j+1];fi++){int packed=t.F[fi],h=packed>>8,f=packed&255;value=min(value,f+add(j,h,z)+B[l-bottom][max(pos+1,h)]);}raw[e*Q+q][j]=value;rr[j]=min(value,int(rr[j+1]));}'''
 assert old in s
 s=s.replace(old,new)
 old='''  }
 }
 }
 cerr<<"exact_refit accepted="'''
 new='''  }
  for(int e=0;e<M;e++){int v=scenarios[e].target;auto&t=*tables[e];for(int q=0;q<Q;q++){int z=level[l][q],best=INF;for(int j=start[v][z];j+1<l;j++){best=min(best,int(raw[e*Q+q][j]));if(best<t.bypass[j+1])t.bypass[j+1]=best;}}}
 }
 }
 cerr<<"exact_refit accepted="'''
 assert old in s
 s=s.replace(old,new)
 (p/f'round5_sampling_{name}_fast.cpp').write_text(s)

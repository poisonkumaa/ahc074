from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy4_agent_direct_reachable.cpp').read_text()
s=s.replace('order=a;uint64_t bit=', '''for(int i=0;i<K;i++)for(int j=i+1;j<K;j++)if(length[a[i]]<length[a[j]])for(int h=j+1;h<K;h++)if(length[a[h]]<length[a[i]]){cerr<<"231 pattern test="<<test<<" r="<<r<<" values="<<length[a[i]]<<","<<length[a[j]]<<","<<length[a[h]]<<"\\n";return 4;}
  order=a;uint64_t bit=''')
(p/'policy4_agent_direct_invariant.cpp').write_text(s)

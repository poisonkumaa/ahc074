from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy2_agent_union_fast.cpp').read_text()
s=s.replace('namespace GlobalPrefix {','namespace GlobalPrefix {\nstatic int future_samples=8, move_beam=4;static int future_next[64][192], future_bit[64];\n')
s=s.replace('std::pair<int,int> answer{-1,-1};double best_quality=', 'struct Choice{int from,to;double q;int inv;};std::vector<Choice> candidates;\n    std::pair<int,int> answer{-1,-1};double best_quality=')
s=s.replace('int direction=initial_moved[k]||!selected[k]?1:-1;', 'Choice local{i,-1,std::numeric_limits<double>::infinity(),INT_MAX};\n        int direction=initial_moved[k]||!selected[k]?1:-1;')
s=s.replace('if(quality<best_quality-1e-10 ||', '''if(quality<local.q-1e-10||(std::abs(quality-local.q)<=1e-10&&invsum<local.inv))local={i,d,quality,invsum};
                if(quality<best_quality-1e-10 ||''')
s=s.replace('''        }
    }
    return answer;
}
}''', '''        }
        if(local.to>=0)candidates.push_back(local);
    }
    if(access&&move_beam>1){
     std::sort(candidates.begin(),candidates.end(),[](auto a,auto b){return a.q==b.q?a.inv<b.inv:a.q<b.q;});
     double best=1e30;int best_inv=INT_MAX;
     for(int ci=0;ci<std::min(move_beam,int(candidates.size()));ci++){
      auto c=candidates[ci];auto p=order;int movedpaper=p[c.from];p.erase(p.begin()+c.from);p.insert(p.begin()+c.to,movedpaper);int pos[192];for(int i=0;i<K;i++)pos[p[i]]=i;
      int extras=0;
      for(int f=0;f<future_samples;f++){
       uint64_t bit=1ULL<<future_bit[f];bool moved[192]{},bad[192]{};int minimum=65;
       for(int i=K-1;i>=0;i--){int k=p[i];moved[k]=length[k]>=minimum;if((*access)[k]&bit)minimum=std::min(minimum,length[k]);}
       for(int k=0;k<K;k++)if(!((*access)[k]&bit)&&!moved[k]&&pos[k]<pos[future_next[f][k]])bad[future_next[f][k]]=true;
       for(int k=0;k<K;k++)if(((*access)[k]&bit)&&!moved[k]&&bad[k])extras++;
      }
      double score=c.q+64.*extras/future_samples;
      if(score<best-1e-10||(std::abs(score-best)<1e-10&&c.inv<best_inv)){best=score;best_inv=c.inv;answer={c.from,c.to};}
     }
    }
    return answer;
}
}''',1)
s=s.replace(' for(int r=0;r<R;r++){',' GlobalPrefix::future_samples=argc>1?atoi(argv[1]):8;GlobalPrefix::move_beam=argc>2?atoi(argv[2]):4;\n for(int r=0;r<R;r++){')
s=s.replace('  int moves;auto chosen=dp.solve(order,A[product],moves);', '''  for(int f=0;f<GlobalPrefix::future_samples;f++){int b=GlobalPrefix::future_bit[f]=f*64/GlobalPrefix::future_samples;uint64_t bit=1ULL<<b;int owner[65],root=-1;fill(owner,owner+65,-1);for(int k=0;k<int(papers.size());k++)if(future[k]&bit)owner[length[k]]=k;for(int l=64;l>=0;l--){if(owner[l]>=0)root=owner[l];owner[l]=root;}for(int k=0;k<int(papers.size());k++)GlobalPrefix::future_next[f][k]=owner[length[k]];}
  int moves;auto chosen=dp.solve(order,A[product],moves);''')
s=s.replace('{int period=argc>1?atoi(argv[1]):1;', '{int period=1;')
(p/'policy2_agent_optional.cpp').write_text(s)

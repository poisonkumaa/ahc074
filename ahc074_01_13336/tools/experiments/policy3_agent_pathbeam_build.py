from pathlib import Path
p=Path(__file__).resolve().parent
s=(p.parents[1]/'main.cpp').read_text();(p/'policy3_agent_base.cpp').write_text(s)
s=s.replace('int main(){','int main(int argc,char**argv){')
s=s.replace('namespace GlobalPrefix {','namespace GlobalPrefix {\nstruct MoveChoice {int from,to;double quality;int inversions,sequence;};\n')
s=s.replace('const std::vector<uint64_t>* access=nullptr) {','const std::vector<uint64_t>* access=nullptr, std::vector<MoveChoice>* choices=nullptr, int limit=8) {')
s=s.replace('if(distance!=initial_distance-1)continue;', 'if(distance!=initial_distance-1)continue;\n                if(choices)choices->push_back({i,d,quality,invsum,int(choices->size())});')
a=s.index('inline std::pair<int,int> best_move(');b=s.index('\n}\n}',a)
t=s[a:b]
t=t.replace('    return answer;', '''    if(choices){auto less=[](auto a,auto b){if(a.quality!=b.quality)return a.quality<b.quality;if(a.inversions!=b.inversions)return a.inversions<b.inversions;return a.sequence<b.sequence;};int take=std::min(limit,int(choices->size()));std::partial_sort(choices->begin(),choices->begin()+take,choices->end(),less);choices->resize(take);}
    return answer;''')
s=s[:a]+t+s[b:]
start=s.index('  vector<pair<int,int>>operations;operations.reserve(moves);')
end=s.index('  {int period=1;',start)
s=s[:start]+'''  vector<pair<int,int>>operations;
  int width=argc>1?atoi(argv[1]):4,branch=argc>2?atoi(argv[2]):8;
  if(double(clock()-structure2_training_origin)/CLOCKS_PER_SEC>1.35)width=1;
  if(width<=1){operations.reserve(moves);for(int i=0;i<moves;i++){auto[from,to]=GlobalPrefix::best_move(order,length,selected,.5,&future);if(from<0)return 0;int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);operations.push_back({k,to});}}
  else{
   struct State{vector<int>order;vector<pair<int,int>>ops;double quality=0;int inv=0;bool baseline=false;uint64_t hash=0;};
   vector<State> beam(1);beam[0].order=order;beam[0].baseline=true;
   for(int depth=0;depth<moves;depth++){
    vector<State>next;next.reserve(width*branch);vector<GlobalPrefix::MoveChoice>choices;choices.reserve(4096);
    for(const auto&state:beam){choices.clear();GlobalPrefix::best_move(state.order,length,selected,.5,&future,&choices,branch);
     for(int ci=0;ci<int(choices.size());ci++){const auto&c=choices[ci];State child=state;int k=child.order[c.from];child.order.erase(child.order.begin()+c.from);child.order.insert(child.order.begin()+c.to,k);child.ops.push_back({k,c.to});child.quality=c.quality;child.inv=c.inversions;child.baseline=state.baseline&&ci==0;uint64_t hash=0x125713aULL;for(int z:child.order)hash=(hash^uint64_t(z+1))*0x9e3779b97f4a7c15ULL;child.hash=hash;
      bool duplicate=false;for(auto&prev:next)if(prev.hash==hash&&prev.order==child.order){prev.baseline|=child.baseline;duplicate=true;break;}if(!duplicate)next.push_back(move(child));
     }
    }
    stable_sort(next.begin(),next.end(),[](const auto&a,const auto&b){return a.quality==b.quality?a.inv<b.inv:a.quality<b.quality;});
    if(int(next.size())>width){int base=-1;for(int i=0;i<int(next.size());i++)if(next[i].baseline){base=i;break;}if(base>=width)next[width-1]=move(next[base]);next.resize(width);}
    beam=move(next);
   }
   if(moves){int at=0;for(int i=1;i<int(beam.size());i++)if(beam[i].quality<beam[at].quality||(beam[i].quality==beam[at].quality&&beam[i].inv<beam[at].inv))at=i;order=move(beam[at].order);operations=move(beam[at].ops);}
  }
''' +s[end:]
(p/'policy3_agent_pathbeam.cpp').write_text(s)

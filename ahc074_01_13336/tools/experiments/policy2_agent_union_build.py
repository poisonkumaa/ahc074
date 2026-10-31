from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'structure2_agent_main.cpp').read_text()
s=s.replace('int main(){','int main(int argc,char**argv){')
s=s.replace('double decay=.5) {','double decay=.5, const std::vector<std::array<uint64_t,4>>* access=nullptr, int words=4) {')
s=s.replace('double initial_quality=0;', '''uint64_t unions[65][193][4];
    if(access){
      for(int l=1;l<=N;l++){for(int w=0;w<words;w++)unions[l][K][w]=0;
       for(int i=K-1;i>=0;i--){int k=order[i];for(int w=0;w<words;w++)unions[l][i][w]=unions[l][i+1][w]|(length[k]<=l?(*access)[k][w]:0ULL);}
      }
    }
    double initial_quality=0;''')
s=s.replace('initial_quality+=1-power[inversions[k]];', '''if(access){for(int w=0;w<words;w++)initial_quality+=__builtin_popcountll(unions[length[k]][i+1][w]);}
        else initial_quality+=1-power[inversions[k]];''')
s=s.replace('int following_min=suffix[i+1];double quality=initial_quality;', '''int following_min=suffix[i+1];double quality=initial_quality;uint64_t own[4]{};int own_count=0;
            if(access)for(int w=0;w<words;w++){own[w]=unions[kl][i+1][w];own_count+=__builtin_popcountll(own[w]);}''')
s=s.replace('if(xl<kl){invk--;invsum--;}\n                    else if(xl>kl){quality+=power[inversions[x]]-power[inversions[x]+1];invsum++;}', '''if(xl<kl){invk--;invsum--;}else if(xl>kl){if(!access)quality+=power[inversions[x]]-power[inversions[x]+1];invsum++;}
                    if(access){
                     int next_count=0;for(int w=0;w<words;w++)next_count+=__builtin_popcountll(unions[kl][d+1][w]);quality+=next_count-own_count;own_count=next_count;
                     if(xl>=kl)for(int w=0;w<words;w++)quality+=__builtin_popcountll((*access)[k][w]&~unions[xl][d+1][w]);
                    }''')
s=s.replace('if(xl<kl){invk++;invsum++;}\n                    else if(xl>kl){quality-=power[inversions[x]-1]-power[inversions[x]];invsum--;}', '''if(xl<kl){invk++;invsum++;}else if(xl>kl){if(!access)quality-=power[inversions[x]-1]-power[inversions[x]];invsum--;}
                    if(access){
                     int next_count=0;for(int w=0;w<words;w++){own[w]|=(*access)[x][w];next_count+=__builtin_popcountll(own[w]);}quality+=next_count-own_count;own_count=next_count;
                    }''')
s=s.replace('quality+=power[old_invk]-power[invk];','if(!access)quality+=power[old_invk]-power[invk];')
start=s.index(' for(int k=0;k<K;k++){\n  string pattern')
end=s.index(' for(int r=0;r<R;r++){',start)
design=s[start:end];design=design[:design.index(' PrefixDP dp(n,papers);')]
setup=''' PrefixDP dp(n,papers);vector<int>order(papers.size()),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);
 int samples=argc>1?atoi(argv[1]):128,words=(samples+63)/64;vector<array<uint64_t,4>> future(papers.size());mt19937 freq_rng(912367);
 vector<int>train_targets(P);iota(train_targets.begin(),train_targets.end(),0);shuffle(train_targets.begin(),train_targets.end(),freq_rng);
 for(int r=0;r<128+samples;r++){
  int target=r<128?freq_rng()%P:train_targets[r-128],moves;auto chosen=dp.solve(order,A[target],moves);vector<char>selected(papers.size());for(int k:chosen){selected[k]=true;if(r>=128)future[k][(r-128)/64]|=1ULL<<((r-128)%64);}
  for(int t=0;t<moves;t++){auto[from,to]=GlobalPrefix::best_move(order,length,selected);if(from<0)abort();int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);}
 }
 cerr<<"PolicyCPU="<<double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<<endl;
 iota(order.begin(),order.end(),0);
'''
s=s[:start]+setup+design+s[end:]
s=s.replace('GlobalPrefix::best_move(order,length,selected);if(from<0)return 0;', 'GlobalPrefix::best_move(order,length,selected,.5,&future,words);if(from<0)return 0;')
(p/'policy2_agent_union.cpp').write_text(s)

from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy3_agent_base.cpp').read_text().replace('int main(){','int main(int argc,char**argv){')
t=(p/'policy2_agent_goal_sparse.cpp').read_text();a=s.index('struct PrefixDP {');b=s.index('\n};',a)+3;c=t.index('struct PrefixDP {');d=t.index('\n};',c)+3
s=s[:a]+t[c:d]+s[b:]
s=s.replace(' PrefixDP dp(n,papers);vector<int>order',' PrefixDP dp(n,papers);dp.public_data(A);vector<int>order')
a=s.index('int main(')
s=s[:a]+'''struct FreeChoice{int from,to,quality,inv;};
vector<FreeChoice> free_moves(const vector<int>&order,const vector<int>&length,const vector<uint64_t>&future){
 int K=order.size();uint64_t tree[65]{},below[192]{};int inversions[192]{},itree[65]{},q0=0,inv0=0;
 for(int i=K-1;i>=0;i--){int k=order[i];for(int x=length[k];x>0;x-=x&-x)below[k]|=tree[x];for(int x=length[k];x<=64;x+=x&-x)tree[x]|=future[k];for(int x=length[k]-1;x>0;x-=x&-x)inversions[k]+=itree[x];for(int x=length[k];x<=64;x+=x&-x)itree[x]++;q0+=__builtin_popcountll(below[k]);inv0+=inversions[k];}
 vector<FreeChoice>out;out.reserve(K*30);int high=0;
 for(int i=0;i<K;i++){
  int k=order[i],kl=length[k];if(kl<=high)continue;high=kl;uint64_t remaining[193];remaining[K]=0;for(int d=K-1;d>i;d--){int x=order[d];remaining[d]=remaining[d+1]|(length[x]<=kl?future[x]:0ULL);}
  int q=q0,inv=inv0,own=__builtin_popcountll(below[k]);
  for(int d=i+1;d<K;d++){int x=order[d],xl=length[x],next=__builtin_popcountll(remaining[d+1]);q+=next-own;own=next;if(xl>=kl)q+=__builtin_popcountll(future[k]&~below[x]);inv+=(xl>kl)-(xl<kl);out.push_back({i,d,q,inv});}
  q=q0;inv=inv0;uint64_t u=below[k];own=__builtin_popcountll(u);
  for(int d=i-1;d>=0;d--){int x=order[d];u|=future[x];int next=__builtin_popcountll(u);q+=next-own;own=next;inv++;out.push_back({i,d,q,inv});}
 }
 sort(out.begin(),out.end(),[](auto a,auto b){return a.quality==b.quality?a.inv<b.inv:a.quality<b.quality;});return out;
}
''' + s[a:]
s=s.replace(' for(int r=0;r<R;r++){',' long long tested=0,improved=0;\n for(int r=0;r<R;r++){')
start=s.index('  vector<pair<int,int>>operations;operations.reserve(moves);')
end=s.index('  {int period=1;',start)
s=s[:start]+'''  vector<pair<int,int>>operations;operations.reserve(moves);bool search=double(clock()-structure2_training_origin)/CLOCKS_PER_SEC<1.35;int budget=argc>1?atoi(argv[1]):8;
  for(int i=0;i<moves;i++){
   auto[from,to]=GlobalPrefix::best_move(order,length,selected,.5,&future);if(from<0)return 0;
   if(search&&moves-i>1){auto candidates=free_moves(order,length,future);int baseq=INT_MAX,baseinv=INT_MAX;for(auto c:candidates)if(c.from==from&&c.to==to){baseq=c.quality;baseinv=c.inv;break;}
    int attempts=0;int perpaper[192]{};
    for(auto c:candidates){if(c.quality>baseq||(c.quality==baseq&&c.inv>=baseinv))break;int k=order[c.from];if(perpaper[k]++>=2)continue;if(attempts++>=budget)break;tested++;
     auto next=order;next.erase(next.begin()+c.from);next.insert(next.begin()+c.to,k);int distance;dp.solve(next,A[product],distance,false);if(distance!=moves-i-1)continue;
     auto goal=dp.solve(next,A[product],distance);fill(selected.begin(),selected.end(),false);for(int z:goal)selected[z]=true;from=c.from;to=c.to;improved++;break;
    }
   }
   int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);operations.push_back({k,to});
  }
''' + s[end:]
s=s.replace(' cerr<<"FinishCPU="',' cerr<<"Tested="<<tested<<" Improved="<<improved<<"\\n";\n cerr<<"FinishCPU="')
(p/'policy3_agent_freegoal.cpp').write_text(s)

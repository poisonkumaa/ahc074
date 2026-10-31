// Fast exact specialization for ordered prefix-stack insertion.
// The caller retains final_order_incremental_reference as the general fallback.
namespace GlobalPrefix {
static long long r7_final_calls=0,r7_final_fallbacks=0,r7_final_inserts=0;
inline std::vector<int> final_order_incremental_fast(const std::vector<int>&order,const std::vector<int>&length,const std::vector<char>&selected,const std::vector<uint64_t>*future=nullptr){
 using namespace std;
 r7_final_calls++;
 auto fallback=[&](){r7_final_fallbacks++;return final_order_incremental_reference(order,length,selected,future);};
 int K=order.size(),N=*max_element(length.begin(),length.end());
 // One selected paper per length in each sampled goal makes equal-length
 // access events disjoint. Check this assumption before using frequency sums.
 int frequency[192]{};uint64_t used[65]{};
 if(future)for(int k=0;k<K;k++){
  uint64_t bits=(*future)[k];if(used[length[k]]&bits)return fallback();
  used[length[k]]|=bits;frequency[k]=__builtin_popcountll(bits);
 }
 int owner[65],root=-1;fill(owner,owner+65,-1);
 for(int k=0;k<K;k++)if(selected[k])owner[length[k]]=k;
 for(int l=N;l>=0;l--){if(owner[l]>=0)root=owner[l];owner[l]=root;}
 bool mandatory[192]{};int suffix=N+1;
 for(int i=K-1;i>=0;i--){int k=order[i];mandatory[k]=length[k]>=suffix;if(selected[k])suffix=min(suffix,length[k]);}
 vector<vector<int>>children(K);vector<int>insert;
 for(int k:order)if(!selected[k]){if(mandatory[k])insert.push_back(k);else children[owner[length[k]]].push_back(k);}
 stable_sort(insert.begin(),insert.end(),[&](int a,int b){return length[a]<length[b];});
 for(int k:insert){
  r7_final_inserts++;auto&v=children[owner[length[k]]];int M=v.size(),l=length[k],lo=0,hi=M;
  while(lo<M&&length[v[lo]]<l)lo++;
  hi=lo;while(hi<M&&length[v[hi]]==l)hi++;
  // A strict threshold partition is sufficient for exactness. Other stacks
  // remain valid inputs and use the reference implementation for the whole call.
  for(int i=hi;i<M;i++)if(length[v[i]]<=l)return fallback();
  int at=lo,best=0,cost=0;
  if(future)for(int i=lo;i<hi;i++){
   cost+=frequency[k]-frequency[v[i]];
   if(cost<best){best=cost;at=i+1;}
  }
  v.insert(v.begin()+at,k);
 }
 vector<int>out;out.reserve(K);
 for(int l=1;l<=N;l++){int k=owner[l];if(k>=0&&selected[k]&&length[k]==l){out.push_back(k);for(int x:children[k])out.push_back(x);}}
 return out;
}
inline std::vector<int> final_order_incremental(const std::vector<int>&order,const std::vector<int>&length,const std::vector<char>&selected,const std::vector<uint64_t>*future=nullptr){
 auto result=final_order_incremental_fast(order,length,selected,future);
#ifdef R7_FINAL_CHECK
 auto reference=final_order_incremental_reference(order,length,selected,future);
 if(result!=reference){std::cerr<<"R7 final mismatch call="<<r7_final_calls<<"\n";std::abort();}
#endif
 return result;
}
}

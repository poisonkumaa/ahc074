inline bool visible_cover_plan(vector<int>&order,const vector<U64>&mask,const vector<U64>&color,U64 target,double exponent,vector<pair<int,int>>&operations){
 int K=order.size(),fixed=0;U64 covered=0;double weight[65];for(int gain=0;gain<=64;gain++)weight[gain]=pow(double(gain),exponent);operations.clear();
 auto apply=[&](int from,int to){int k=order[from];U64 previous=0;for(int i=0;i<from;i++)previous|=mask[order[i]];assert(mask[k]&~previous);order.erase(order.begin()+from);order.insert(order.begin()+to,k);operations.push_back({k,to});};
 while(fixed<K){
  while(fixed<K){int k=order[fixed];if((color[k]^target)&mask[k]&~covered)break;covered|=mask[k];fixed++;}
  if(fixed==K)return true;
  int counts[64]{},chosen=-1,witness=-1;double best=-1;int bestgain=-1;
  for(int i=fixed;i<K;i++){int k=order[i];U64 unseen=mask[k]&~covered;
   if(unseen&&!((color[k]^target)&unseen)){
    int cost=K+1,bit=-1;U64 rest=unseen;while(rest){int b=__builtin_ctzll(rest);rest&=rest-1;if(counts[b]+1<cost){cost=counts[b]+1;bit=b;}}
    int gain=__builtin_popcountll(unseen);double score=weight[gain]/cost;
    if(score>best+1e-12||(abs(score-best)<1e-12&&gain>bestgain)){best=score;bestgain=gain;chosen=k;witness=bit;}
   }
   U64 rest=unseen;while(rest){int b=__builtin_ctzll(rest);rest&=rest-1;counts[b]++;}
  }
  if(chosen<0)return false;U64 bit=1ULL<<witness;
  while(true){int at=find(order.begin(),order.end(),chosen)-order.begin(),from=fixed;while(from<at&&!(mask[order[from]]&bit))from++;
   if(from==at){if(at!=fixed)apply(at,fixed);covered|=mask[chosen];fixed++;break;}
   apply(from,at);if(operations.size()>384)return false;
  }
  if(operations.size()>384)return false;
 }
 return true;
}

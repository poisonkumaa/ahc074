#pragma once
static int round6_merge_width=2,round6_merge_max=44,round6_merge_refit=0,round6_merge_repetitions=0;
inline int round6_intrinsic(const vector<U64>&A,const vector<PrefixPaper>&papers){int total=0;vector<vector<int>>level(65);for(int k=0;k<int(papers.size());k++)level[papers[k].len].push_back(k);for(U64 target:A){int f[65];fill(f,f+65,1000);f[0]=0;for(int l=1;l<=64;l++)for(int k:level[l]){U64 diff=(papers[k].color^target)&PrefixDP::low(l);int start=diff?64-__builtin_clzll(diff):0;for(int j=start;j<l;j++)f[l]=min(f[l],f[j]+1);}total+=f[64];}return total;}
inline void round6_merge_dictionary(const vector<U64>&A,vector<PrefixPaper>&papers,int width,int maxl,int afterrefit){
 int original=round6_intrinsic(A,papers),initial=papers.size(),accepted=0;
 for(int l=maxl;l>=width;l--){vector<PrefixPaper>candidate;int amount=0;for(auto p:papers)if(p.len<=l&&p.len>l-width)amount++;else candidate.push_back(p);if(amount<2)continue;
  int use=amount-1;if(use>16)continue;for(int q=0;q<use;q++)candidate.push_back({l,A[q]&PrefixDP::low(l)});
  if(!round6_refit_level(A,candidate,l,32))continue;
  int before=round6_intrinsic(A,papers),after=round6_intrinsic(A,candidate);
  if(after<=before+8){papers=move(candidate);accepted++;}
 }
 if(afterrefit)refit_all_prefix_papers(A,papers,afterrefit,32);
 stable_sort(papers.begin(),papers.end(),[](auto a,auto b){return a.len<b.len;});
 cerr<<"Merge width="<<width<<" accepted="<<accepted<<" papers="<<initial<<"->"<<papers.size()<<" intrinsic="<<double(original)/A.size()<<"->"<<double(round6_intrinsic(A,papers))/A.size()<<"\n";
}

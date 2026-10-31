#include <bits/stdc++.h>
#define GENERAL_FUTURE_CHECK
#include "round6_general_future.h"
using namespace std;
int main(){mt19937_64 rng(171287);for(int trial=0;trial<2000;trial++){
 int K=12,N=6;vector<int>length(K),order(K);vector<char>selected(K),visible(K,1);vector<uint64_t>access(K);
 for(int k=0;k<K;k++){length[k]=1+rng()%N;access[k]=rng();}length.back()=N;selected.back()=1;
 for(int l=1;l<N;l++)if(rng()%2){vector<int>c;for(int k=0;k<K;k++)if(length[k]==l)c.push_back(k);if(!c.empty())selected[c[rng()%c.size()]]=1;}
 iota(order.begin(),order.end(),0);shuffle(order.begin(),order.end(),rng);
 auto move=GeneralPrefixFuture::best_move(order,length,selected,visible,length,.5,&access);
 int initial=GeneralPrefixFuture::fixed_distance(order,length,selected),best=initial;
 for(int i=0;i<K;i++)for(int j=0;j<K;j++)if(i!=j){auto p=order;int k=p[i];p.erase(p.begin()+i);p.insert(p.begin()+j,k);best=min(best,GeneralPrefixFuture::fixed_distance(p,length,selected));}
 if(move.first<0){assert(best==initial);continue;}int k=order[move.first];order.erase(order.begin()+move.first);order.insert(order.begin()+move.second,k);assert(GeneralPrefixFuture::fixed_distance(order,length,selected)==best);
 }cerr<<"general future checks passed\n";}

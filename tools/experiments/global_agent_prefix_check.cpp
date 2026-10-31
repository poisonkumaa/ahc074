#include <bits/stdc++.h>
#include "global_agent_prefix.h"
using namespace std;
int main(int argc,char**argv){
 int n=argc>1?atoi(argv[1]):3,c=argc>2?atoi(argv[2]):2,k=n*c;
 vector<int>len(k);for(int i=0;i<k;i++)len[i]=i/c+1;
 vector<vector<int>>ps;vector<int>p(k);iota(p.begin(),p.end(),0);do{ps.push_back(p);}while(next_permutation(p.begin(),p.end()));
 map<vector<int>,int>ids;for(int s=0;s<(int)ps.size();s++)ids[ps[s]]=s;
 vector<vector<int>>rev(ps.size());vector<int>shown(ps.size());
 for(int s=0;s<(int)ps.size();s++){int hi=0;for(int i=0;i<k;i++){int x=ps[s][i];if(len[x]<=hi)continue;hi=len[x];shown[s]|=1<<x;for(int d=0;d<k;d++)if(d!=i){auto q=ps[s];q.erase(q.begin()+i);q.insert(q.begin()+d,x);rev[ids[q]].push_back(s);}}}
 int tested=0;
 for(int mask=1;mask<(1<<k);mask++){
  int used=0;bool valid=true;vector<char>sel(k);for(int j=0;j<k;j++)if(mask>>j&1){if(used>>len[j]&1)valid=false;used|=1<<len[j];sel[j]=1;}if(!valid||!(used>>n&1))continue;
  vector<int>ds(ps.size(),999),q;for(int s=0;s<(int)ps.size();s++)if(shown[s]==mask)ds[s]=0,q.push_back(s);for(int h=0;h<(int)q.size();h++)for(int s:rev[q[h]])if(ds[s]>ds[q[h]]+1)ds[s]=ds[q[h]]+1,q.push_back(s);
  for(int s=0;s<(int)ps.size();s++){
   int d=GlobalPrefix::fixed_distance(ps[s],len,sel);if(d!=ds[s]){cerr<<"DIST ERROR "<<mask<<" "<<s<<" calc="<<d<<" true="<<ds[s]<<"\n";return 1;}
   auto [i,j]=GlobalPrefix::best_move(ps[s],len,sel);if(d==0){if(i!=-1)return 2;continue;}if(i<0){cerr<<"MISSING "<<mask<<" "<<s<<"\n";return 3;}
   int high=0;for(int h=0;h<i;h++)high=max(high,len[ps[s][h]]);if(len[ps[s][i]]<=high){cerr<<"INVISIBLE\n";return 4;}
   auto pp=ps[s];int x=pp[i];pp.erase(pp.begin()+i);pp.insert(pp.begin()+j,x);if(ds[ids[pp]]!=d-1){cerr<<"MOVE ERROR "<<mask<<" "<<s<<"\n";return 5;}tested++;
  }
 }
 cerr<<"states="<<ps.size()<<" verified_moves="<<tested<<" errors=0\n";
}

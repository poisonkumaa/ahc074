#define main submission_main
#include "../../main.cpp"
#undef main
int main(){
 int n=4,k=8;mt19937_64 rng(831);vector<PrefixPaper>papers(k);for(int z=0;z<k;z++){int l=z/2+1;papers[z]={l,(rng()&PrefixDP::low(l-1))|((U64)(z%2)<<(l-1))};}
 vector<vector<int>>ps;vector<int>p(k);iota(p.begin(),p.end(),0);do{ps.push_back(p);}while(next_permutation(p.begin(),p.end()));map<vector<int>,int>ids;for(int s=0;s<int(ps.size());s++)ids[ps[s]]=s;
 vector<vector<int>>rev(ps.size());vector<U64>shown(ps.size());
 for(int s=0;s<int(ps.size());s++){int hi=0;for(int i=0;i<k;i++){int z=ps[s][i],l=papers[z].len;if(l<=hi)continue;shown[s]|=papers[z].color&(PrefixDP::low(l)^PrefixDP::low(hi));hi=l;for(int d=0;d<k;d++)if(d!=i){auto q=ps[s];q.erase(q.begin()+i);q.insert(q.begin()+d,z);rev[ids[q]].push_back(s);}}}
 PrefixDP dp(n,papers);int tested=0;vector<int>len;for(auto p:papers)len.push_back(p.len);
 for(int t=0;t<(1<<n);t++){
  vector<int>dist(ps.size(),999),q;for(int s=0;s<int(ps.size());s++)if(shown[s]==U64(t)){dist[s]=0;q.push_back(s);}for(int h=0;h<int(q.size());h++)for(int s:rev[q[h]])if(dist[s]>dist[q[h]]+1){dist[s]=dist[q[h]]+1;q.push_back(s);}
  for(int s=0;s<int(ps.size());s++){
   for(int z=0;z<k;z++)dp.reward[z]=16+2*(rng()%65);int moves;auto goal=dp.solve(ps[s],t,moves);if(moves!=dist[s]){cerr<<"DP distance mismatch "<<t<<" "<<s<<" "<<moves<<" "<<dist[s]<<endl;return 1;}
   vector<char>selected(k);U64 color=0;int hi=0;for(int z:goal){if(papers[z].len<=hi)return 2;color|=papers[z].color&(PrefixDP::low(papers[z].len)^PrefixDP::low(hi));hi=papers[z].len;selected[z]=1;}if(hi!=n||color!=U64(t))return 3;
   if(GlobalPrefix::fixed_distance(ps[s],len,selected)!=moves)return 4;tested++;
  }
 }
 cerr<<"Weighted DP checked="<<tested<<" errors=0\n";
}

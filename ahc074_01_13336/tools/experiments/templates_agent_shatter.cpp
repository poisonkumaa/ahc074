#include <bits/stdc++.h>
using namespace std;using U64=uint64_t;
int main(int argc,char**argv){int n=argc>1?atoi(argv[1]):4,b=argc>2?atoi(argv[2]):n;vector<vector<int>>ps;vector<int>p(n);iota(p.begin(),p.end(),0);do{ps.push_back(p);}while(next_permutation(p.begin(),p.end()));int ns=ps.size();
 vector<vector<uint8_t>>fs;vector<string>cs;set<vector<uint8_t>>seen;int nc=1;for(int i=0;i<n;i++)nc*=3;for(int code=0;code<nc;code++){int x=code;string col(n,'.');for(auto &c:col){c=".01"[x%3];x/=3;}vector<uint8_t>f;for(auto &p:ps){int v=0;for(int k:p)if(col[k]!='.'){v=col[k]-'0';break;}f.push_back(v);}if(f[0]){for(auto &v:f)v^=1;for(auto &c:col)if(c!='.')c^=1;}if(accumulate(f.begin(),f.end(),0)==0)continue;if(seen.insert(f).second)fs.push_back(f),cs.push_back(col);}
 cerr<<"n="<<n<<" functions="<<fs.size()<<" states="<<ns<<"\n";
 vector<int>chosen;long long nodes=0;auto rec=[&](auto&&self,vector<int>&label,int depth,int begin)->bool{
  nodes++;if(depth==b){for(int c:chosen)cout<<cs[c]<<"\n";return true;}
  for(int c=begin;c<(int)fs.size();c++){vector<int>count(1<<(depth+1)),next(ns);for(int s=0;s<ns;s++){next[s]=label[s]*2+fs[c][s];count[next[s]]++;}bool ok=true;for(int z:count)if(z<(1<<(b-depth-1))){ok=false;break;}if(!ok)continue;chosen.push_back(c);if(self(self,next,depth+1,c+1))return true;chosen.pop_back();}
  return false;
 };vector<int>label(ns);bool found=rec(rec,label,0,0);cerr<<"found="<<found<<" nodes="<<nodes<<"\n";
}

#include <bits/stdc++.h>
using namespace std;
int main(int argc,char**argv){
 int K=argc>1?atoi(argv[1]):4,D=argc>2?atoi(argv[2]):5;vector<vector<int>>perms;vector<int>p(K);iota(p.begin(),p.end(),0);do{perms.push_back(p);}while(next_permutation(p.begin(),p.end()));int S=perms.size();
 vector<vector<uint8_t>>functions;set<vector<uint8_t>>seen;int power=1;for(int i=0;i<K;i++)power*=3;
 for(int code=0;code<power;code++){int c=code;vector<int>color(K);for(int k=0;k<K;k++)color[k]=c%3,c/=3;vector<uint8_t>f(S);for(int s=0;s<S;s++)for(int k:perms[s])if(color[k]){f[s]=color[k]==2;break;}if(seen.insert(f).second)functions.push_back(f);}
 cerr<<"K="<<K<<" states="<<S<<" functions="<<functions.size()<<" D="<<D<<"\n";
 auto coverage=[&](const vector<int>&rows){uint64_t found[8]{};int answer=0;for(auto&f:functions){int value=0;for(int i=0;i<D;i++)value|=int(f[rows[i]])<<i;uint64_t bit=1ULL<<(value%64);if(!(found[value/64]&bit))found[value/64]|=bit,answer++;}return answer;};
 vector<int>rows(D);iota(rows.begin(),rows.end(),0);int best=0;vector<int>bestrows;long long iterations=argc>3?atoll(argv[3]):1000000;mt19937 rng(3124123);
 if(K==4&&D==5){while(true){int score=coverage(rows);if(score>best){best=score;bestrows=rows;}if(best==(1<<D))break;int i=D-1;while(i>=0&&rows[i]==S-D+i)i--;if(i<0)break;rows[i]++;for(int j=i+1;j<D;j++)rows[j]=rows[j-1]+1;}}
 else {int score=coverage(rows);for(int it=0;it<iterations;it++){int i=rng()%D,old=rows[i],value=rng()%S;if(find(rows.begin(),rows.end(),value)!=rows.end())continue;rows[i]=value;int now=coverage(rows);double temp=.15+1.0*(1.-double(it%100000)/100000);if(now>=score||double(rng())/rng.max()<exp((now-score)/temp))score=now;else rows[i]=old;if(score>best){best=score;bestrows=rows;cerr<<"best="<<best<<"\n";}if(best==(1<<D))break;}}
 cout<<"BEST "<<best<<" / "<<(1<<D)<<"\n";for(int s:bestrows){cout<<s<<":";for(int k:perms[s])cout<<k;cout<<"\n";}
}

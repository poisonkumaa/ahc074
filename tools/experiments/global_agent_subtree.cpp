#define main v3_main
#include "/mnt/c/Users/dokuk/Downloads/ahc074_v3_main.cpp"
#undef main
using namespace std;
string shape(const vector<int>&parent,int x){vector<string>children;for(int j=x+1;j<(int)parent.size();j++)if(parent[j]==x)children.push_back(shape(parent,j));sort(children.begin(),children.end());string s="(";for(auto&a:children)s+=a;return s+")";}
int main(int argc,char**argv){
 int n=argc>1?atoi(argv[1]):6;vector<int>parent(n,-1);set<string>seen;vector<pair<string,vector<int>>>trees;
 function<void(int)>gen=[&](int x){if(x==n){string s=shape(parent,0);if(seen.insert(s).second)trees.push_back({s,parent});return;}for(int p=0;p<x;p++)parent[x]=p,gen(x+1);};gen(1);
 vector<pair<double,string>>results;
 for(auto [label,pa]:trees){
  vector<string>patterns(n+1,string(n,'.'));patterns[n]=string(n,'1');
  for(int j=0;j<n;j++)for(int x=j;x>=0;x=pa[x])patterns[x][j]=(x==j?'0':'1');
  Graph graph(patterns);auto policy=graph.make_policy(vector<int>(1<<n,1));Random rnd{12345};int state=0;long moves=0;
  const int R=100000;
  for(int r=0;r<R+1000;r++){int target=rnd.pick(1<<n),count=0;while(graph.display[state]!=target){auto edge=graph.edges[state][policy[target*graph.states+state]];state=edge.next;count++;}if(r>=1000)moves+=count;}
  double mean=double(moves)/R;results.push_back({mean,label});cerr<<label<<" mean="<<mean<<" perbit="<<mean/n<<"\n";
 }
 sort(results.begin(),results.end());cerr<<"BEST ";for(int i=0;i<min(5,int(results.size()));i++)cerr<<results[i].first<<" "<<results[i].second<<" | ";cerr<<"\n";
}

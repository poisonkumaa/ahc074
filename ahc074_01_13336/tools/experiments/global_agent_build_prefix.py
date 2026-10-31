from pathlib import Path
root=Path(__file__).resolve().parent
dp=(root/'prefix_dp.h').read_text().replace('#pragma once\n','')
mover=(root/'global_agent_prefix.h').read_text().replace('#pragma once\n','')
source=(root/'prefix_run.cpp').read_text()
trie=source[source.index('vector<U64> trie_patterns('):source.index('int fixed_dist(')]
main=r'''
int main(){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int n,K,P,R;if(!(cin>>n>>K>>P>>R)||n!=64||K!=192||P!=256)return 0;
 vector<U64>A(P);for(int i=0;i<P;i++){string s;cin>>s;if(int(s.size())!=n)return 0;for(int j=0;j<n;j++)if(s[j]=='1')A[i]|=1ULL<<j;}
 mt19937_64 rng(12345);vector<int>cols(n),flip(n);iota(cols.begin(),cols.end(),0);
 int correlation[64][64]{};for(int x=0;x<n;x++)for(int y=0;y<n;y++)for(U64 v:A)correlation[x][y]+=((v>>x)^(v>>y))&1;
 auto cost=[&](){int s=0;for(int j=1;j<n;j++){int d=correlation[cols[j-1]][cols[j]];s+=min(d,P-d);}return s;};
 int current=cost(),best=current;auto bestcols=cols;
 for(int it=0;it<300000;it++){
  int x=rng()%n,y=rng()%n;if(x>y)swap(x,y);(void)(rng()%3);
  reverse(cols.begin()+x,cols.begin()+y+1);int next=cost();double temperature=8*pow(.002,double(it)/300000);
  if(next<=current||double(rng()>>11)*0x1p-53<exp((current-next)/temperature)){current=next;if(current<best)best=current,bestcols=cols;}
  else reverse(cols.begin()+x,cols.begin()+y+1);
 }
 cols=bestcols;for(int j=1;j<n;j++)flip[j]=flip[j-1]^(correlation[cols[j-1]][cols[j]]>P/2);
 for(U64&v:A){U64 old=v;v=0;for(int j=0;j<n;j++)v|=(((old>>cols[j])&1)^flip[j])<<j;}
 vector<PrefixPaper>papers;
 for(int length=1;length<=n;length++){
  auto patterns=trie_patterns(A,length,16,3);
  bool has[2]{};for(U64 v:patterns)has[v>>(length-1)&1]=true;
  if(patterns.size()>1&&(!has[0]||!has[1]))patterns[0]^=1ULL<<(length-1);
  for(U64 v:patterns)papers.push_back({length,v});
 }
 if(papers.size()>size_t(K))return 0;
 for(int k=0;k<K;k++){
  string pattern(n,'.');if(k<int(papers.size()))for(int j=0;j<papers[k].len;j++)pattern[cols[j]]=char('0'+(((papers[k].color>>j)&1)^flip[j]));
  cout<<pattern<<'\n';
 }
 cout.flush();
 PrefixDP dp(n,papers);vector<int>order(papers.size()),length;for(auto p:papers)length.push_back(p.len);iota(order.begin(),order.end(),0);
 for(int r=0;r<R;r++){
  int product;if(!(cin>>product)||product<0||product>=P)return 0;
  int moves;auto chosen=dp.solve(order,A[product],moves);if(moves<0||moves>2*K)return 0;
  vector<char>selected(papers.size());for(int k:chosen)selected[k]=true;
  vector<pair<int,int>>operations;operations.reserve(moves);
  for(int i=0;i<moves;i++){
   auto [from,to]=GlobalPrefix::best_move(order,length,selected);if(from<0)return 0;
   int k=order[from];order.erase(order.begin()+from);order.insert(order.begin()+to,k);operations.push_back({k,to});
  }
  cout<<operations.size()<<'\n';for(auto [k,d]:operations)cout<<k<<' '<<d<<'\n';cout.flush();
 }
}
'''
out='// AHC074: public-data prefix dictionaries, exact legal reactive paths.\n#pragma GCC optimize("O3")\n'+dp+'\n'+mover+'\n'+trie+'\n'+main
(root/'global_agent_prefix_submit.cpp').write_text(out)

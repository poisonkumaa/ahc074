// Witness-color precedence DAG. A maximum antichain of its intersection
// with the reversed current order is the longest retainable subsequence.
inline vector<int> color_dag_target(const vector<int>&order,const vector<int>&length,const vector<char>&selected,const vector<U64>&mask,const vector<U64>&color,U64 target,int mode,int&minimum,const vector<int>*feasible_order=nullptr){
 int K=order.size(),pos[192],witness[64];for(int i=0;i<K;i++)pos[order[i]]=i;
 vector<int>roots;if(feasible_order){U64 used=0;for(int k:*feasible_order){if(mask[k]&~used)roots.push_back(k);used|=mask[k];}}else{for(int k=0;k<K;k++)if(selected[k])roots.push_back(k);sort(roots.begin(),roots.end(),[&](int x,int y){return length[x]<length[y];});}
 U64 covered=0;for(int k:roots){U64 bits=mask[k]&~covered;assert(((color[k]^target)&bits)==0);while(bits){int bit=__builtin_ctzll(bits);bits&=bits-1;witness[bit]=k;}covered|=mask[k];}assert(covered==~0ULL);
 array<U64,3>reach[192]{},inverse[192]{};
 for(int k=0;k<K;k++){U64 bad=(color[k]^target)&mask[k];while(bad){int bit=__builtin_ctzll(bad);bad&=bad-1;int source=witness[bit];reach[source][k/64]|=1ULL<<(k%64);}}
 for(int i=int(roots.size())-1;i>=0;i--){int source=roots[i];for(int j=i+1;j<int(roots.size());j++){int via=roots[j];if(reach[source][via/64]>>(via%64)&1)for(int b=0;b<3;b++)reach[source][b]|=reach[via][b];}}
 for(int source:roots)for(int k=0;k<K;k++)if(pos[k]<pos[source]&&(reach[source][k/64]>>(k%64)&1))inverse[source][k/64]|=1ULL<<(k%64);
 int left[192],right[192],seen[192]{},epoch=0;fill(left,left+K,-1);fill(right,right+K,-1);
 auto each=[&](const array<U64,3>&bits,auto&&f){for(int block=0;block<3;block++){int b=mode&1?2-block:block;U64 word=bits[b];while(word){int bit=mode&1?63-__builtin_clzll(word):__builtin_ctzll(word);word&=~(1ULL<<bit);if(f(b*64+bit))return true;}}return false;};
 function<bool(int)>augment=[&](int source){if(seen[source]==epoch)return false;seen[source]=epoch;return each(inverse[source],[&](int destination){if(right[destination]<0||augment(right[destination])){left[source]=destination;right[destination]=source;return true;}return false;});};
 minimum=0;auto sources=roots;if(mode&2)reverse(sources.begin(),sources.end());for(int source:sources){epoch++;minimum+=augment(source);}
 bool visitLeft[192]{},visitRight[192]{};vector<int>queue;for(int k=0;k<K;k++)if(left[k]<0){visitLeft[k]=true;queue.push_back(k);}
 for(int at=0;at<int(queue.size());at++){int source=queue[at];each(inverse[source],[&](int destination){if(destination==left[source]||visitRight[destination])return false;visitRight[destination]=true;int next=right[destination];if(next>=0&&!visitLeft[next]){visitLeft[next]=true;queue.push_back(next);}return false;});}
 vector<char>keep(K);int count=0,last=-1;for(int k:order)if(visitLeft[k]&&!visitRight[k]){keep[k]=1;count++;if(last>=0)reach[last][k/64]|=1ULL<<(k%64);last=k;}assert(count==K-minimum);
 int indegree[192]{};for(int k=0;k<K;k++)each(reach[k],[&](int v){indegree[v]++;return false;});
 vector<int>result;vector<char>done(K);for(int i=0;i<K;i++){int next=-1;for(int k:order)if(!done[k]&&!indegree[k]){next=k;break;}assert(next>=0);done[next]=1;result.push_back(next);each(reach[next],[&](int v){indegree[v]--;return false;});}
 return result;
}

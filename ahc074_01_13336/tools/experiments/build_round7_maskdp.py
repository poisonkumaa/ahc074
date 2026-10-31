from pathlib import Path
d=Path(__file__).resolve().parent
s=(d.parents[1]/'main.cpp').read_text().replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100').replace('#define AHC_REFINEMENT_BUDGET 1.65','#define AHC_REFINEMENT_BUDGET 100')
s=s.replace('int pos[192];vector<int>reward;', 'int pos[192];array<array<U64,3>,66>positionMask{};array<array<U64,3>,193>prefixMask{};vector<int>reward;')
old='void prepare(const vector<int>&p){for(int i=0;i<k;i++)pos[p[i]]=i;for(int l=0;l<=n+1;l++){cnt[l][0]=0;for(int i=0;i<k;i++)cnt[l][i+1]=cnt[l][i]+(a[p[i]].len>=l);}}'
new='''void prepare(const vector<int>&p){
  for(auto&row:positionMask)row.fill(0);
  prefixMask[0].fill(0);for(int i=0;i<k;i++){pos[p[i]]=i;positionMask[a[p[i]].len][i>>6]|=1ULL<<(i&63);prefixMask[i+1]=prefixMask[i];prefixMask[i+1][i>>6]|=1ULL<<(i&63);}
  for(int l=n-1;l>=1;l--)for(int w=0;w<3;w++)positionMask[l][w]|=positionMask[l+1][w];
 #ifdef PREFIX_MASK_CHECK
  for(int l=0;l<=n+1;l++){cnt[l][0]=0;for(int i=0;i<k;i++)cnt[l][i+1]=cnt[l][i]+(a[p[i]].len>=l);}
 #endif
 }
 inline int mask_cost(int j,int l,int q,int h){
  int moved=0;U64 extra=0;for(int w=h>>6;w<=(q-1)>>6;w++){U64 range=prefixMask[q][w]^prefixMask[h][w];moved+=__builtin_popcountll(positionMask[l][w]&range);extra|=(positionMask[j+1][w]^positionMask[l][w])&range;}int result=moved+(extra!=0);
 #ifdef PREFIX_MASK_CHECK
  int reference=cnt[l][q]-cnt[l][h]+(cnt[j+1][q]-cnt[j+1][h]>cnt[l][q]-cnt[l][h]);assert(result==reference);
 #endif
  return result;
 }'''
assert old in s;s=s.replace(old,new)
oldcost='if(q>=h){int moved=cnt[l][q]-cnt[l][h],upper=cnt[j+1][q]-cnt[j+1][h];add=moved+(upper>moved);}'
assert oldcost in s;s=s.replace(oldcost,'if(q>h)add=mask_cost(j,l,q,h);')
s=s.replace('int main(){','int main(int argc,char**argv){int reps=argc>1?atoi(argv[1]):8;')
s=s.replace('exact_state_refit(A,papers,1,43,32,-1,0);','exact_state_refit(A,papers,1,43,32,reps,0);')
(d/'round7_maskdp.cpp').write_text(s)

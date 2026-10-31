#pragma once
#ifndef WORDS
#define WORDS 4
#endif
struct Access {
 array<uint64_t,WORDS> w{};
 Access(uint64_t v=0){w[0]=v;}
 static Access bit(int i){Access x;x.w[(i>>6)%WORDS]=uint64_t(1)<<(i&63);return x;}
 Access& operator|=(const Access&a){for(int i=0;i<WORDS;i++)w[i]|=a.w[i];return *this;}
 friend Access operator|(Access a,const Access&b){return a|=b;}
 friend Access operator&(const Access&a,const Access&b){Access c;for(int i=0;i<WORDS;i++)c.w[i]=a.w[i]&b.w[i];return c;}
 friend Access operator~(Access a){for(auto&x:a.w)x=~x;return a;}
};
inline int maskpop(uint64_t x){x-= (x>>1)&0x5555555555555555ULL;x=(x&0x3333333333333333ULL)+((x>>2)&0x3333333333333333ULL);x=(x+(x>>4))&0x0f0f0f0f0f0f0f0fULL;return (x*0x0101010101010101ULL)>>56;}
inline int maskpop(const Access&a){int n=0;for(auto x:a.w)n+=maskpop(x);return n;}

namespace GlobalPrefix256 {

inline int fixed_distance(const std::vector<int>& order,
                          const std::vector<int>& length,
                          const std::vector<char>& selected) {
    const int K = order.size();
    const int N = *std::max_element(length.begin(), length.end());
    std::vector<int> owner(N+2,-1), pos(K), next(K), bad(K);
    std::vector<char> moved(K);
    for (int k=0;k<K;k++) if(selected[k]) owner[length[k]]=k;
    int root=-1;
    for(int l=N;l>=0;l--) { if(owner[l]>=0)root=owner[l]; owner[l]=root; }
    for(int k=0;k<K;k++) next[k]=owner[length[k]];
    int minimum=N+1, answer=0;
    for(int i=K-1;i>=0;i--) {
        int k=order[i]; pos[k]=i;
        moved[k]=length[k]>=minimum; answer+=moved[k];
        if(selected[k]) minimum=std::min(minimum,length[k]);
    }
    for(int k=0;k<K;k++) if(!selected[k]&&!moved[k]&&pos[k]<pos[next[k]]) bad[next[k]]++;
    for(int k=0;k<K;k++) if(selected[k]&&!moved[k]&&bad[k]) answer++;
    return answer;
}

// Returns (old position, insertion position after removal), or (-1,-1) if
// the goal is already reached.  Every returned move is legal and lowers
// fixed_distance by exactly one.  Enumerating all positions is O(K * V),
// where V is the number of currently visible papers; distance updates are O(1).
// Among shortest-path moves minimize the number of mandatory removals averaged
// over 64 previous goals. A suffix union counts correlated choices exactly.
// Length inversions break remaining ties and initialize the access model.
inline std::pair<int,int> best_move(const std::vector<int>& order,
                                  const std::vector<int>& length,
                                  const std::vector<char>& selected,
                                  double decay=.5, const std::vector<Access>* access=nullptr) {
    const int K=order.size();
    const int N=*std::max_element(length.begin(),length.end());
    std::array<int,66> owner;owner.fill(-1);std::array<int,192> next,pos;std::array<int,193> suffix;suffix.fill(N+1);
    std::array<int,192> initial_bad{},inversions{};
    std::array<char,192> initial_moved{},initial_before{};
    std::array<double,193> power;power[0]=1.;
    for(int i=1;i<=K;i++)power[i]=power[i-1]*decay;
    for(int k=0;k<K;k++)if(selected[k])owner[length[k]]=k;
    int root=-1;
    for(int l=N;l>=0;l--){if(owner[l]>=0)root=owner[l];owner[l]=root;}
    for(int k=0;k<K;k++)next[k]=owner[length[k]];
    for(int i=0;i<K;i++)pos[order[i]]=i;
    int initial_distance=0;
    for(int i=K-1;i>=0;i--){
        int k=order[i];initial_moved[k]=length[k]>=suffix[i+1];
        initial_distance+=initial_moved[k];
        suffix[i]=std::min(suffix[i+1],selected[k]?length[k]:N+1);
    }
    for(int k=0;k<K;k++)if(!selected[k]){
        initial_before[k]=pos[k]<pos[next[k]];
        if(initial_before[k]&&!initial_moved[k])initial_bad[next[k]]++;
    }
    for(int k=0;k<K;k++)if(selected[k]&&!initial_moved[k]&&initial_bad[k])initial_distance++;
    if(initial_distance==0)return {-1,-1};
    Access below[192]{}, mask_tree[65]{};
    if(access){for(int i=K-1;i>=0;i--){int k=order[i];for(int x=length[k];x>0;x-=x&-x)below[k]|=mask_tree[x];for(int x=length[k];x<=N;x+=x&-x)mask_tree[x]|=(*access)[k];}}
    double initial_quality=0;
    int initial_inversions=0;
    std::array<int,66> fenwick{};
    for(int i=K-1;i>=0;i--){
        int k=order[i];
        for(int x=length[k]-1;x>0;x-=x&-x)inversions[k]+=fenwick[x];
        for(int x=length[k];x<=N;x+=x&-x)fenwick[x]++;
        if(access)initial_quality+=maskpop(below[k]);
        else initial_quality+=1-power[inversions[k]];
        initial_inversions+=inversions[k];
    }
    std::pair<int,int> answer{-1,-1};double best_quality=std::numeric_limits<double>::infinity();
    int best_inversions=std::numeric_limits<int>::max(), high=0;
    std::array<char,192> moved,before;std::array<int,192>bad;
    for(int i=0;i<K;i++){
        int k=order[i],kl=length[k];if(kl<=high)continue;high=kl;
        if(!initial_moved[k]) {
            if(selected[k]) { if(!initial_bad[k])continue; }
            else if(!initial_before[k]||initial_moved[next[k]]||initial_bad[next[k]]!=1)continue;
        }
        // Mandatory papers must move downward.  An optional chosen record
        // moves upward over its uncovered interval; the sole optional leaf
        // can instead move downward below that chosen record.
        int direction=initial_moved[k]||!selected[k]?1:-1;
        Access remaining[193];if(access&&direction==1){remaining[K]=0;for(int d=K-1;d>i;d--){int x=order[d];remaining[d]=remaining[d+1]|(length[x]<=kl?(*access)[x]:Access(0));}}
        {
            moved=initial_moved;before=initial_before;bad=initial_bad;
            int distance=initial_distance, invk=inversions[k], invsum=initial_inversions;
            int following_min=suffix[i+1];double quality=initial_quality;Access own=0;int own_count=0;
            if(access){own=below[k];own_count=maskpop(own);}
            auto extra=[&](int group){return !moved[group]&&bad[group]>0;};
            auto change_moved=[&](int x,bool value){
                if(bool(moved[x])==value)return;
                int group=next[x];distance-=extra(group);
                distance+=int(value)-int(moved[x]);
                if(!selected[x]&&before[x])bad[group]+=int(moved[x])-int(value);
                moved[x]=value;distance+=extra(group);
            };
            auto change_before=[&](int x,bool value){
                if(bool(before[x])==value)return;
                int group=next[x];distance-=extra(group);
                if(!moved[x])bad[group]+=int(value)-int(before[x]);
                before[x]=value;distance+=extra(group);
            };
            for(int d=i+direction;d>=0&&d<K;d+=direction){
                int x=order[d],xl=length[x],old_invk=invk;
                if(direction==1){
                    if(selected[k]&&xl>=kl)change_moved(x,true);
                    change_moved(k,kl>=suffix[d+1]);
                    if(!selected[x]&&next[x]==k)change_before(x,true);
                    if(!selected[k]&&next[k]==x)change_before(k,false);
                    if(xl<kl){invk--;invsum--;}else if(xl>kl){if(!access)quality+=power[inversions[x]]-power[inversions[x]+1];invsum++;}
                    if(access){
                     int next_count=maskpop(remaining[d+1]);quality+=next_count-own_count;own_count=next_count;
                     if(xl>=kl)quality+=maskpop((*access)[k]&~below[x]);
                    }
                }else{
                    if(selected[k]&&xl>=kl)change_moved(x,xl>=following_min);
                    if(selected[x]&&kl>=xl)change_moved(k,true);
                    if(!selected[x]&&next[x]==k)change_before(x,false);
                    if(!selected[k]&&next[k]==x)change_before(k,true);
                    if(selected[x])following_min=std::min(following_min,xl);
                    if(xl<kl){invk++;invsum++;}else if(xl>kl){if(!access)quality-=power[inversions[x]-1]-power[inversions[x]];invsum--;}
                    if(access){
                     own|=(*access)[x];int next_count=maskpop(own);quality+=next_count-own_count;own_count=next_count;
                    }
                }
                if(!access)quality+=power[old_invk]-power[invk];
                if(distance!=initial_distance-1)continue;
                if(quality<best_quality-1e-10 ||
                   (std::abs(quality-best_quality)<=1e-10&&invsum<best_inversions)){
                    best_quality=quality;best_inversions=invsum;answer={i,d};
                }
            }
        }
        if(answer.first>=0)break;
    }
    return answer;
}
}


namespace GlobalPrefix256 {
inline std::vector<int> final_order(const std::vector<int>&order,const std::vector<int>&length,const std::vector<char>&selected,const std::vector<Access>*future=nullptr){
 using namespace std;
 int K=order.size(),N=*max_element(length.begin(),length.end()),owner[65],root=-1;fill(owner,owner+65,-1);for(int k=0;k<K;k++)if(selected[k])owner[length[k]]=k;for(int l=N;l>=0;l--){if(owner[l]>=0)root=owner[l];owner[l]=root;}
 vector<char>mandatory(K);int suffix=N+1;for(int i=K-1;i>=0;i--){int k=order[i];mandatory[k]=length[k]>=suffix;if(selected[k])suffix=min(suffix,length[k]);}
 vector<vector<int>>children(K);vector<int>insert;int pos[192];for(int i=0;i<K;i++){int k=order[i];pos[k]=i;if(selected[k])continue;if(mandatory[k])insert.push_back(k);else children[owner[length[k]]].push_back(k);}
 stable_sort(insert.begin(),insert.end(),[&](int a,int b){return length[a]<length[b];});
 double power[193];power[0]=1;for(int i=1;i<=K;i++)power[i]=power[i-1]*.5;
 for(int k:insert){auto&v=children[owner[length[k]]];int M=v.size(),kl=length[k];Access below[192]{},remain[193]{};int inversions[192]{},lower_after[193]{};
  for(int i=M-1;i>=0;i--){int x=v[i];for(int j=i+1;j<M;j++){int y=v[j];if(length[y]<length[x])inversions[x]++;if(future&&length[y]<=length[x])below[x]|=(*future)[y];}remain[i]=remain[i+1]|(future&&length[x]<=kl?(*future)[x]:Access(0));lower_after[i]=lower_after[i+1]+(length[x]<kl);}
  double gain=0,best=1e100;int larger=0,bestinv=INT_MAX,at=0;
  for(int i=0;i<=M;i++){double q=gain+(future?double(maskpop(remain[i])):1-power[lower_after[i]]);int inv=larger+lower_after[i];if(q<best-1e-10||(abs(q-best)<=1e-10&&inv<bestinv)){best=q;bestinv=inv;at=i;}
   if(i<M){int x=v[i];if(length[x]>kl)larger++;if(future){if(length[x]>=kl)gain+=maskpop((*future)[k]&~below[x]);}else if(length[x]>kl)gain+=power[inversions[x]]*.5;}
  }
  v.insert(v.begin()+at,k);
 }
 vector<int>out;out.reserve(K);for(int l=1;l<=N;l++){int k=owner[l];if(k>=0&&selected[k]&&length[k]==l){out.push_back(k);for(int x:children[k])out.push_back(x);}}return out;
}
}

from pathlib import Path
d=Path(__file__).resolve().parent
s=(d.parents[1]/'main.cpp').read_text()
s=s.replace('#define AHC_LEARNING_BUDGET 0.5','#define AHC_LEARNING_BUDGET 100').replace('#define AHC_REFINEMENT_BUDGET 1.65','#define AHC_REFINEMENT_BUDGET 100')
begin=s.index('inline std::vector<int> final_order_incremental(')
end=s.index('\n}\n}\n\ninline vector<int> direct_final',begin)+2
f=s[begin:end].replace('final_order_incremental','control_final')
f=f.replace('uint64_t below[192]{};int inversions[192]{};', 'uint64_t below[192]{},belowOwner[192]{};int inversions[192]{},exactOwner[192]{};')
f=f.replace('below[x]|=(*future)[y];','{below[x]|=(*future)[y];belowOwner[x]|=ownerMask[y];exactOwner[x]+=ownerFreq[y];}')
f=f.replace('uint64_t remain[193];int lower_after[193];remain[M]=0;lower_after[M]=0;', 'uint64_t remain[193],remainOwner[193];int lower_after[193],afterOwner[193];remain[M]=remainOwner[M]=0;lower_after[M]=afterOwner[M]=0;')
f=f.replace('lower_after[i]=lower_after[i+1]+(length[x]<kl);','lower_after[i]=lower_after[i+1]+(length[x]<kl);remainOwner[i]=remainOwner[i+1]|(length[x]<=kl?ownerMask[x]:0ULL);afterOwner[i]=afterOwner[i+1]+(length[x]<=kl?ownerFreq[x]:0);')
f=f.replace('double(__builtin_popcountll(remain[i]))', '(4.0*__builtin_popcountll(remain[i])+control_beta*(afterOwner[i]-4*__builtin_popcountll(remainOwner[i])))')
f=f.replace('gain+=__builtin_popcountll((*future)[k]&~below[x]);','gain+=4.0*__builtin_popcountll((*future)[k]&~below[x])+control_beta*(ownerFreq[k]-4*__builtin_popcountll(ownerMask[k]&~belowOwner[x]));')
f=f.replace('below[k]=remain[at];','below[k]=remain[at];belowOwner[k]=remainOwner[at];exactOwner[k]=afterOwner[at];')
f=f.replace('below[x]|=(*future)[k];','{below[x]|=(*future)[k];belowOwner[x]|=ownerMask[k];exactOwner[x]+=ownerFreq[k];}')
declarations='''static double control_beta=1;static int control_reward=0;
static int ownerFreq[192],productOwner[256];static U64 ownerMask[192];
inline void control_init(const vector<U64>&A,const vector<PrefixPaper>&papers){
 for(int v=0;v<int(A.size());v++){int best=-1;for(int k=0;k<int(papers.size());k++)if((best<0||papers[k].len>papers[best].len)&&!((papers[k].color^A[v])&PrefixDP::low(papers[k].len)))best=k;assert(best>=0);productOwner[v]=best;ownerFreq[best]++;}
 vector<int>targets(A.size());iota(targets.begin(),targets.end(),0);mt19937 rng(912367);shuffle(targets.begin(),targets.end(),rng);for(int r=0;r<64;r++)ownerMask[productOwner[targets[r]]]|=1ULL<<r;
}
'''
s=s.replace('int main(){', declarations+f+'\nint main(int argc,char**argv){control_beta=argc>1?atof(argv[1]):1;control_reward=argc>2?atoi(argv[2]):0;int reps=argc>3?atoi(argv[3]):8;')
s=s.replace('exact_state_refit(A,papers,1,43,32,-1,0);','exact_state_refit(A,papers,1,43,32,reps,0);')
s=s.replace(' cout.flush();int fallbacks=0;', ' cout.flush();int fallbacks=0,missingOwner=0;control_init(A,papers);')
main=s.index('int main(int argc')
s=s[:main]+s[main:].replace('dp.reward[k]=16+2*__builtin_popcountll(future[k]);','dp.reward[k]=16+2*__builtin_popcountll(future[k])+(control_reward?lround(control_beta*(ownerFreq[k]*.5-2*__builtin_popcountll(ownerMask[k]))):0);').replace('auto target=direct_final(order,length,selected,&future);','auto target=control_final(order,length,selected,&future);').replace('  uint64_t bit=1ULL<<(r%64);','  missingOwner+=!selected[productOwner[product]];uint64_t ob=1ULL<<(r%64);for(int k=0;k<int(papers.size());k++)ownerMask[k]=(ownerMask[k]&~ob)|(k==productOwner[product]?ob:0ULL);\n  uint64_t bit=1ULL<<(r%64);').replace(' cerr<<"PathFallbacks="',' cerr<<"MissingOwner="<<missingOwner<<"\\n";cerr<<"PathFallbacks="')
(d/'round7_control.cpp').write_text(s)

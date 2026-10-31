from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'round7_adaptive.cpp').read_text()
s=s.replace('reward(k,16){}','reward(k,16){for(int i=0;i<k;i++){prefixMask[i+1]=prefixMask[i];prefixMask[i+1][i>>6]|=1ULL<<(i&63);}}')
s=s.replace('prefixMask[0].fill(0);','')
old='positionMask[a[p[i]].len][i>>6]|=1ULL<<(i&63);prefixMask[i+1]=prefixMask[i];prefixMask[i+1][i>>6]|=1ULL<<(i&63);'
assert old in s;s=s.replace(old,'positionMask[a[p[i]].len][i>>6]|=1ULL<<(i&63);')
s=s.replace('#ifndef AHC_LEARNING_BUDGET','#ifndef AHC_CAPTURE_FACTOR\n#define AHC_CAPTURE_FACTOR 7\n#endif\n#ifndef AHC_LEARNING_BUDGET')
s=s.replace('(5.0*max(per_repetition,1e-6))','(AHC_CAPTURE_FACTOR*max(per_repetition,1e-6))')
(d/'round7_adaptive_v2.cpp').write_text(s)

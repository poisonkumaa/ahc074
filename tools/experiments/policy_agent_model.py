import numpy as np
from pathlib import Path
import sys
p=Path(__file__).resolve().parent
bits=int(sys.argv[1]);suffix=sys.argv[2] if len(sys.argv)>2 else ''
v=1<<bits;r=24
m=np.fromfile(p/f'policy_agent_matrix{bits}{suffix}.bin',dtype=np.float64).reshape(v,v)
m=(m+m.T)/2
row=m.mean(axis=1);mean=m.mean();center=m-row[:,None]-row[None,:]+mean
eig,vec=np.linalg.eigh(center);ix=np.argsort(np.abs(eig))[-r:][::-1]
features=vec[:,ix]*np.sqrt(np.abs(eig[ix]))/256
features=np.c_[features,2*row/256,np.zeros((v,3))]
out=f'constexpr int MODEL_BITS={bits},MODEL_V={v},RANK={r},PAD=28;\nconstexpr float COST_MEAN={mean:.9g}f;\n'
out+='const float SIGNS[24]={'+','.join('1' if eig[i]>0 else '-1' for i in ix)+'};\n'
out+='alignas(32) const float FEATURE[MODEL_V][PAD]={\n'
for a in features:
    out+='{'+','.join(f'{x:.8g}f' if x else '0.f' for x in a)+'},\n'
out+='};\n'
(p/f'policy_agent_model{bits}{suffix}.h').write_text(out)
print(bits,mean,eig[ix],flush=True)

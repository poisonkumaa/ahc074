import sys,time,json
from pathlib import Path
from collections import Counter
import numpy as np
from scipy.optimize import milp,Bounds,LinearConstraint
from scipy.sparse import coo_array
p=Path(__file__).resolve().parent
name=sys.argv[1] if len(sys.argv)>1 else 'templates_agent_hub16_result.txt'
lines=(p/name).read_text().splitlines();N=len(lines[0]);K=N*3;P=N*4
patterns=[list(s) for s in lines[:K]]
targets=[];paths=[];final=[];checks=[]
for s in lines[K:]:
    words=s.split();targets.append(words[0]);moves=[tuple(map(int,w.split(','))) for w in words[1:]];paths.append(moves)
    order=list(range(K))
    for k,d in moves:
        checks.append((tuple(order),k));old=order.index(k);order.remove(k);order.insert(d,k)
        if abs(old-d)!=1:checks.append((tuple(order),k))
    final.append(order)
def visible(order,k,exclude=-1):
    for j in range(N):
        if j==exclude:continue
        for q in order:
            if patterns[q][j]!='.':
                if q==k:return True
                break
    return False
def errors():
    error=0;bad=0
    for a,order in enumerate(final):
        for j in range(N):
            c=next((patterns[k][j] for k in order if patterns[k][j]!='.'),'0')
            error+=c!=targets[a][j]
    for order,k in checks:bad+=not visible(order,k)
    return error,bad,error+3*bad
def solve(j,timeout):
    critical=Counter((order,k) for order,k in checks if not visible(order,k,j))
    if len(sys.argv)>4 and sys.argv[4]=='novis':critical.clear()
    num=K*2+P+len(critical);obj=np.zeros(num)
    for k in range(K):
        obj[2*k]=(-1 if patterns[k][j]=='0' else 1)*1e-5
        obj[2*k+1]=(-1 if patterns[k][j]=='1' else 1)*1e-5
    obj[K*2:K*2+P]=1
    rows=[];cols=[];vals=[];ub=[]
    def add(items,upper):
        r=len(ub);ub.append(upper)
        for c,v in items:rows.append(r);cols.append(c);vals.append(v)
    for k in range(K):add([(2*k,1),(2*k+1,1)],1)
    for a,order in enumerate(final):
        target=int(targets[a][j]);prefix=[];err=K*2+a
        for k in order:
            add([(2*k+1-target,1),(err,-1)]+prefix,0)
            prefix.append((2*k+target,-1))
        if target:add(prefix+[(err,-1)],-1)
    for h,((order,k),weight) in enumerate(critical.items()):
        var=K*2+P+h;obj[var]=3*weight
        add([(2*k,-1),(2*k+1,-1),(var,-1)],-1)
        for q in order:
            if q==k:break
            add([(2*q,1),(2*q+1,1),(var,-1)],0)
    mat=coo_array((np.array(vals,float),(np.array(rows,np.int32),np.array(cols,np.int32))),shape=(len(ub),num)).tocsc()
    before=errors();tic=time.perf_counter()
    result=milp(obj,integrality=np.ones(num),bounds=Bounds(np.zeros(num),np.ones(num)),constraints=LinearConstraint(mat,-np.inf,np.array(ub)),options={'time_limit':timeout,'mip_rel_gap':0.0})
    old=[patterns[k][j] for k in range(K)]
    if result.x is not None:
        for k in range(K):patterns[k][j]='0' if result.x[2*k]>.5 else '1' if result.x[2*k+1]>.5 else '.'
    after=errors()
    metric=0 if len(sys.argv)>4 and sys.argv[4]=='novis' else 2
    if after[metric]>before[metric]:
        for k in range(K):patterns[k][j]=old[k]
        after=before
    print('column',j,'score',before,'->',after,'bound',getattr(result,'mip_gap',None),'status',result.status,'seconds',round(time.perf_counter()-tic,3),flush=True)
print('initial',errors(),flush=True)
passes=int(sys.argv[2]) if len(sys.argv)>2 else 2
timeout=float(sys.argv[3]) if len(sys.argv)>3 else 1.0
for it in range(passes):
    old=errors()
    for j in range(N):solve(j,timeout)
    if errors()==old:break
out=[''.join(s) for s in patterns]
for a in range(P):out.append(targets[a]+' '+' '.join(f'{k},{d}' for k,d in paths[a]))
(p/('policy_agent_codebook_'+name)).write_text('\n'.join(out)+'\n')
print('FINAL',errors(),flush=True)

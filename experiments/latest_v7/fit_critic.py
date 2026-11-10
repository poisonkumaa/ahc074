"""Fit a regularized residual to the original critic, grouping CV by case.

Only explicitly supplied training reports are read. Labels are paired synthetic
continuations; neither the real future requests nor their realized score enter
the regression. Validation reports must stay outside this invocation.
"""
from pathlib import Path
import argparse,json
import numpy as np

ORIGINAL=np.array([.7074732953,.2206926173,2.123296404,.1205360474,.1966473467,-.003839332454,-.03799749686,.2478845857,.5201813283,-.8211577986,-.005086526272,.7537432423,1.174147517,1.352149126,-.8314049437,.02066116177,-.01661045614,.04533768486])
PRIOR=.75*ORIGINAL
PRIOR[0]+=.25

def load(paths):
    rows=[]
    for path in paths:
        data=json.loads(Path(path).read_text(encoding='utf-8'))
        for case in data['cases']:
            for line in case['stderr'].splitlines():
                if not line.startswith('CRITIC_SAMPLE '):continue
                fields=line.split()
                assert len(fields)==23,(path,line)
                q,a,d=map(int,fields[1:4])
                features=list(map(float,fields[4:22]))
                target=float(fields[22])
                rows.append((case['seed'],q,a,d,features,target))
    assert rows,'No CRITIC_SAMPLE rows found'
    return rows

def fit(x,y,lam):
    scale=np.sqrt(np.mean(x*x,axis=0))
    scale=np.maximum(scale,.02)
    z=x/scale
    residual=y-x@PRIOR
    update=np.linalg.solve(z.T@z+lam*len(z)*np.eye(18),z.T@residual)
    return PRIOR+update/scale

def scores(x,y,delta,w):
    error=x@w-y
    prediction=delta+x@w
    observed=delta+y
    # One noisy paired continuation per row: diagnostic, not actual benchmark.
    selected=np.where(prediction<-.05,observed,0.)
    return dict(mse=float(np.mean(error*error)),mae=float(np.mean(abs(error))),
                paired_choice_cost=float(np.mean(selected)))

def main():
    p=argparse.ArgumentParser()
    p.add_argument('reports',nargs='+')
    p.add_argument('--output',required=True)
    args=p.parse_args()
    rows=load(args.reports)
    groups=np.array([r[0] for r in rows])
    delta=np.array([r[3] for r in rows],dtype=float)
    x=np.array([r[4] for r in rows]);y=np.array([r[5] for r in rows])
    assert np.isfinite(x).all() and np.isfinite(y).all()
    seeds=sorted(map(int,set(groups)));assert len(seeds)>=5
    fold_of={seed:i%5 for i,seed in enumerate(seeds)}
    folds=np.array([fold_of[s] for s in groups])
    entries=[]
    for lam in [.03,.1,.3,1.,3.,10.,30.,100.]:
        errors=[];out=np.zeros_like(y)
        for f in range(5):
            train=folds!=f;valid=~train
            w=fit(x[train],y[train],lam)
            out[valid]=x[valid]@w
            errors.append(float(np.mean((out[valid]-y[valid])**2)))
        entries.append(dict(penalty=lam,mse=float(np.mean((out-y)**2)),
                            fold_mse=errors,se=float(np.std(errors,ddof=1)/np.sqrt(5))))
    best=min(entries,key=lambda v:v['mse'])
    # Prefer the stronger prior when its CV error is within one standard error.
    chosen=max((v for v in entries if v['mse']<=best['mse']+best['se']),key=lambda v:v['penalty'])
    weight=fit(x,y,chosen['penalty'])
    prior=scores(x,y,delta,PRIOR)
    result=dict(count=len(rows),seeds=seeds,target_mean=float(y.mean()),
                target_sd=float(y.std()),prior=prior,cv=entries,chosen=chosen,
                prior_cv_better=prior['mse']<chosen['mse'],
                best_cv_weights=((fit(x,y,best['penalty'])-.25*np.eye(18)[0])/.75).tolist(),
                total_weights=weight.tolist(),
                learned_weights=((weight-.25*np.eye(18)[0])/.75).tolist(),
                training=scores(x,y,delta,weight))
    Path(args.output).write_text(json.dumps(result,indent=2),encoding='utf-8')
    print(json.dumps({k:result[k] for k in ['count','prior','chosen','prior_cv_better','training']},indent=2))

if __name__=='__main__':main()

from pathlib import Path
import json,re
p=Path(__file__).resolve().parent
s=(p.parent/'v25_v6/fit_critic.py').read_text(encoding='utf-8')
s=s.replace('seeds=sorted(set(groups))','seeds=sorted(map(int,set(groups)))')
s=s.replace('total_weights=weight.tolist(),',"best_cv_weights=((fit(x,y,best['penalty'])-.25*np.eye(18)[0])/.75).tolist(),\n                total_weights=weight.tolist(),")
(p/'fit_critic.py').write_text(s,encoding='utf-8')
if (p/'learned_weights.json').exists():
    fit=json.loads((p/'learned_weights.json').read_text())
    s=(p/'main_baseline.cpp').read_text(encoding='utf-8')
    pat=r'static constexpr array<double,18> weight=\{([^}]+)\};'
    old=re.search(pat,s).group(0)
    new=old.replace(' weight=',' original_weight=')+'\n'
    for name,key in [('regularized_weight','learned_weights'),('fitted_weight','best_cv_weights')]:
        new+='                static constexpr array<double,18> '+name+'={'+','.join(format(v,'.14g') for v in fit[key])+'};\n'
    new+='                static const int critic_mode=search_budget::integer("LEARNED_CRITIC",1);\n'
    new+='                const auto& weight=critic_mode==2?fitted_weight:critic_mode==1?regularized_weight:original_weight;'
    assert s.count(old)==1
    s=s.replace(old,new)
    (p/'learned.cpp').write_text(s,encoding='utf-8')

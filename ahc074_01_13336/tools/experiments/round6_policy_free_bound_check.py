from itertools import permutations
from random import Random
from bisect import bisect_left
rng=Random(9813)
length=[1,2,2,3,4,4]
states=list(permutations(range(6)))
checked=0
for _ in range(30):
 selected={rng.choice([4,5])}
 for l in range(1,4):
  if rng.randrange(2):selected.add(rng.choice([k for k in range(6) if length[k]==l]))
 roots=sorted(selected,key=lambda k:length[k])
 owner={l:next(length[k] for k in roots if length[k]>=l) for l in range(1,5)}
 valid=[]
 for target in states:
  high=0;shown=set()
  for k in target:
   if length[k]>high:shown.add(k);high=length[k]
  if shown==selected:valid.append(target)
 for order in rng.sample(states,10):
  cost=[100]*5;cost[0]=0
  for k in order:
   nxt=[v+1 for v in cost]
   for h in range(5):
    if k not in selected or length[k]>h:nxt[max(h,owner[length[k]])]=min(nxt[max(h,owner[length[k]])],cost[h])
   cost=nxt
  exact=6
  for target in valid:
   rank={k:i for i,k in enumerate(target)};lis=[]
   for k in order:
    at=bisect_left(lis,rank[k])
    if at==len(lis):lis.append(rank[k])
    else:lis[at]=rank[k]
   exact=min(exact,6-len(lis))
  assert min(cost)==exact,(order,selected,min(cost),exact)
  checked+=1
print('Owner-key DP exactly matches exhaustive permutation distance:',checked,'cases')

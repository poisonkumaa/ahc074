import argparse,concurrent.futures,json,pathlib,re,subprocess,time
root=pathlib.Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser()
parser.add_argument('--count',type=int,default=100)
parser.add_argument('--profile',type=int,default=1)
parser.add_argument('--rotations',type=int,nargs='+',default=[0,32])
parser.add_argument('--repetitions',type=int,default=8)
parser.add_argument('--cutoff',type=int,default=57)
parser.add_argument('--upper',type=int,default=8)
parser.add_argument('--lower',type=int,default=4)
parser.add_argument('--workers',type=int,default=1)
parser.add_argument('--binary',default='round5_design')
parser.add_argument('--extra',nargs='*',default=[])
args=parser.parse_args()
def run(case):
 rotation,seed=case
 command=[str(root/'tools/target/release/tester'),str(root/'tools/experiments'/args.binary)]
 command+=list(map(str,[args.profile,rotation,args.repetitions,args.cutoff,args.upper,args.lower]))
 command+=args.extra
 p=subprocess.run(command,input=(root/f'tools/in/{seed:04}.txt').read_bytes(),capture_output=True,cwd=root)
 msg=p.stderr.decode();score=re.search(r'Score = (\d+)',msg);cpu=re.search(r'FinishCPU=([\d.]+)',msg)
 assert p.returncode==0 and score and cpu,(case,msg)
 data=p.stdout.split();at=192;total=0;mx=0
 for _ in range(1024):
  m=int(data[at]);assert 0<=m<=384;total+=m;mx=max(mx,m);at+=1+2*m
 assert at==len(data) and total==int(score[1]),(case,total,score[1])
 return dict(rotation=rotation,seed=seed,score=int(score[1]),cpu=float(cpu[1]),max_moves=mx,diagnostics=msg)
rows=[]
with concurrent.futures.ThreadPoolExecutor(max_workers=args.workers) as pool:
 for row in pool.map(run,[(rot,seed) for rot in args.rotations for seed in range(args.count)]):
  rows.append(row)
  if len(rows)%25==0:print('completed',len(rows),'of',args.count*len(args.rotations),flush=True)
for rotation in args.rotations:
 r=[x for x in rows if x['rotation']==rotation]
 print(rotation,'mean',sum(x['score'] for x in r)/len(r),'maxcpu',max(x['cpu'] for x in r),flush=True)
label=f'p{args.profile}_r{args.repetitions}_c{args.cutoff}_u{args.upper}_l{args.lower}_{args.count}'
if args.extra:label+='_x'+'_'.join(args.extra)
(root/f'tools/experiments/{args.binary}_{label}.json').write_text(json.dumps(dict(parameters=vars(args),cases=rows),indent=2))

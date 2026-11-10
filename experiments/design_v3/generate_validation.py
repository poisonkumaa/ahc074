"""Independent, reproducible cases following the statement's distribution.

File IDs are local IDs, not official generator seeds.
"""
from pathlib import Path
import argparse
import random

parser=argparse.ArgumentParser()
parser.add_argument('--start',type=int,default=100)
parser.add_argument('--end',type=int,default=120)
args=parser.parse_args()
out=Path(__file__).resolve().parent/'validation_in'
out.mkdir(exist_ok=True)
for file_id in range(args.start,args.end):
    rng=random.Random(740000+file_id)
    catalog=[]
    seen=set()
    while len(catalog)<256:
        ones=set(rng.sample(range(64),32))
        barcode=''.join('1' if i in ones else '0' for i in range(64))
        if barcode not in seen:
            catalog.append(barcode)
            seen.add(barcode)
    orders=[rng.randrange(256)]
    for _ in range(1023):
        x=rng.randrange(255)
        orders.append(x+(x>=orders[-1]))
    path=out/f'{file_id:04d}.txt'
    content='64 192 256 1024\n'+'\n'.join(catalog)+'\n'+'\n'.join(map(str,orders))+'\n'
    if path.exists():
        assert path.read_text(encoding='ascii')==content
    else:
        path.write_text(content,encoding='ascii')

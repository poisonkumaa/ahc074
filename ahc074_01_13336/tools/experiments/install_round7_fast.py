from pathlib import Path
import hashlib
d=Path(__file__).resolve().parent;r=d.parents[1]
expected='9c78b086622882223782412512fa1ab4b3d2b0c538e818f75be0aa5f4cbba7f4'
old=(r/'main.cpp').read_bytes();assert hashlib.sha256(old).hexdigest()==expected,'main.cpp changed; preserve user changes'
assert (d/'main_13340_round6_final.cpp').read_bytes()==old
s=(d/'round7_adaptive_v2.cpp').read_text().replace('#define AHC_CAPTURE_FACTOR 7','#define AHC_CAPTURE_FACTOR 6.5')
(d/'round7_final.cpp').write_text(s);(r/'main.cpp').write_text(s)
print('Installed',hashlib.sha256((r/'main.cpp').read_bytes()).hexdigest())

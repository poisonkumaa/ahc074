from pathlib import Path
import hashlib
d=Path(__file__).resolve().parent;root=d.parents[1]
assert hashlib.sha256((root/'main.cpp').read_bytes()).hexdigest()=='72c1acba09cedc2d2f3d92f576a3314bc66fdc42fa323cf391176e845ed416a8','main.cpp changed; inspect before installing'
src=d/'validation.json';backup=d/'baseline_13853_validation.json'
if not backup.exists():backup.write_bytes(src.read_bytes())
(root/'main.cpp').write_bytes((d/'round4_submit.cpp').read_bytes())
print('Installed SHA256',hashlib.sha256((root/'main.cpp').read_bytes()).hexdigest())

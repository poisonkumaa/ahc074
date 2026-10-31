from pathlib import Path
import subprocess
root=Path(__file__).resolve().parents[2]
here=root/'tools/experiments/multi_fresh6000'
here.mkdir(exist_ok=True)
(here/'seeds.txt').write_text(''.join(f'{s}\n' for s in range(6000,6050)))
subprocess.run([str(root/'tools/target/release/gen'),str(here/'seeds.txt'),'-d',str(here/'in')],check=True)

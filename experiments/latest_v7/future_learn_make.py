"""Ablate the old nonlinear calibration after fitting direct future-cost labels.

Mode 0: original coefficients and calibration.
Mode 1: one-standard-error regularized fit and original calibration.
Mode 2: best-CV fit and original calibration.
Mode 3: best-CV fit alone, without the old conditional loan correction.
"""
from pathlib import Path
import hashlib
import subprocess
import sys

here = Path(__file__).resolve().parent
source = here / 'learned.cpp'
assert hashlib.sha256(source.read_bytes()).hexdigest() == '0fc15340a6b2c5344c76c4371be0ac892f6534b5ceba5e8092993351c6b1a97d'
s = source.read_text(encoding='utf-8')
old = 'const auto& weight=critic_mode==2?fitted_weight:critic_mode==1?regularized_weight:original_weight;'
new = 'const auto& weight=(critic_mode==2 || critic_mode==3)?fitted_weight:critic_mode==1?regularized_weight:original_weight;'
assert s.count(old) == 1
s = s.replace(old, new, 1)
old = 'if(abs(forecast)<.74 && (!first_state.supports.empty() || !other.supports.empty()) &&'
new = 'if(critic_mode!=3 && abs(forecast)<.74 && (!first_state.supports.empty() || !other.supports.empty()) &&'
assert s.count(old) == 1
s = s.replace(old, new, 1)
target = here / 'future_learn.cpp'
target.write_text(s, encoding='utf-8')
subprocess.run([sys.executable, str(here.parent/'beam_v1/make_fixture.py'), str(target), str(here/'future_learn_fixture.cpp')], check=True)
print(hashlib.sha256(target.read_bytes()).hexdigest())

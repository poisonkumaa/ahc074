"""Charge synthetic design CPU time to exercise normal routing deadlines.

This is a test harness, not a submission source. FIXTURE_CHARGE uses CPU seconds.
"""
from pathlib import Path
import argparse
import subprocess
import sys

p=argparse.ArgumentParser()
p.add_argument('source',type=Path)
p.add_argument('target',type=Path)
a=p.parse_args()
root=Path(__file__).resolve().parents[2]
subprocess.run([sys.executable,str(root/'experiments/beam_v1/make_fixture.py'),str(a.source),str(a.target)],check=True)
s=a.target.read_text(encoding='utf-8')
needle='    const clock_t design_started=clock();'
assert s.count(needle)==1
s=s.replace(needle,'''    const double fixture_charge=search_budget::real("FIXTURE_CHARGE",0);
    const clock_t design_started=clock()-clock_t(fixture_charge*CLOCKS_PER_SEC);
    cerr<<"CHARGED_FIXTURE synthetic_design_cpu="<<fixture_charge<<"\\n";''')
a.target.write_text(s,encoding='utf-8')

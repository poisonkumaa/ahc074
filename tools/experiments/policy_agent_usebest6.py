from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy_agent_fast100000_02.cpp').read_text()
s=s.replace('".10.00","111000",".11100"','"110100","11.000","111100"')
(p/'policy_agent_best6.cpp').write_text(s)

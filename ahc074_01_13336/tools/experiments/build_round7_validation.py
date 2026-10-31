from pathlib import Path
d=Path(__file__).resolve().parent
for suffix in ('cutoff','final'):
 s=(d/f'check_round6_{suffix}.py').read_text().replace('round6','round7')
 (d/f'check_round7_{suffix}.py').write_text(s)

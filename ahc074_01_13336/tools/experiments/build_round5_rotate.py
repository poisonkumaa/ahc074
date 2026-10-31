from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'round5_sampling_adaptive_diagnostic.cpp').read_text()
s=s.replace('auto model=train_multi_model(original,12345);','auto model=train_multi_model(original,32);')
(d/'round5_rotate.cpp').write_text(s)

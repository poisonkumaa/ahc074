"""Research collector wrapper; retain replay checks but allow long labeling."""
from pathlib import Path
original=Path(__file__).resolve().parent.parent/'improve_eval/bench.py'
s=original.read_text(encoding='utf-8')
assert s.count('args.official, 20, out)')==1
s=s.replace('args.official, 20, out)','args.official, 90, out)')
exec(compile(s,str(original),'exec'),dict(__name__='__main__',__file__=str(original)))
